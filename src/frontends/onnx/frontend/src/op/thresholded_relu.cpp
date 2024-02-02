// Copyright (C) 2018-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "op/thresholded_relu.hpp"

#include "openvino/op/constant.hpp"
#include "openvino/op/convert.hpp"
#include "openvino/op/greater.hpp"
#include "openvino/op/multiply.hpp"

using namespace ov::op;

OPENVINO_SUPPRESS_DEPRECATED_START
namespace ngraph {
namespace onnx_import {
namespace op {
namespace set_1 {
ov::OutputVector thresholded_relu(const Node& node) {
    const auto data = node.get_ng_inputs().at(0);
    const double alpha = node.get_attribute_value<double>("alpha", 1.0);

    const auto alpha_node = v0::Constant::create(data.get_element_type(), ov::Shape{}, {alpha});

    const auto data_map =
        std::make_shared<v0::Convert>(std::make_shared<v1::Greater>(data, alpha_node), data.get_element_type());

    return {std::make_shared<v1::Multiply>(data, data_map)};
}

}  // namespace set_1

}  // namespace op

}  // namespace onnx_import

}  // namespace ngraph
OPENVINO_SUPPRESS_DEPRECATED_END
