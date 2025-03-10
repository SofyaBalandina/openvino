// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "detection_output_inst.h"
#include "primitive_type_base.h"
#include "intel_gpu/graph/serialization/string_serializer.hpp"
#include "intel_gpu/runtime/error_handler.hpp"
#include "json_object.h"
#include <string>

namespace cldnn {
GPU_DEFINE_PRIMITIVE_TYPE_ID(detection_output)

layout detection_output_inst::calc_output_layout(detection_output_node const& node, kernel_impl_params const& impl_param) {
    assert(static_cast<bool>(impl_param.desc->output_data_types[0]) == false &&
           "Output data type forcing is not supported for "
           "detection_output_node!");
    auto desc = impl_param.typed_desc<detection_output>();
    CLDNN_ERROR_NOT_EQUAL(desc->id,
                          "Detection output layer input number",
                          impl_param.input_layouts.size(),
                          "expected number of inputs",
                          static_cast<size_t>(3),
                          "");

    auto input_layout = impl_param.get_input_layout();

    // Batch size and feature size are 1.
    // Number of bounding boxes to be kept is set to keep_top_k*batch size.
    // If number of detections is lower than top_k, will write dummy results at the end with image_id=-1.
    // Each row is a 7 dimension vector, which stores:
    // [image_id, label, confidence, xmin, ymin, xmax, ymax]
    int output_size = static_cast<int>(input_layout.get_linear_size()) / PRIOR_BOX_SIZE;
    int num_classes = desc->num_classes;

    if (desc->share_location) {
        num_classes = (desc->background_label_id == 0) ? desc->num_classes - 1
                                                       : desc->num_classes;
        output_size *= num_classes;
    }

    if (desc->top_k != -1) {
        int top_k = desc->top_k * num_classes * input_layout.batch();
        if (top_k < output_size) {
            output_size = top_k;
        }
    }

    output_size *= DETECTION_OUTPUT_ROW_SIZE;
    // Add space for number of output results per image - needed in the next detection output step
    output_size += ((input_layout.batch() + 15) / 16) * 16;

    return {input_layout.data_type, cldnn::format::bfyx,
            cldnn::tensor(1, 1, DETECTION_OUTPUT_ROW_SIZE, desc->keep_top_k * input_layout.batch())};
}

std::string detection_output_inst::to_string(detection_output_node const& node) {
    auto node_info = node.desc_to_json();
    auto desc = node.get_primitive();
    auto share_location = desc->share_location ? "true" : "false";
    auto variance_encoded = desc->variance_encoded_in_target ? "true" : "false";
    auto prior_is_normalized = desc->prior_is_normalized ? "true" : "false";
    auto decrease_label_id = desc->decrease_label_id ? "true" : "false";
    auto clip_before_nms = desc->clip_before_nms ? "true" : "false";
    auto clip_after_nms = desc->clip_after_nms ? "true" : "false";
    auto& input_location = node.location();
    auto& input_prior_box = node.prior_box();
    auto& input_confidence = node.confidence();

    std::stringstream primitive_description;
    std::string str_code_type;

    switch (desc->code_type) {
        case prior_box_code_type::corner:
            str_code_type = "corner";
            break;
        case prior_box_code_type::center_size:
            str_code_type = "center size";
            break;
        case prior_box_code_type::corner_size:
            str_code_type = "corner size";
            break;
        default:
            str_code_type = "not supported code type";
            break;
    }

    json_composite detec_out_info;
    detec_out_info.add("input location id", input_location.id());
    detec_out_info.add("input confidence id", input_confidence.id());
    detec_out_info.add("input prior box id", input_prior_box.id());
    detec_out_info.add("num_classes:", desc->num_classes);
    detec_out_info.add("keep_top_k", desc->keep_top_k);
    detec_out_info.add("share_location", share_location);
    detec_out_info.add("background_label_id", desc->background_label_id);
    detec_out_info.add("nms_treshold", desc->nms_threshold);
    detec_out_info.add("top_k", desc->top_k);
    detec_out_info.add("eta", desc->eta);
    detec_out_info.add("code_type", str_code_type);
    detec_out_info.add("variance_encoded", variance_encoded);
    detec_out_info.add("confidence_threshold", desc->confidence_threshold);
    detec_out_info.add("prior_info_size", desc->prior_info_size);
    detec_out_info.add("prior_coordinates_offset", desc->prior_coordinates_offset);
    detec_out_info.add("prior_is_normalized", prior_is_normalized);
    detec_out_info.add("input_width", desc->input_width);
    detec_out_info.add("input_height", desc->input_height);
    detec_out_info.add("decrease_label_id", decrease_label_id);
    detec_out_info.add("clip_before_nms", clip_before_nms);
    detec_out_info.add("clip_after_nms", clip_after_nms);
    detec_out_info.dump(primitive_description);

    node_info->add("dection output info", detec_out_info);
    node_info->dump(primitive_description);

    return primitive_description.str();
}

detection_output_inst::typed_primitive_inst(network& network, detection_output_node const& node)
    : parent(network, node) {
    auto location_layout = node.location().get_output_layout();
    auto confidence_layout = node.confidence().get_output_layout();
    auto prior_box_layout = node.prior_box().get_output_layout();
    CLDNN_ERROR_NOT_PROPER_FORMAT(node.id(),
                                  "Location memory format",
                                  location_layout.format.value,
                                  "expected bfyx input format",
                                  format::bfyx);
    CLDNN_ERROR_NOT_PROPER_FORMAT(node.id(),
                                  "Confidence memory format",
                                  confidence_layout.format.value,
                                  "expected bfyx input format",
                                  format::bfyx);
    CLDNN_ERROR_NOT_PROPER_FORMAT(node.id(),
                                  "Prior box memory format",
                                  prior_box_layout.format.value,
                                  "expected bfyx input format",
                                  format::bfyx);

    CLDNN_ERROR_NOT_EQUAL(node.id(),
                          "Location input dimensions",
                          (location_layout.feature() * location_layout.batch()),
                          "detection output layer dimensions",
                          static_cast<int>(location_layout.count()),
                          "Location input/ detection output dims mismatch");

    CLDNN_ERROR_NOT_EQUAL(node.id(),
                          "Confidence input dimensions",
                          (confidence_layout.feature() * confidence_layout.batch()),
                          "detection output layer dimensions",
                          static_cast<int>(confidence_layout.count()),
                          "Confidence input/detection output dims mistmach");

    CLDNN_ERROR_NOT_EQUAL(node.id(),
                          "Confidence batch size",
                          confidence_layout.batch(),
                          "location input batch size",
                          location_layout.batch(),
                          "Batch sizes mismatch.");

    auto desc = node.get_primitive();
    int prior_feature_size = desc->variance_encoded_in_target ? 1 : 2;
    CLDNN_ERROR_NOT_EQUAL(node.id(), "Prior box spatial X", prior_box_layout.spatial(0), "expected value", 1, "");
    CLDNN_ERROR_NOT_EQUAL(node.id(),
                          "Prior box feature size",
                          prior_box_layout.feature(),
                          "expected value",
                          prior_feature_size,
                          "");

    CLDNN_ERROR_BOOL(node.id(),
                     "Detection output layer padding",
                     node.is_padded(),
                     "Detection output layer doesn't support output padding.");
    CLDNN_ERROR_BOOL(node.id(),
                     "Detection output layer Prior-box input padding",
                     node.get_dependency(2).is_padded(),
                     "Detection output layer doesn't support input padding in Prior-Box input");
}

void detection_output_inst::save(cldnn::BinaryOutputBuffer& ob) const {
    parent::save(ob);

    // argument (struct detection_output)
    ob << argument->id;
    ob << argument->input[0].pid;
    ob << argument->input[1].pid;
    ob << argument->input[2].pid;
    ob << make_data(&argument->output_paddings[0], sizeof(argument->output_paddings[0]));
    ob << argument->num_classes;
    ob << argument->keep_top_k;
    ob << argument->share_location;
    ob << argument->background_label_id;
    ob << argument->nms_threshold;
    ob << argument->top_k;
    ob << argument->eta;
    ob << make_data(&argument->code_type, sizeof(argument->code_type));
    ob << argument->variance_encoded_in_target;
    ob << argument->confidence_threshold;
    ob << argument->prior_info_size;
    ob << argument->prior_coordinates_offset;
    ob << argument->prior_is_normalized;
    ob << argument->input_width;
    ob << argument->input_height;
    ob << argument->decrease_label_id;
    ob << argument->clip_before_nms;
    ob << argument->clip_after_nms;
}

void detection_output_inst::load(cldnn::BinaryInputBuffer& ib) {
    parent::load(ib);

    primitive_id id;
    primitive_id input_location;
    primitive_id input_confidence;
    primitive_id input_prior_box;
    uint32_t num_classes;
    uint32_t keep_top_k;
    bool share_location;
    int background_label_id;
    float nms_threshold;
    int top_k;
    float eta;
    prior_box_code_type code_type;
    bool variance_encoded_in_target;
    float confidence_threshold;
    int32_t prior_info_size;
    int32_t prior_coordinates_offset;
    bool prior_is_normalized;
    int32_t input_width;
    int32_t input_height;
    bool decrease_label_id;
    bool clip_before_nms;
    bool clip_after_nms;
    // primitive_id ext_prim_id;
    padding output_padding;

    ib >> id;
    ib >> input_location;
    ib >> input_confidence;
    ib >> input_prior_box;
    ib >> make_data(&output_padding, sizeof(output_padding));
    ib >> num_classes;
    ib >> keep_top_k;
    ib >> share_location;
    ib >> background_label_id;
    ib >> nms_threshold;
    ib >> top_k;
    ib >> eta;
    ib >> make_data(&code_type, sizeof(code_type));
    ib >> variance_encoded_in_target;
    ib >> confidence_threshold;
    ib >> prior_info_size;
    ib >> prior_coordinates_offset;
    ib >> prior_is_normalized;
    ib >> input_width;
    ib >> input_height;
    ib >> decrease_label_id;
    ib >> clip_before_nms;
    ib >> clip_after_nms;

    argument = std::make_shared<detection_output>(
        id, input_info(input_location), input_info(input_confidence), input_info(input_prior_box),
        num_classes, keep_top_k, share_location, background_label_id, nms_threshold, top_k, eta, code_type,
        variance_encoded_in_target, confidence_threshold, prior_info_size, prior_coordinates_offset,
        prior_is_normalized, input_width, input_height, decrease_label_id, clip_before_nms, clip_after_nms,
        output_padding);
}
}  // namespace cldnn
