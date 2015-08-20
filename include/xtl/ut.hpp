/// @file      ut.hpp
/// @brief     eXtra Template Library (XTL) - micro unit tests framework implementation.
/// @copyright Licensed under the MIT License.
/// @author    Rostislav Ostapenko (rostislav.ostapenko@gmail.com)
/// @date      20-Apr-2011

#pragma once

#include <cassert>

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace xtl
{

    namespace ut
    {
        class test_suite_manager;

        using string_type = std::string;

        template <typename T>
        using ref_vector = std::vector < std::reference_wrapper<T> > ;

        struct global_context_data
        {
            using arguments_container = std::vector < string_type > ;
            arguments_container arguments;
        };

        struct test_suite_itf
        {
            virtual void run() = 0;
        };

        namespace details
        {

            template <class T>
            struct test_case_registry
            {
                using test_suite_type = T;
                using this_type = test_case_registry < T > ;
                using test_case_func = void(test_suite_type::*)();

                struct test_case;
                using test_cases = ref_vector < test_case > ;

                static test_cases& storage()
                {
                    static test_cases cases;
                    return cases;
                }

                static void add_case(test_case& new_case)
                {
                    auto& cases = storage();

                    const auto is_exists = [&new_case](const test_case& v)
                    { return new_case.func_ == v.func_; };

                    if(std::find_if(std::begin(cases), std::end(cases), is_exists)
                        == std::end(cases))
                    {
                        cases.push_back(new_case);
                    }
                }

                struct test_case
                {
                    const test_case_func func_;
                    const string_type name_;

                    test_case(const test_case&) = delete;
                    test_case& operator=(const test_case&) = delete;

                    test_case(test_case_func func, const string_type& name)
                        : func_(func)
                        , name_(name)
                    {
                        this_type::add_case(*this);
                    }

                    const test_case_func& func() const
                    {
                        return func_;
                    }

                    const string_type& name() const
                    {
                        return name_;
                    }
                };
            };

        } // namespace details

        template <class T>
        class test_suite
            : protected details::test_case_registry<T>
            , public test_suite_itf
        {
            const string_type name_;
            const global_context_data& global_context_;

            test_suite(const test_suite&) = delete;
            test_suite& operator=(const test_suite&) = delete;

        protected:

            using base_type = details::test_case_registry<T> ;
            using test_case = typename base_type::test_case;
            using test_case_func = typename base_type::test_case_func;
            
            const string_type& name() const
            {
                return name_;
            }

            const global_context_data& global_context() const
            {
                return global_context_;
            }

            test_suite(const string_type& name);

        public:

            virtual void run() override
            {
                std::cout << "run: " << name() << std::endl;

                for(test_case& t : base_type::storage())
                    std::cout << " - " << t.name() << std::endl;
            }

        };

        class test_suite_manager
        {
            const string_type name_ = "Default XTL UT Manager";

            global_context_data context_;

            using test_suites = ref_vector < test_suite_itf > ;
            test_suites test_suites_;

            test_suite_manager()
            {}

            test_suite_manager(const test_suite_manager&) = delete;
            test_suite_manager& operator=(const test_suite_manager&) = delete;

            void parse_cmd_args(int argc, char* argv[])
            {
                argc; argv;
            }

        public:

            const string_type& name() const
            {
                return name_;
            }

            const global_context_data& context() const
            {
                return context_;
            }

            void add_suite(test_suite_itf& suite)
            {
                test_suites_.push_back(suite);
            }

            void run(int argc, char* argv[])
            {
                parse_cmd_args(argc, argv);

                for(test_suite_itf& test : test_suites_) test.run();
            }

            static test_suite_manager& defaultManager()
            {
                static test_suite_manager manager;
                return manager;
            }
        };

        template <class T>
        inline test_suite<T>::test_suite(const std::string& name)
            : name_(name)
            , global_context_(test_suite_manager::defaultManager().context())
        {
            test_suite_manager::defaultManager().add_suite(*this);
        }

#define XTL_UT_TEST_CASE(name) \
    static test_case_func name##_ptr() { return &name; } \
    const test_case name##_case = { name##_ptr(), #name }; \
    void name()

#define XTL_UT_TEST_CASE_DECLARE(name) XTL_UT_TEST_CASE(name);

#define XTL_UT_TEST_CASE_DEFINE(name) void name()

#define XTL_UT_TEST_SUITE_BEGIN(name) \
    class name: xtl::ut::test_suite<name> { \
    public: \
        using base_type = xtl::ut::test_suite<name>; \
        name(): base_type(#name) {} \
    private: \
        name(const name&) = delete; \
        name& operator=(const name&) = delete;

#define XTL_UT_TEST_SUITE_END(name) \
    } name##_suite;

} // namespace ut

} // namespace xtl
