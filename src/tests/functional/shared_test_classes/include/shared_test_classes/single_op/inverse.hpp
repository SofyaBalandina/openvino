// Copyright (C) 2018-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <string>
#include <tuple>

#include "openvino/core/type/element_type.hpp"
#include "shared_test_classes/base/ov_subgraph.hpp"

namespace ov {
namespace test {

using InverseTestParams = typename std::tuple<ov::Shape,          // input shape
                                              ov::element::Type,  // element type
                                              bool,               // adjoint
                                              bool,               // test_static
                                              unsigned int,       // seed
                                              std::string         // device_name
                                              >;

class InverseLayerTest : public testing::WithParamInterface<InverseTestParams>, virtual public SubgraphBaseTest {
public:
    static std::string getTestCaseName(const testing::TestParamInfo<InverseTestParams>& obj);

protected:
    void SetUp() override;

    void generate_inputs(const std::vector<ov::Shape>& target_shapes) override;

private:
    unsigned int m_seed;
};
}  // namespace test
}  // namespace ov
