// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "openvino/core/deprecated.hpp"
OPENVINO_SUPPRESS_DEPRECATED_START

#include "core/node.hpp"
#include "openvino/op/less.hpp"

namespace ngraph {
namespace onnx_import {
namespace op {
namespace set_1 {
inline ov::OutputVector less(const Node& node) {
    return {std::make_shared<ov::op::v1::Less>(node.get_ng_inputs().at(0), node.get_ng_inputs().at(1))};
}

}  // namespace set_1

}  // namespace op

}  // namespace onnx_import

}  // namespace ngraph
OPENVINO_SUPPRESS_DEPRECATED_END
