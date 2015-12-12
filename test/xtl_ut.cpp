/// @file      xtl_ut.cpp
/// @brief     eXtra Template Library (XTL) unit tests.
/// @copyright Licensed under the MIT License.
/// @author    Rostislav Ostapenko (rostislav.ostapenko@gmail.com)
/// @date      20-Apr-2013

#include "xtl/ut.hpp"

XTL_UT_TEST_SUITE_BEGIN(MySuite)

    XTL_UT_SETUP()
    {
        std::cout << "setup" << std::endl;
    }

    XTL_UT_TEARDOWN()
    {
        std::cout << "teardown" << std::endl;
    }

    XTL_UT_TEST_CASE(test1)
    {
        std::cout << "Running test1" << std::endl;
    }

    XTL_UT_TEST_CASE(test2)
    {
        std::cout << "Running test2" << std::endl;
    }

    XTL_UT_TEST_CASE(test3)
    {
        std::cout << "Running test3" << std::endl;
    }

XTL_UT_TEST_SUITE_END(MySuite)

int main(int argc, char* argv[])
{
    return XTL_UT_RUN(argc, argv);
}
