// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "arithmetic_ops.hpp"

using Type = ::testing::Types<ngraph::op::v1::Subtract>;

INSTANTIATE_TYPED_TEST_SUITE_P(type_prop_subtract, ArithmeticOperator, Type);
