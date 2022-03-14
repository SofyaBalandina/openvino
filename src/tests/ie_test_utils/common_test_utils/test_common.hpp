// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <gtest/gtest.h>

#include <string>
#include "test_assertions.hpp"

namespace CommonTestUtils {

class TestsCommon : virtual public ::testing::Test {
public:
    static void PrintMemUsage(std::string comment);
protected:
    TestsCommon();
    ~TestsCommon() override;

    static std::string GetTimestamp();
    std::string GetTestName() const;
};

}  // namespace CommonTestUtils
