// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <openvino/pass/graph_rewrite.hpp>
#include <transformations_visibility.hpp>

/**
 * @ingroup ie_transformation_common_api
 * @brief Resolves transpose_b key from MatMul operation if corresponding input is constant or FakeQuantize by inserting
 * Transpose
 */

namespace ov {
namespace pass {

class TRANSFORMATIONS_API MatMulConstTransposesExtraction : public MatcherPass {
public:
    OPENVINO_RTTI("TRANSFORMATIONS_API", "0");
    MatMulConstTransposesExtraction();
};

}  // namespace pass
}  // namespace ov

namespace ngraph {
namespace pass {
using ov::pass::MatMulConstTransposesExtraction;
}  // namespace pass
}  // namespace ngraph
