// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "pooling_inst.h"
#include "program_node.h"
#include "pass_manager.h"
#include "convolution_inst.h"
#include "sliding_window_utils.hpp"
#include <algorithm>

using namespace cldnn;
using namespace ov::intel_gpu;

void prepare_padding::run(program& p) {
    if (output_size_handling_enabled) {
        // Prepare upper padding for primitives that support output_size parameter.
        for (const auto& node : p.get_processing_order()) {
            if (node->get_dependencies().empty())
                continue;

            if (node->get_dependency(0).is_type<data>())
                continue;

            // Padded offsets aren't supported by onednn kernels
            if (node->get_preferred_impl_type() == impl_types::onednn)
                continue;

            auto add_required_padding = [&p](program_node& node, padding& needed_padding) {
                // Add extra reorder for cldnn primitive to handle required padding if needed
                auto& input = node.get_dependency(0);
                bool is_usr_onednn = false;
                for (auto& input_usr : input.get_users())
                    if (input_usr->get_preferred_impl_type() == impl_types::onednn)
                        is_usr_onednn = true;

                if ((input.get_preferred_impl_type() == impl_types::onednn || is_usr_onednn) &&
                    node.get_preferred_impl_type() == impl_types::ocl &&
                    static_cast<bool>(needed_padding)) {
                    auto new_reorder = std::make_shared<reorder>(node.id() + "_padding_reorder_for_" + input.id(), input.id(), input.get_output_layout());
                    auto& new_reorder_node = p.get_or_create(new_reorder);
                    p.add_intermediate(new_reorder_node, node, input);
                }

                p.apply_needed_padding(node, node.get_dependency(0), needed_padding);
            };

            if (node->is_type<convolution>()) {
                auto& prim_node = node->as<convolution>();
                const auto& prim = prim_node.get_primitive();

                if (!prim->with_output_size)
                    continue;

                auto format = node->get_output_layout().format;
                if (format == format::b_fs_zyx_fsv16 ||
                    format == format::bs_fs_zyx_bsv16_fsv16 ||
                    format == format::bs_fs_yx_bsv16_fsv16 ||
                    format == format::bs_fs_yx_bsv32_fsv32 ||
                    format == format::b_fs_zyx_fsv32)
                    continue;

                auto filter_size = prim_node.weights(0).get_output_layout().get_tensor();

                auto needed_padding = calc_sliding_window_needed_input_padding(prim_node.input().get_output_layout(),
                                                                               prim->output_size,
                                                                               filter_size,
                                                                               prim->pad,
                                                                               prim->stride,
                                                                               prim->dilation,
                                                                               false,
                                                                               1);

                add_required_padding(prim_node, needed_padding);
            } else if (node->is_type<deconvolution>()) {
                auto& prim_node = node->as<deconvolution>();
                const auto& prim = prim_node.get_primitive();

                if (!prim->with_output_size)
                    continue;

                auto filter_size = prim_node.weights(0).get_output_layout().get_tensor();

                auto needed_padding = calc_sliding_window_needed_input_padding(prim_node.input().get_output_layout(),
                                                                               prim->output_size,
                                                                               filter_size,
                                                                               prim->pad,
                                                                               prim->stride,
                                                                               ov::Strides(prim->stride.size(), 1),
                                                                               true,
                                                                               1);

                add_required_padding(prim_node, needed_padding);
            } else if (node->is_type<pooling>()) {
                auto& prim_node = node->as<pooling>();
                const auto& prim = prim_node.get_primitive();

                if (!prim->with_output_size)
                    continue;

                padding needed_padding;
                // WA for this format. sliding window needs to be fixed --perf degradation for IncepctionV1 type models
                tensor size(1);
                for (size_t i = 0; i < prim->size.size(); i++) {
                    size.spatial[i] = prim->size[prim->size.size() - i - 1];
                }

                if (node->get_output_layout().format == format::b_fs_yx_fsv16)
                    needed_padding = calc_sliding_window_needed_input_padding(prim_node.input().get_output_layout(),
                                                                              prim->output_size,
                                                                              size,
                                                                              ov::CoordinateDiff(prim->pads_begin.begin(), prim->pads_begin.end()),
                                                                              prim->stride,
                                                                              ov::Strides(prim->size.size(), 1),
                                                                              false,
                                                                              1);
                else
                    needed_padding = prim_node.input().get_output_layout().data_padding;

                add_required_padding(prim_node, needed_padding);
            } else if (node->is_type<binary_convolution>()) {
                auto& prim_node = node->as<binary_convolution>();

                auto needed_padding = prim_node.input().get_output_layout().data_padding;

                add_required_padding(prim_node, needed_padding);
            }
        }
    }

    // Prepare optimized padding for bfyx convolution.
    for (auto& pair : p.nodes_map) {
        if (pair.second->type() != convolution::type_id())
            continue;

        auto& node = pair.second->as<convolution>();
        if (node.get_dependencies().empty())
            continue;

        auto conv = node.get_primitive();
        if (node.is_dynamic()) continue;
        auto& conv_input_node = node.get_dependency(0);
        auto conv_layout = node.get_output_layout();

        // right now output padding optimization is only available for bfyx format and data type = float32
        if (conv_layout.format != cldnn::format::bfyx &&
            conv_layout.format != cldnn::format::b_fs_yx_fsv16 &&
            conv_layout.format != cldnn::format::b_fs_zyx_fsv16 &&
            conv_layout.format != cldnn::format::bs_fs_yx_bsv16_fsv16 &&
            conv_layout.format != cldnn::format::b_fs_yx_fsv4 &&
            conv_layout.format != cldnn::format::fs_b_yx_fsv32 &&
            conv_layout.format != cldnn::format::b_fs_yx_32fp) {
            continue;
        }

        // convolution have only one input primitive
        auto prev_prim_output_layout = conv_input_node.get_output_layout();

        // For 3d convolution padding is needed only for int8 case
        // FP16/32 kernels can work w/o physical padding
        if (prev_prim_output_layout.format == cldnn::format::b_fs_zyx_fsv16 &&
            prev_prim_output_layout.data_type != data_types::i8 && prev_prim_output_layout.data_type != data_types::u8)
            continue;

        // We shoudn't apply any padding to nodes which are marked as outputs or have type as data
        if (conv_input_node.is_output() || conv_input_node.is_type<data>())
            continue;

        // Padded offsets aren't supported by onednn kernels
        if (conv_input_node.get_preferred_impl_type() == impl_types::onednn)
            continue;

        if (node.get_preferred_impl_type() == impl_types::onednn)
            continue;

        // Calculating input padding needed for convolution
        auto& filter_node = node.as<convolution>().weights(0);
        auto filter_prim = filter_node.get_primitive();

        layout filter_layout = filter_node.get_output_layout().convert_to_weights_layout(conv->grouped_weights_shape);

        // Compute initial required paddings for primitive used as input for convolution.
        auto pad = conv->pad;
        auto stride = conv->stride;
        auto dilation = conv->dilation;
        uint32_t stride_z = stride.size() >= 3 ? stride[stride.size() - 3] : 1;
        uint32_t stride_y = stride.size() >= 2 ? stride[stride.size() - 2] : 1;
        uint32_t stride_x = stride.size() >= 1 ? stride[stride.size() - 1] : 1;

        uint32_t dilation_z = dilation.size() >= 3 ? dilation[dilation.size() - 3] : 1;
        uint32_t dilation_y = dilation.size() >= 2 ? dilation[dilation.size() - 2] : 1;
        uint32_t dilation_x = dilation.size() >= 1 ? dilation[dilation.size() - 1] : 1;

        tensor::value_type pad_z = pad.size() >= 3 ? pad[pad.size() - 3] : 0;
        tensor::value_type pad_y = pad.size() >= 2 ? pad[pad.size() - 2] : 0;
        tensor::value_type pad_x = pad.size() >= 1 ? pad[pad.size() - 1] : 0;

        auto input_limit_x = -pad_x + (conv_layout.spatial(0) - 1) * stride_x +
                             (filter_layout.spatial(0) - 1) * dilation_x + 1;
        auto input_limit_y = -pad_y + (conv_layout.spatial(1) - 1) * stride_y +
                             (filter_layout.spatial(1) - 1) * dilation_y + 1;
        auto input_limit_z = -pad_z + (conv_layout.spatial(2) - 1) * stride_z +
                             (filter_layout.spatial(2) - 1) * dilation_z + 1;

        auto padding_begin_x = std::max(pad_x, 0);
        auto padding_begin_y = std::max(pad_y, 0);
        auto padding_begin_z = std::max(pad_z, 0);
        auto padding_end_x = std::max<tensor::value_type>(input_limit_x - prev_prim_output_layout.spatial(0), 0);
        auto padding_end_y = std::max<tensor::value_type>(input_limit_y - prev_prim_output_layout.spatial(1), 0);
        auto padding_end_z = std::max<tensor::value_type>(input_limit_z - prev_prim_output_layout.spatial(2), 0);

        // Adjust right padding, so entire buffer size in X dimension is properly aligned.
        // TODO: NOTE: Will be reenabled with next check-in once heuristic for line-aligned algorithm will be added.
        // auto needed_buffer_size_x = static_cast<cldnn::tensor::value_type>(
        //    round_up_to(left_padding + prev_prim_output_layout.spatial(0) + right_padding, 16));
        // right_padding = needed_buffer_size_x - left_padding - prev_prim_output_layout.spatial(0);

        cldnn::padding needed_padding({0, 0, padding_begin_x, padding_begin_y, padding_begin_z}, {0, 0, padding_end_x, padding_end_y, padding_end_z}, 0);
        needed_padding = padding::max(prev_prim_output_layout.data_padding, needed_padding);
        p.apply_needed_padding(node, conv_input_node, needed_padding);
    }

    for (auto& pair : p.nodes_map) {
        if (pair.second->type() != binary_convolution::type_id())
            continue;

        auto& node = pair.second->as<binary_convolution>();
        if (node.get_dependencies().empty())
            continue;

        if (node.is_dynamic()) continue;
        auto conv = node.get_primitive();
        auto& conv_input_node = node.get_dependency(0);
        auto conv_layout = node.get_output_layout();

        // right now output padding optimization is only available for bfyx format and data type = float32
        if (conv_layout.format != cldnn::format::bfyx && conv_layout.format != cldnn::format::b_fs_yx_32fp)
            continue;

        // We shoudn't apply any padding to nodes which are marked as outputs or have type as data
        if (conv_input_node.is_output() || conv_input_node.is_type<data>())
            continue;

        // Calculating input padding needed for convolution
        auto& filter_node = node.as<binary_convolution>().weights(0);
        auto filter_prim = filter_node.get_primitive();

        layout filter_layout = filter_node.get_output_layout();

        // convolution have only one input primitive
        auto prev_prim_output_layout = conv_input_node.get_output_layout();

        // Compute initial required paddings for primitive used as input for convolution.
        auto pad = conv->pad;
        auto stride = conv->stride;
        auto dilation = conv->dilation;

        auto stride_z = stride.size() >= 3 ? stride[stride.size() - 3] : 1;
        auto stride_y = stride.size() >= 2 ? stride[stride.size() - 2] : 1;
        auto stride_x = stride.size() >= 1 ? stride[stride.size() - 1] : 1;

        auto dilation_z = dilation.size() >= 3 ? dilation[dilation.size() - 3] : 1;
        auto dilation_y = dilation.size() >= 2 ? dilation[dilation.size() - 2] : 1;
        auto dilation_x = dilation.size() >= 1 ? dilation[dilation.size() - 1] : 1;

        auto pad_z = pad.size() >= 3 ? pad[pad.size() - 3] : 0;
        auto pad_y = pad.size() >= 2 ? pad[pad.size() - 2] : 0;
        auto pad_x = pad.size() >= 1 ? pad[pad.size() - 1] : 0;

        auto input_limit_x = -pad_x + (conv_layout.spatial(0) - 1) * stride_x +
                             (filter_layout.spatial(0) - 1) * dilation_x + 1;
        auto input_limit_y = -pad_y + (conv_layout.spatial(1) - 1) * stride_y +
                             (filter_layout.spatial(1) - 1) * dilation_y + 1;
        auto input_limit_z = -pad_z + (conv_layout.spatial(2) - 1) * stride_z +
                             (filter_layout.spatial(2) - 1) * dilation_z + 1;

        auto padding_begin_x = std::max<tensor::value_type>(pad_x, 0);
        auto padding_begin_y = std::max<tensor::value_type>(pad_y, 0);
        auto padding_begin_z = std::max<tensor::value_type>(pad_z, 0);
        auto padding_end_x = std::max<tensor::value_type>(input_limit_x - prev_prim_output_layout.spatial(0), 0);
        auto padding_end_y = std::max<tensor::value_type>(input_limit_y - prev_prim_output_layout.spatial(1), 0);
        auto padding_end_z = std::max<tensor::value_type>(input_limit_z - prev_prim_output_layout.spatial(2), 0);

        cldnn::padding needed_padding({0, 0, padding_begin_x, padding_begin_y, padding_begin_z}, {0, 0, padding_end_x, padding_end_y, padding_end_z}, 0);
        needed_padding = padding::max(prev_prim_output_layout.data_padding, needed_padding);

        p.apply_needed_padding(node, conv_input_node, needed_padding);
    }
}
