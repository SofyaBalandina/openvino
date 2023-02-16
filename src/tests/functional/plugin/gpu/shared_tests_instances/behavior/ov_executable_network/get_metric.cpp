// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "behavior/ov_executable_network/get_metric.hpp"
#include "behavior/ov_plugin/properties_tests.hpp"
#include "openvino/runtime/core.hpp"

using namespace ov::test::behavior;

using namespace InferenceEngine::PluginConfigParams;

namespace {
//
// Executable Network GetMetric
//

INSTANTIATE_TEST_SUITE_P(nightly_CompiledModelGetMetricTest,
                         CompiledModelGetMetricTest_OPTIMAL_NUMBER_OF_INFER_REQUESTS,
                         ::testing::Values("GPU", "MULTI:GPU", "HETERO:GPU", "AUTO:GPU,CPU", "BATCH:GPU"));

INSTANTIATE_TEST_SUITE_P(nightly_CompiledModelGetMetricTest,
                         CompiledModelGetMetricTest_SUPPORTED_CONFIG_KEYS,
                         ::testing::Values("GPU", "MULTI:GPU", "HETERO:GPU", "AUTO:GPU,CPU", "BATCH:GPU"));

INSTANTIATE_TEST_SUITE_P(nightly_CompiledModelGetMetricTest,
                         CompiledModelGetMetricTest_SUPPORTED_METRICS,
                         ::testing::Values("GPU", "MULTI:GPU", "HETERO:GPU", "AUTO:GPU,CPU", "BATCH:GPU"));

INSTANTIATE_TEST_SUITE_P(nightly_CompiledModelGetMetricTest,
                         CompiledModelGetMetricTest_NETWORK_NAME,
                         ::testing::Values("GPU", "MULTI:GPU", "HETERO:GPU", "AUTO:GPU,CPU", "BATCH:GPU"));

INSTANTIATE_TEST_SUITE_P(nightly_CompiledModelGetMetricTest,
                         CompiledModelGetMetricTest_ThrowsUnsupported,
                         ::testing::Values("GPU", "MULTI:GPU", "HETERO:GPU", "AUTO:GPU,CPU", "BATCH:GPU"));

const std::vector<std::tuple<std::string, std::pair<ov::AnyMap, std::string>>> GetMetricTest_ExecutionDevice_GPU = {
        {"GPU", std::make_pair(ov::AnyMap{}, "GPU.0")},
        {"GPU.0", std::make_pair(ov::AnyMap{}, "GPU.0")},
        {"BATCH:GPU", std::make_pair(ov::AnyMap{}, "GPU.0")}};

INSTANTIATE_TEST_SUITE_P(nightly_CompiledModelGetMetricTest,
                         CompiledModelGetMetricTest_EXEC_DEVICES,
                         ::testing::ValuesIn(GetMetricTest_ExecutionDevice_GPU),
                         OVCompileModelGetExecutionDeviceTests::getTestCaseName);

auto multiDevicePriorityConfigs = []() {
    return std::vector<ov::AnyMap>{{ov::device::priorities(CommonTestUtils::DEVICE_CPU)},
                                   {ov::device::priorities(CommonTestUtils::DEVICE_GPU)},
                                   {ov::device::priorities(CommonTestUtils::DEVICE_CPU, CommonTestUtils::DEVICE_GPU)}};
};

INSTANTIATE_TEST_SUITE_P(nightly_CompiledModelGetMetricTest,
                         CompiledModelGetMetricTest_DEVICE_PRIORITY,
                         ::testing::Combine(::testing::Values("MULTI", "AUTO"),
                                            ::testing::ValuesIn(multiDevicePriorityConfigs())));

auto multiModelPriorityConfigs = []() {
    return std::vector<ov::AnyMap>{{ov::hint::model_priority(ov::hint::Priority::HIGH)},
                                   {ov::hint::model_priority(ov::hint::Priority::MEDIUM)},
                                   {ov::hint::model_priority(ov::hint::Priority::LOW)}};
};

INSTANTIATE_TEST_SUITE_P(nightly_CompiledModelGetMetricTest,
                         CompiledModelGetMetricTest_MODEL_PRIORITY,
                         ::testing::Combine(::testing::Values("AUTO"),
                                            ::testing::ValuesIn(multiModelPriorityConfigs())),
                         CompiledModelGetMetricTest_MODEL_PRIORITY::getTestCaseName);


//
// Executable Network GetConfig / SetConfig
//

INSTANTIATE_TEST_SUITE_P(nightly_CompiledModelPropertyTest,
                         CompiledModelPropertyTest,
                         ::testing::Values("GPU"));

INSTANTIATE_TEST_SUITE_P(nightly_CompiledModelSetConfigTest,
                         CompiledModelSetConfigTest,
                         ::testing::Values("GPU"));

//
// Hetero Executable Network GetMetric
//

INSTANTIATE_TEST_SUITE_P(nightly_OVClassHeteroExecutableNetworlGetMetricTest,
                         OVClassHeteroExecutableNetworkGetMetricTest_SUPPORTED_CONFIG_KEYS,
                         ::testing::Values("GPU"));

INSTANTIATE_TEST_SUITE_P(nightly_OVClassHeteroExecutableNetworlGetMetricTest,
                         OVClassHeteroExecutableNetworkGetMetricTest_SUPPORTED_METRICS,
                         ::testing::Values("GPU"));

INSTANTIATE_TEST_SUITE_P(nightly_OVClassHeteroExecutableNetworlGetMetricTest,
                         OVClassHeteroExecutableNetworkGetMetricTest_NETWORK_NAME,
                         ::testing::Values("GPU"));

INSTANTIATE_TEST_SUITE_P(nightly_OVClassHeteroExecutableNetworlGetMetricTest,
                         OVClassHeteroExecutableNetworkGetMetricTest_TARGET_FALLBACK,
                         ::testing::Values("GPU"));

INSTANTIATE_TEST_SUITE_P(nightly_OVClassHeteroExecutableNetworlGetMetricTest,
                         OVClassHeteroExecutableNetworkGetMetricTest_EXEC_DEVICES,
                         ::testing::Values("GPU.0"));

} // namespace

