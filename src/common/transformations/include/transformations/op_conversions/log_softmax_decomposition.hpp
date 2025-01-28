// Copyright (C) 2018-2025 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "openvino/pass/matcher_pass.hpp"
#include "transformations_visibility.hpp"

namespace ov {
namespace pass {

class TRANSFORMATIONS_API LogSoftmaxDecomposition;

}  // namespace pass
}  // namespace ov

/**
 * @ingroup ov_transformation_common_api
 * @brief LogSoftmaxDecomposition transformation into sub-graph x - log(reduce_sum(exp(x), axis)).
 */
class ov::pass::LogSoftmaxDecomposition : public ov::pass::MatcherPass {
public:
    OPENVINO_MATCHER_PASS_RTTI("LogSoftmaxDecomposition");
    LogSoftmaxDecomposition();
};
