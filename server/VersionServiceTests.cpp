// Testing...
#include "VersionService.h"

// Dependencies
#include <gtest/gtest.h>

class VersionServiceValidVersionsTest
    : public ::testing::TestWithParam<std::string> {};
TEST_P(VersionServiceValidVersionsTest, ValidatesMultipleValidVersionInputs) {
    std::string expected = GetParam();

    ncs::VersionService versionSvc(expected);
    std::string actual = versionSvc.getVersion();

    EXPECT_EQ(expected, actual);
}

INSTANTIATE_TEST_SUITE_P(ValidVersionTests, VersionServiceValidVersionsTest,
                         ::testing::Values("0.1.0dev", "0.1.0debug", "1.0.0",
                                           "2.4.42", "2.1.5-rc"));

class VersionServiceInvalidVersionsTest
    : public ::testing::TestWithParam<std::string> {};
TEST_P(VersionServiceInvalidVersionsTest,
       ValidatesMultipleInvalidVersionInputs) {
    std::string input = GetParam();

    EXPECT_THROW(ncs::VersionService versionSvc(input);
                 , ncs::VersionService::InvalidVersionError);
}

INSTANTIATE_TEST_SUITE_P(InvalidVersionTests, VersionServiceInvalidVersionsTest,
                         ::testing::Values("", "\0"));
