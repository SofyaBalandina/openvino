// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "test_utils.h"
#include "fusion_test_common.hpp"

#include <intel_gpu/primitives/input_layout.hpp>
#include <intel_gpu/primitives/quantize.hpp>
#include <intel_gpu/primitives/eltwise.hpp>
#include <intel_gpu/primitives/data.hpp>

#include <cmath>

using namespace cldnn;
using namespace ::tests;

namespace {
struct eltwise_test_params {
    tensor input_size;
    data_types input_type;
    data_types input_type2;
    format input_format;
    data_types default_type;
    format default_format;
    eltwise_mode mode;
    size_t expected_fused_primitives;
    size_t expected_not_fused_primitives;
};

class EltwiseFusingTest : public ::BaseFusingTest<eltwise_test_params> {
public:
    void execute(eltwise_test_params& p) {
        auto input_prim = get_mem(get_input_layout(p));
        auto input_prim2 = get_mem(get_input_layout2(p));

        network network_not_fused(this->engine, this->topology_non_fused, bo_not_fused);
        network network_fused(this->engine, this->topology_fused, bo_fused);

        auto inputs = network_fused.get_input_ids();
        network_fused.set_input_data("input", input_prim);
        network_not_fused.set_input_data("input", input_prim);
        if (std::find(inputs.begin(), inputs.end(), "input2") != inputs.end()) {
            network_fused.set_input_data("input2", input_prim2);
            network_not_fused.set_input_data("input2", input_prim2);
        }

        compare(network_not_fused, network_fused, p);
    }

    layout get_input_layout(eltwise_test_params& p) {
        return layout{ p.input_type, p.input_format, p.input_size };
    }

    layout get_input_layout2(eltwise_test_params& p) {
        return layout{ p.input_type2, p.input_format, p.input_size };
    }

    layout get_per_channel_layout(eltwise_test_params& p) {
        return layout{ p.default_type, p.default_format, tensor{ 1, p.input_size.feature[0], 1, 1 } };
    }
};
}  // namespace

/* ----------------------------------------------------------------------------------------------------- */
/* ---------------------------------------- Eltwise cases ---------------------------------------------- */
/* ----------------------------------------------------------------------------------------------------- */
#define CASE_ELTWISE_FP32_1         { 2, 16, 4, 4 }, data_types::f32, data_types::f32, format::bfyx,           data_types::f32,  format::bfyx,             eltwise_mode::sum
#define CASE_ELTWISE_FP32_2         { 2, 16, 4, 4 }, data_types::f32, data_types::f32, format::bfzyx,          data_types::f32,  format::bfzyx,            eltwise_mode::sum
#define CASE_ELTWISE_FP32_3         { 2, 32, 4, 8 }, data_types::f32, data_types::f32, format::b_fs_yx_fsv16,  data_types::f32,  format::b_fs_yx_fsv16,    eltwise_mode::sum
#define CASE_ELTWISE_FP32_4         { 2, 16, 4, 4 }, data_types::f32, data_types::f32, format::bfwzyx,         data_types::f32,  format::bfwzyx,           eltwise_mode::sum
#define CASE_ELTWISE_FP16_1         { 2, 16, 4, 4 }, data_types::f16, data_types::f16, format::bfyx,           data_types::f16,  format::bfyx,             eltwise_mode::sum
#define CASE_ELTWISE_FP16_2         { 2, 16, 4, 4 }, data_types::f16, data_types::f16, format::bfzyx,          data_types::f16,  format::bfzyx,            eltwise_mode::sum
#define CASE_ELTWISE_FP16_3         { 2, 32, 4, 8 }, data_types::f16, data_types::f16, format::b_fs_yx_fsv16,  data_types::f16,  format::b_fs_yx_fsv16,    eltwise_mode::sum
#define CASE_ELTWISE_FP16_4         { 3, 32, 4, 4 }, data_types::f16, data_types::f16, format::fs_b_yx_fsv32,  data_types::f16,  format::fs_b_yx_fsv32,    eltwise_mode::sum
#define CASE_ELTWISE_I8_1           { 2, 16, 4, 4 }, data_types::i8,  data_types::i8,  format::bfyx,           data_types::f32,  format::bfyx,             eltwise_mode::sum
#define CASE_ELTWISE_I8_2           { 2, 16, 4, 4 }, data_types::i8,  data_types::i8,  format::bfzyx,          data_types::f32,  format::bfzyx,            eltwise_mode::sum
#define CASE_ELTWISE_I8_3           { 2, 16, 4, 4 }, data_types::i8,  data_types::i8,  format::b_fs_yx_fsv16,  data_types::f32,  format::b_fs_yx_fsv16,    eltwise_mode::sum
#define CASE_ELTWISE_U8_1           { 2, 16, 4, 4 }, data_types::u8,  data_types::u8,  format::bfyx,           data_types::f32,  format::bfyx,             eltwise_mode::sum
#define CASE_ELTWISE_U8_2           { 2, 16, 4, 4 }, data_types::u8,  data_types::u8,  format::bfzyx,          data_types::f32,  format::bfzyx,            eltwise_mode::sum
#define CASE_ELTWISE_U8_3           { 2, 16, 4, 4 }, data_types::u8,  data_types::u8,  format::b_fs_yx_fsv16,  data_types::f32,  format::b_fs_yx_fsv16,    eltwise_mode::sum
#define CASE_ELTWISE_FP32_FP16_1    { 2, 16, 4, 4 }, data_types::f32, data_types::f16, format::bfyx,           data_types::f32,  format::bfyx,             eltwise_mode::sum
#define CASE_ELTWISE_FP32_FP16_2    { 2, 16, 4, 4 }, data_types::f32, data_types::f16, format::bfzyx,          data_types::f32,  format::bfzyx,            eltwise_mode::sum
#define CASE_ELTWISE_FP32_FP16_3    { 2, 32, 4, 4 }, data_types::f32, data_types::f16, format::b_fs_yx_fsv16,  data_types::f32,  format::b_fs_yx_fsv16,    eltwise_mode::sum
#define CASE_ELTWISE_FP16_FP32_1    { 2, 16, 4, 4 }, data_types::f16, data_types::f32, format::bfyx,           data_types::f16,  format::bfyx,             eltwise_mode::sum
#define CASE_ELTWISE_FP16_FP32_2    { 2, 16, 4, 4 }, data_types::f16, data_types::f32, format::bfzyx,          data_types::f16,  format::bfzyx,            eltwise_mode::sum
#define CASE_ELTWISE_FP16_FP32_3    { 2, 32, 4, 4 }, data_types::f16, data_types::f32, format::b_fs_yx_fsv16,  data_types::f16,  format::b_fs_yx_fsv16,    eltwise_mode::sum
#define CASE_ELTWISE_I8_FP16_1      { 2, 16, 4, 4 }, data_types::i8,  data_types::f16, format::bfyx,           data_types::f32,  format::bfyx,             eltwise_mode::sum
#define CASE_ELTWISE_I8_FP16_2      { 2, 16, 4, 4 }, data_types::i8,  data_types::f16, format::bfzyx,          data_types::f32,  format::bfzyx,            eltwise_mode::sum
#define CASE_ELTWISE_I8_FP16_3      { 2, 32, 4, 4 }, data_types::i8,  data_types::f16, format::b_fs_yx_fsv16,  data_types::f32,  format::b_fs_yx_fsv16,    eltwise_mode::sum
#define CASE_ELTWISE_I8_FP32_1      { 2, 16, 4, 4 }, data_types::i8,  data_types::f32, format::bfyx,           data_types::f16,  format::bfyx,             eltwise_mode::sum
#define CASE_ELTWISE_I8_FP32_2      { 2, 16, 4, 4 }, data_types::i8,  data_types::f32, format::bfzyx,          data_types::f16,  format::bfzyx,            eltwise_mode::sum
#define CASE_ELTWISE_I8_FP32_3      { 2, 32, 4, 4 }, data_types::i8,  data_types::f32, format::b_fs_yx_fsv16,  data_types::f16,  format::b_fs_yx_fsv16,    eltwise_mode::sum
#define CASE_ELTWISE_U8_FP16_1      { 2, 16, 4, 4 }, data_types::u8,  data_types::f16, format::bfyx,           data_types::f32,  format::bfyx,             eltwise_mode::sum
#define CASE_ELTWISE_U8_FP16_2      { 2, 16, 4, 4 }, data_types::u8,  data_types::f16, format::bfzyx,          data_types::f32,  format::bfzyx,            eltwise_mode::sum
#define CASE_ELTWISE_U8_FP16_3      { 2, 32, 4, 4 }, data_types::u8,  data_types::f16, format::b_fs_yx_fsv16,  data_types::f32,  format::b_fs_yx_fsv16,    eltwise_mode::sum
#define CASE_ELTWISE_U8_FP32_1      { 2, 16, 4, 4 }, data_types::u8,  data_types::f32, format::bfyx,           data_types::f16,  format::bfyx,             eltwise_mode::sum
#define CASE_ELTWISE_U8_FP32_2      { 2, 16, 4, 4 }, data_types::u8,  data_types::f32, format::bfzyx,          data_types::f16,  format::bfzyx,            eltwise_mode::sum
#define CASE_ELTWISE_U8_FP32_3      { 2, 32, 4, 4 }, data_types::u8,  data_types::f32, format::b_fs_yx_fsv16,  data_types::f16,  format::b_fs_yx_fsv16,    eltwise_mode::sum

#define CASE_ELTWISE_FP32_5         { 1,  5, 4, 4 }, data_types::f32, data_types::f32, format::b_fs_yx_fsv4,  data_types::f32,  format::b_fs_yx_fsv4,    eltwise_mode::sum
#define CASE_ELTWISE_FP32_6         { 2, 32, 4, 8 }, data_types::f32, data_types::f32, format::b_fs_yx_fsv4,  data_types::f32,  format::b_fs_yx_fsv4,    eltwise_mode::sum
#define CASE_ELTWISE_FP16_5         { 2, 32, 4, 8 }, data_types::f16, data_types::f16, format::b_fs_yx_fsv4,  data_types::f16,  format::b_fs_yx_fsv4,    eltwise_mode::sum
#define CASE_ELTWISE_FP16_6         { 1, 32, 4, 8 }, data_types::f16, data_types::f16, format::byxf,          data_types::f16,  format::byxf,            eltwise_mode::sum
#define CASE_ELTWISE_I8_4           { 2, 16, 4, 4 }, data_types::i8,  data_types::i8,  format::b_fs_yx_fsv4,  data_types::f32,  format::b_fs_yx_fsv4,    eltwise_mode::sum
#define CASE_ELTWISE_U8_4           { 2, 16, 4, 4 }, data_types::u8,  data_types::u8,  format::b_fs_yx_fsv4,  data_types::f32,  format::b_fs_yx_fsv4,    eltwise_mode::sum

class eltwise_quantize : public EltwiseFusingTest {};
TEST_P(eltwise_quantize, u8) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        eltwise("eltwise", { input_info("input"), input_info("input2") }, p.mode, p.default_type),
        data("in_lo", get_mem(get_single_element_layout(p), min_random, 0)),
        data("in_hi", get_mem(get_single_element_layout(p), 1, max_random)),
        data("out_lo", get_mem(get_single_element_layout(p), 0)),
        data("out_hi", get_mem(get_single_element_layout(p), 255)),
        quantize("quantize", input_info("eltwise"), input_info("in_lo"), input_info("in_hi"),
                 input_info("out_lo"), input_info("out_hi"), 256, data_types::u8),
        reorder("out", input_info("quantize"), p.default_format, data_types::f32)
    );

    tolerance = 1.f;
    execute(p);
}

TEST_P(eltwise_quantize, i8_per_channel) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        eltwise("eltwise", { input_info("input"), input_info("input2") }, p.mode, p.default_type),
        data("in_lo", get_mem(get_per_channel_layout(p), min_random, 0)),
        data("in_hi", get_mem(get_per_channel_layout(p), 1, max_random)),
        data("out_lo", get_mem(get_single_element_layout(p), -128)),
        data("out_hi", get_mem(get_single_element_layout(p), 127)),
        quantize("quantize", input_info("eltwise"), input_info("in_lo"), input_info("in_hi"),
                 input_info("out_lo"), input_info("out_hi"), 256, data_types::i8),
        reorder("out", input_info("quantize"), p.default_format, data_types::f32)
    );

    tolerance = 1.f;
    execute(p);
}

INSTANTIATE_TEST_SUITE_P(fusings_gpu, eltwise_quantize, ::testing::ValuesIn(std::vector<eltwise_test_params>{
    eltwise_test_params{ CASE_ELTWISE_FP16_1, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP16_2, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP16_3, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_1, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_2, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_3, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_FP16_1, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_FP16_2, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_FP16_3, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP16_FP32_1, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP16_FP32_2, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP16_FP32_3, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_I8_FP32_1, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_I8_FP32_2, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_I8_FP32_3, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_U8_FP32_1, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_U8_FP32_2, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_U8_FP32_3, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_I8_FP16_1, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_I8_FP16_2, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_I8_FP16_3, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_U8_FP16_1, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_U8_FP16_2, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_U8_FP16_3, 3, 4 },
    // fsv4
    eltwise_test_params{ CASE_ELTWISE_FP16_5, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_5, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_6, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_I8_4, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_U8_4, 3, 4 },
}));

class eltwise_const_path : public EltwiseFusingTest {};
TEST_P(eltwise_const_path, not_fuse_to_const_eltwise) {
    auto p = GetParam();
    create_topologies(
        data("const1", get_mem(get_input_layout2(p), -10, 10)),
        data("const2", get_mem(get_input_layout2(p), -10, 10)),
        input_layout("input", get_input_layout2(p)),
        eltwise("eltwise", { input_info("const1"), input_info("const2") }, p.mode, p.default_type),
        eltwise("add", { input_info("eltwise"), input_info("input") }, eltwise_mode::sum),
        activation("activation", input_info("add"), activation_func::negative),
        reorder("out", input_info("activation"), p.default_format, data_types::f32)
    );
    // Activation won't be fused because onednn doesn't support negative activation
    if (engine.get_device_info().supports_immad)
        p.expected_fused_primitives++;

    tolerance = 1e-5f;
    execute(p);
}

INSTANTIATE_TEST_SUITE_P(fusings_gpu, eltwise_const_path, ::testing::ValuesIn(std::vector<eltwise_test_params>{
    eltwise_test_params{ CASE_ELTWISE_FP16_3, 2, 3 },
    eltwise_test_params{ CASE_ELTWISE_FP32_3, 2, 3 },
    eltwise_test_params{ CASE_ELTWISE_FP32_5, 2, 3 },
    eltwise_test_params{ CASE_ELTWISE_FP32_6, 2, 3 },
    eltwise_test_params{ CASE_ELTWISE_I8_4, 2, 3 },
    eltwise_test_params{ CASE_ELTWISE_U8_4, 2, 3 },
}));

class eltwise_fp32_fsv16 : public EltwiseFusingTest {};
TEST_P(eltwise_fp32_fsv16, add) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        data("add_data", get_mem(get_per_channel_layout(p), -10, 10)),
        eltwise("eltwise", { input_info("input"), input_info("input2") }, p.mode, p.default_type),
        eltwise("add", { input_info("eltwise"), input_info("add_data") }, eltwise_mode::sum),
        activation("activation", input_info("add"), activation_func::negative),
        reorder("out", input_info("activation"), p.default_format, data_types::f32)
    );
    // Activation won't be fused because onednn doesn't support negative activation
    if (engine.get_device_info().supports_immad)
        p.expected_fused_primitives++;

    implementation_desc eltw_impl = { format::b_fs_yx_fsv16, "eltwise_b_fs_yx_fsv16" };
    bo_fused.set_option(build_option::force_implementations({ { "eltwise", eltw_impl } }));

    tolerance = 1e-5f;
    execute(p);
}

TEST_P(eltwise_fp32_fsv16, add_per_element) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        data("add_data", get_mem(get_input_layout(p), -10, 10)),
        eltwise("eltwise", { input_info("input"), input_info("input2") }, p.mode, p.default_type),
        eltwise("add", { input_info("eltwise"), input_info("add_data") }, eltwise_mode::sum),
        activation("activation", input_info("add"), activation_func::negative),
        reorder("out", input_info("activation"), p.default_format, data_types::f32)
    );
    // Activation won't be fused because onednn doesn't support negative activation
    if (engine.get_device_info().supports_immad)
        p.expected_fused_primitives++;

    implementation_desc eltw_impl = { format::b_fs_yx_fsv16, "eltwise_b_fs_yx_fsv16" };
    bo_fused.set_option(build_option::force_implementations({ { "eltwise", eltw_impl } }));

    tolerance = 1e-5f;
    execute(p);
}

INSTANTIATE_TEST_SUITE_P(fusings_gpu, eltwise_fp32_fsv16, ::testing::ValuesIn(std::vector<eltwise_test_params>{
    eltwise_test_params{ CASE_ELTWISE_FP16_3, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_FP32_3, 3, 5 },
}));

class eltwise_fp32_fsv32 : public EltwiseFusingTest {};
TEST_P(eltwise_fp32_fsv32, add) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        data("add_data", get_mem(get_per_channel_layout(p), -10, 10)),
        eltwise("eltwise", { input_info("input"), input_info("input2") }, p.mode, p.default_type),
        eltwise("add", { input_info("eltwise"), input_info("add_data") }, eltwise_mode::sum),
        activation("activation", input_info("add"), activation_func::negative),
        reorder("out", input_info("activation"), p.default_format, data_types::f32)
    );
    // Activation won't be fused because onednn doesn't support negative activation
    if (engine.get_device_info().supports_immad)
        p.expected_fused_primitives++;

    implementation_desc eltw_impl = { format::fs_b_yx_fsv32, "eltwise_fs_b_yx_fsv32" };
    bo_fused.set_option(build_option::force_implementations({ { "eltwise", eltw_impl } }));

    tolerance = 1e-5f;
    execute(p);
}

TEST_P(eltwise_fp32_fsv32, add_per_element) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        data("add_data", get_mem(get_input_layout(p), -10, 10)),
        eltwise("eltwise", { input_info("input"), input_info("input2") }, p.mode, p.default_type),
        eltwise("add", { input_info("eltwise"), input_info("add_data") }, eltwise_mode::sum),
        activation("activation", input_info("add"), activation_func::negative),
        reorder("out", input_info("activation"), p.default_format, data_types::f32)
    );
    // Activation won't be fused because onednn doesn't support negative activation
    if (engine.get_device_info().supports_immad)
        p.expected_fused_primitives++;

    implementation_desc eltw_impl = { format::fs_b_yx_fsv32, "eltwise_fs_b_yx_fsv32" };
    bo_fused.set_option(build_option::force_implementations({ { "eltwise", eltw_impl } }));

    tolerance = 1e-5f;
    execute(p);
}

INSTANTIATE_TEST_SUITE_P(fusings_gpu, eltwise_fp32_fsv32, ::testing::ValuesIn(std::vector<eltwise_test_params>{
    // There's no optimized eltwise kernel yet for fsv32 layout that supports fused_ops
    // So only activation is fused via legacy mechanism
    eltwise_test_params{ CASE_ELTWISE_FP16_4, 4, 5 },
}));

class eltwise_fp32_fsv4 : public EltwiseFusingTest {};
TEST_P(eltwise_fp32_fsv4, add) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        data("add_data", get_mem(get_per_channel_layout(p), -10, 10)),
        eltwise("eltwise", { input_info("input"), input_info("input2") }, p.mode, p.default_type),
        eltwise("add", { input_info("eltwise"), input_info("add_data") }, eltwise_mode::sum),
        activation("activation", input_info("add"), activation_func::negative),
        reorder("out", input_info("activation"), p.default_format, data_types::f32)
    );
    // Activation won't be fused because onednn doesn't support negative activation
    if (engine.get_device_info().supports_immad)
        p.expected_fused_primitives++;

    implementation_desc eltw_impl = { format::b_fs_yx_fsv4, "eltwise_b_fs_yx_fsv4" };
    bo_fused.set_option(build_option::force_implementations({ { "eltwise", eltw_impl } }));

    tolerance = 1e-5f;
    execute(p);
}

TEST_P(eltwise_fp32_fsv4, add_per_element) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        data("add_data", get_mem(get_input_layout(p), -10, 10)),
        eltwise("eltwise", { input_info("input"), input_info("input2") }, p.mode, p.default_type),
        eltwise("add", { input_info("eltwise"), input_info("add_data") }, eltwise_mode::sum),
        activation("activation", input_info("add"), activation_func::negative),
        reorder("out", input_info("activation"), p.default_format, data_types::f32)
    );
    // Activation won't be fused because onednn doesn't support negative activation
    if (engine.get_device_info().supports_immad)
        p.expected_fused_primitives++;

    implementation_desc eltw_impl = { format::b_fs_yx_fsv4, "eltwise_b_fs_yx_fsv4" };
    bo_fused.set_option(build_option::force_implementations({ { "eltwise", eltw_impl } }));

    tolerance = 1e-5f;
    execute(p);
}

INSTANTIATE_TEST_SUITE_P(fusings_gpu, eltwise_fp32_fsv4, ::testing::ValuesIn(std::vector<eltwise_test_params>{
    eltwise_test_params{ CASE_ELTWISE_FP32_5, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_FP32_6, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_I8_4,   3, 5 },
    eltwise_test_params{ CASE_ELTWISE_U8_4,   3, 5 },
}));

class eltwise_fp32_fused_prims : public EltwiseFusingTest {};
TEST_P(eltwise_fp32_fused_prims, scale_activation) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        data("scale_data", get_mem(get_per_channel_layout(p), -10, 10)),
        eltwise("eltwise", { input_info("input"), input_info("input2") }, p.mode, p.default_type),
        eltwise("scale", { input_info("eltwise"), input_info("scale_data") }, eltwise_mode::prod, p.default_type),
        activation("activation", input_info("scale"), activation_func::abs),
        reorder("out", input_info("activation"), p.default_format, data_types::f32)
    );

    tolerance = 1e-5f;
    execute(p);
}

TEST_P(eltwise_fp32_fused_prims, eltwise_activation) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        data("eltwise_data", get_mem(get_input_layout2(p), -10, 10)),
        eltwise("eltwise1", { input_info("input"), input_info("input2") }, p.mode, data_types::f32),
        eltwise("eltwise2", { input_info("eltwise1"), input_info("eltwise_data") }, eltwise_mode::prod, p.default_type),
        activation("activation", input_info("eltwise2"), activation_func::abs),
        reorder("out", input_info("activation"), p.default_format, data_types::f32)
    );

    tolerance = 1e-5f;
    execute(p);
}

TEST_P(eltwise_fp32_fused_prims, eltwise_activation_with_broadcast) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        data("eltwise_data", get_mem(get_per_channel_layout(p), -10, 10)),
        eltwise("eltwise1", { input_info("input"), input_info("input2") }, p.mode, p.default_type),
        eltwise("eltwise2", { input_info("eltwise1"), input_info("eltwise_data") }, eltwise_mode::prod, p.default_type),
        activation("activation", input_info("eltwise2"), activation_func::abs),
        reorder("out", input_info("activation"), p.default_format, data_types::f32)
    );

    tolerance = 1e-5f;
    execute(p);
}

INSTANTIATE_TEST_SUITE_P(fusings_gpu, eltwise_fp32_fused_prims, ::testing::ValuesIn(std::vector<eltwise_test_params>{
    eltwise_test_params{ CASE_ELTWISE_FP16_1, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_FP16_2, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_FP16_3, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_FP32_1, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_FP32_2, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_FP32_3, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_FP32_FP16_1, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_FP32_FP16_2, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_FP32_FP16_3, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_FP16_FP32_1, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_FP16_FP32_2, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_FP16_FP32_3, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_I8_FP32_1, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_I8_FP32_2, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_I8_FP32_3, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_U8_FP32_1, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_U8_FP32_2, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_U8_FP32_3, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_I8_FP16_1, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_I8_FP16_2, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_I8_FP16_3, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_U8_FP16_1, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_U8_FP16_2, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_U8_FP16_3, 3, 5 },
    // fsv4
    eltwise_test_params{ CASE_ELTWISE_FP32_5, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_FP32_6, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_I8_4, 3, 5 },
    eltwise_test_params{ CASE_ELTWISE_U8_4, 3, 5 },
}));

class eltwise_fp32_scale : public EltwiseFusingTest {};
TEST_P(eltwise_fp32_scale, 6d) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        data("scale_data", get_mem(get_per_channel_layout(p), -10, 10)),
        eltwise("eltwise", { input_info("input"), input_info("input2") }, p.mode, p.default_type),
        eltwise("scale", { input_info("eltwise"), input_info("scale_data") }, eltwise_mode::prod, p.default_type),
        reorder("out", input_info("scale"), p.default_format, data_types::f32)
    );

    tolerance = 1e-5f;
    execute(p);
}

INSTANTIATE_TEST_SUITE_P(fusings_gpu, eltwise_fp32_scale, ::testing::ValuesIn(std::vector<eltwise_test_params>{
    eltwise_test_params{ CASE_ELTWISE_FP32_4, 3, 4 },
}));

class eltwise_fp16_byxf : public EltwiseFusingTest {};
TEST_P(eltwise_fp16_byxf, add) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        data("add_data", get_mem(get_per_channel_layout(p), -10, 10)),
        eltwise("eltwise", { input_info("input"), input_info("input2") }, p.mode, p.default_type),
        eltwise("add", { input_info("eltwise"), input_info("add_data") }, eltwise_mode::sum),
        activation("activation", input_info("add"), activation_func::negative),
        reorder("out", input_info("activation"), p.default_format, data_types::f32)
    );
    // Activation won't be fused because onednn doesn't support negative activation
    if (engine.get_device_info().supports_immad)
        p.expected_fused_primitives++;

    implementation_desc eltw_impl = { format::byxf, "generic_eltwise_ref" };
    bo_fused.set_option(build_option::force_implementations({ { "eltwise", eltw_impl } }));

    tolerance = 1e-5f;
    execute(p);
}

INSTANTIATE_TEST_SUITE_P(fusings_gpu, eltwise_fp16_byxf, ::testing::ValuesIn(std::vector<eltwise_test_params>{
    eltwise_test_params{ CASE_ELTWISE_FP16_6, 3, 5 }
}));

class eltwise_no_pitches_same_dims_quantize : public EltwiseFusingTest {};
TEST_P(eltwise_no_pitches_same_dims_quantize, quantize_f32_output) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        eltwise("eltwise", { input_info("input"), input_info("input2") }, p.mode, p.default_type),
        data("in_lo", get_mem(get_per_channel_layout(p), min_random, 0)),
        data("in_hi", get_mem(get_per_channel_layout(p), 1, max_random)),
        data("out_lo", get_mem(get_single_element_layout(p), -128)),
        data("out_hi", get_mem(get_single_element_layout(p), 127)),
        quantize("quantize", input_info("eltwise"), input_info("in_lo"), input_info("in_hi"),
                 input_info("out_lo"), input_info("out_hi"), 256, p.input_type),
        reorder("out", input_info("quantize"), p.default_format, data_types::f32)
    );

    tolerance = 1.f;
    execute(p);
}

INSTANTIATE_TEST_SUITE_P(fusings_gpu, eltwise_no_pitches_same_dims_quantize, ::testing::ValuesIn(std::vector<eltwise_test_params>{
    eltwise_test_params{ CASE_ELTWISE_FP16_1, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP16_2, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP16_3, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_1, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_2, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_3, 3, 4 },
}));

class eltwise_activation : public EltwiseFusingTest {};
TEST_P(eltwise_activation, basic) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        eltwise("eltwise", { input_info("input"), input_info("input2") }, p.mode, p.default_type),
        activation("activation", input_info("eltwise"), activation_func::relu, { 6.0f, 0.0f }),
        reorder("out", input_info("activation"), p.default_format, data_types::f32)
    );

    tolerance = 1e-5f;
    execute(p);
}

TEST_P(eltwise_activation, fp16_out) {
    auto p = GetParam();
    create_topologies(
        input_layout("input", get_input_layout(p)),
        input_layout("input2", get_input_layout2(p)),
        eltwise("eltwise", { input_info("input"), input_info("input2") }, p.mode, data_types::f16),
        activation("activation", input_info("eltwise"), activation_func::relu, { 6.0f, 0.0f }),
        reorder("out", input_info("activation"), p.default_format, data_types::f32)
    );

    tolerance = 1e-5f;
    execute(p);
}

INSTANTIATE_TEST_SUITE_P(fusings_gpu, eltwise_activation, ::testing::ValuesIn(std::vector<eltwise_test_params>{
    eltwise_test_params{ CASE_ELTWISE_FP16_1, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP16_2, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP16_3, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_1, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_2, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_3, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_FP16_1, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_FP16_2, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP32_FP16_3, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP16_FP32_1, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP16_FP32_2, 3, 4 },
    eltwise_test_params{ CASE_ELTWISE_FP16_FP32_3, 3, 4 }
}));
