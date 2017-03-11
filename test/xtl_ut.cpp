/// @file      xtl_ut.cpp
/// @brief     eXtra Template Library (XTL) unit tests.
/// @copyright Licensed under the MIT License.
/// @author    Rostislav Ostapenko (rostislav.ostapenko@gmail.com)
/// @date      20-Apr-2013

#include "xtl/ut.hpp"
#include "xtl/c_array.hpp"

XTL_UT_SUITE(MySuite)
{

    XTL_UT_SETUP()
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    XTL_UT_TEARDOWN()
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    XTL_UT_CASE(test1)
    {
        std::cout << "Test case: " << __FUNCTION__ << std::endl;
    }

    XTL_UT_CASE(test2)
    {
        std::cout << "Test case: " << __FUNCTION__ << std::endl;
    }

    XTL_UT_CASE(test3)
    {
        std::cout << "Test case: " << __FUNCTION__ << std::endl;
    }

};

XTL_UT_SUITE(MySuite2)
{

    XTL_UT_CASE(test1)
    {
        std::cout << "Test case: " << __FUNCTION__ << " at " 
            << current_case()->file() << ", ln " << current_case()->line() << std::endl;
        throw 0;
    }

    XTL_UT_CASE(test2)
    {
        std::cout << "Test case: " << __FUNCTION__ << std::endl;
    }

    XTL_UT_CASE(test3)
    {
        std::cout << "Test case: " << __FUNCTION__ << std::endl;
    }

};

int main(int argc, char* argv[])
{
    return XTL_UT_RUN(argc, argv);
}
