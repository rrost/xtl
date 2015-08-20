/// @file      xtl_ut.cpp
/// @brief     eXtra Template Library (XTL) unit tests.
/// @copyright Licensed under the MIT License.
/// @author    Rostislav Ostapenko (rostislav.ostapenko@gmail.com)
/// @date      20-Apr-2013

#include "xtl/ut.hpp"

XTL_UT_TEST_SUITE_BEGIN(MySuite)

    XTL_UT_TEST_CASE(test1)
    {}

    XTL_UT_TEST_CASE(test2)
    {}

    XTL_UT_TEST_CASE(test3)
    {}

XTL_UT_TEST_SUITE_END(MySuite)

int main(int argc, char* argv[])
{
    xtl::ut::test_suite_manager::defaultManager().run(argc, argv);
    return 0;
}
