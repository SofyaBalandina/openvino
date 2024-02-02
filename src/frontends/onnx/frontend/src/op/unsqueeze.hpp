// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "openvino/core/deprecated.hpp"
OPENVINO_SUPPRESS_DEPRECATED_START

#include "core/node.hpp"

namespace ngraph {
namespace onnx_import {
namespace op {
namespace set_1 {
ov::OutputVector unsqueeze(const Node& node);

}  // namespace set_1

namespace set_13 {
ov::OutputVector unsqueeze(const Node& node);

}  // namespace set_13
}  // namespace op

}  // namespace onnx_import

}  // namespace ngraph
OPENVINO_SUPPRESS_DEPRECATED_END
