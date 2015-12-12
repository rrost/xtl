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

                    const auto already_exists = [&new_case](const test_case& v)
                    { return new_case.func_ == v.func_; };

                    if(std::find_if(std::begin(cases), std::end(cases), already_exists)
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

        protected:
            using base_type = details::test_case_registry<T> ;
            using test_suite_type = typename base_type::test_suite_type;
            using test_case = typename base_type::test_case;
            using test_case_func = typename base_type::test_case_func;
            using init_func = void(test_suite_type::*)();
            using deinit_func = void(test_suite_type::*)();

            struct init_deinit
            {
                init_func setup_ = {};
                deinit_func teardown_ = {};
            }
            init_deinit_suite_;

            template <typename T>
            struct assign_member
            {
                assign_member(T& obj, const T value)
                {
                    obj = value;
                }
            };

            const string_type& name() const
            {
                return name_;
            }

            const global_context_data& global_context() const
            {
                return global_context_;
            }

            test_suite_type& suite()
            {
                return *static_cast<test_suite_type*>(this);
            }

            test_suite(const string_type& name);

            struct suite_initializer
            {
                const init_deinit& init_deinit_;
                test_suite_type& suite_;

                suite_initializer(const suite_initializer&) = delete;
                suite_initializer& operator=(const suite_initializer&) = delete;

                suite_initializer(const init_deinit& funcs, test_suite_type& suite)
                    : init_deinit_(funcs)
                    , suite_(suite)
                {
                    if(init_deinit_.setup_)
                        (suite_.*init_deinit_.setup_)();
                }

                ~suite_initializer()
                {
                    if(init_deinit_.teardown_)
                        (suite_.*init_deinit_.teardown_)();
                }
            };

        public:

            test_suite(const test_suite&) = delete;
            test_suite& operator=(const test_suite&) = delete;

            virtual void run() override
            {
                std::cout << "run: " << name() << std::endl;

                suite_initializer init(init_deinit_suite_, suite());
                for(test_case& test : base_type::storage())
                {
                    std::cout << " - " << test.name() << std::endl;
                    const auto func = test.func();
                    (suite().*func)();
                }
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

            int run(int argc, char* argv[])
            {
                parse_cmd_args(argc, argv);
                for(test_suite_itf& test : test_suites_) test.run();
                return 0;
            }

            static test_suite_manager& instance()
            {
                static test_suite_manager manager;
                return manager;
            }
        };

        template <class T>
        inline test_suite<T>::test_suite(const std::string& name)
            : name_(name)
            , global_context_(test_suite_manager::instance().context())
        {
            test_suite_manager::instance().add_suite(*this);
        }

#define XTL_UT_SETUP() \
    static init_func setup_ptr() { return &setup; } \
    const assign_member<init_func> setup_init = { init_deinit_suite_.setup_, setup_ptr() }; \
    void setup()

#define XTL_UT_TEARDOWN() \
    static deinit_func teardown_ptr() { return &teardown; } \
    const assign_member<deinit_func> teardown_init = { init_deinit_suite_.teardown_, teardown_ptr() }; \
    void teardown()

#define XTL_UT_TEST_CASE(name) \
    static test_case_func name##_ptr() { return &name; } \
    const test_case name##_case = { name##_ptr(), #name }; \
    void name()

#define XTL_UT_TEST_CASE_DECLARE(name) XTL_UT_TEST_CASE(name);

#define XTL_UT_TEST_CASE_DEFINE(suite, name) void suite::name()

#define XTL_UT_TEST_SUITE_BEGIN(name) \
    class name: xtl::ut::test_suite<name> { \
    public: \
        using base_type = xtl::ut::test_suite<name>; \
        name(): base_type(#name) {} \
        name(const name&) = delete; \
        name& operator=(const name&) = delete; \
    private:

#define XTL_UT_TEST_SUITE_END(name) \
    } name##_suite;

#define XTL_UT_RUN(argc, argv) xtl::ut::test_suite_manager::instance().run(argc, argv)

} // namespace ut

} // namespace xtl
