/// @file      ut.hpp
/// @brief     eXtra Template Library (XTL) - micro unit tests framework implementation.
/// @copyright Licensed under the MIT License.
/// @author    Rostislav Ostapenko (rostislav.ostapenko@gmail.com)
/// @date      20-Apr-2011

#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <algorithm>
#include <exception>
#include <functional>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace xtl
{
namespace ut
{
    enum class result_type
    {
        success,
        fail,
        error,
        exception,
        warning
    };

    inline const char* const to_string(result_type value)
    {
        switch (value)
        {
            case result_type::success:    return "OK";
            case result_type::fail:       return "FAIL";
            case result_type::error:      return "ERROR";
            case result_type::exception:  return "EXCEPTION";
            case result_type::warning:    return "WARNING";
            default:                      return "UNKNOWN";
        }
    }

    class test_suite_manager;

    using string_type = std::string;

    template <typename T>
    using ref_vector = std::vector<std::reference_wrapper<T> >;

    struct global_context_data
    {
        using arguments_container = std::vector<string_type>;
        arguments_container arguments;
    };

    struct test_result
    {
        result_type type;
        unsigned int line;
        string_type file_name;
        string_type suite_name;
        string_type case_name;
        string_type function_name;
        string_type message;
        std::thread::id thread_id;

        static test_result make_result(
            result_type res_type,
            unsigned int ln,
            const string_type& f_name,
            const string_type& s_name,
            const string_type& c_name,
            const string_type& fn_name,
            const string_type& msg)
        {
            const test_result res =
                { res_type, ln, f_name, s_name, c_name, fn_name, msg, std::this_thread::get_id() };
            return res;
        }

        static test_result make_success(
            unsigned int ln,
            const string_type& f_name,
            const string_type& s_name,
            const string_type& c_name)
        {
            return make_result(result_type::success, ln, f_name, s_name, c_name, {}, {});
        }

        static test_result make_fail(
            unsigned int ln,
            const string_type& f_name,
            const string_type& s_name,
            const string_type& c_name,
            const string_type& fn_name = {},
            const string_type& msg = {})
        {
            return make_result(result_type::fail, ln, f_name, s_name, c_name, fn_name, msg);
        }

        static test_result make_error(
            unsigned int ln,
            const string_type& f_name,
            const string_type& s_name,
            const string_type& c_name,
            const string_type& fn_name = {},
            const string_type& msg = {})
        {
            return make_result(result_type::error, ln, f_name, s_name, c_name, fn_name, msg);
        }

        static test_result make_exception(
            unsigned int ln,
            const string_type& f_name,
            const string_type& s_name,
            const string_type& c_name,
            const string_type& fn_name = {},
            const string_type& msg = {})
        {
            return make_result(result_type::exception, ln, f_name, s_name, c_name, fn_name, msg);
        }

        static test_result make_warning(
            unsigned int ln,
            const string_type& f_name,
            const string_type& s_name,
            const string_type& c_name,
            const string_type& fn_name = {},
            const string_type& msg = {})
        {
            return make_result(result_type::warning, ln, f_name, s_name, c_name, fn_name, msg);
        }

        string_type to_string() const
        {
            return string_type(xtl::ut::to_string(type)) + " " +
                (has_function_name() ? 
                    full_case_name() + ", " + function_name + "()" :
                    full_case_name()) +
                " at " + file_name + ", line " + std::to_string(line) + 
                (message.empty() ? "" : " - " +  message);
        }

        const string_type full_case_name() const
        {
            return suite_name + "::" + case_name;
        }

        bool has_function_name() const
        {
            return !function_name.empty() && function_name != full_case_name();
        }

    };

    struct test_suite_itf
    {
        virtual const string_type& suite_name() const = 0;
        virtual const string_type& case_name() const = 0;
        virtual void run() = 0;
    };

    static inline const char* const to_pchar(const string_type& str)
    {
        return str.c_str();
    }

    struct fatal_error: std::exception
    {
        fatal_error(const string_type& msg): std::exception(to_pchar(msg)) {}
    };

    static inline void raise_error(const string_type& msg)
    {
        throw fatal_error(msg);
    }

    struct abort_exception {};

    static inline void abort_test()
    {
        throw abort_exception();
    }

namespace details
{

    template <class T>
    struct test_case_registry
    {
        using test_suite_type = T;
        using this_type = test_case_registry<T>;
        using test_case_func = void(test_suite_type::*)();

        class test_case;
        using test_cases = ref_vector<test_case>;

        static test_cases& storage()
        {
            static test_cases cases;
            return cases;
        }

        static void add_case(test_case& new_case)
        {
            auto& cases = storage();

            const auto already_exists = [&new_case](const test_case& v)
                { return new_case.func() == v.func(); };

            if (std::find_if(std::begin(cases), std::end(cases), already_exists)
                == std::end(cases))
            {
                cases.push_back(new_case);
            }
        }

        class test_case
        {
            const test_case_func func_;
            const string_type name_;
            const string_type file_;
            const int line_;

        public:

            test_case(const test_case&) = delete;
            test_case& operator=(const test_case&) = delete;

            test_case(
                test_case_func func,
                const string_type& name,
                const string_type& file,
                const int line)
                : func_(func)
                , name_(name)
                , file_(file)
                , line_(line)
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

            const string_type& file() const
            {
                return file_;
            }

            const int line() const
            {
                return line_;
            }
        };
    };

} // namespace details

    template <class T>
    class test_suite
        : protected details::test_case_registry<T>
        , public test_suite_itf
    {
    protected:

        test_suite(const string_type& name);

        using base_type = details::test_case_registry<T>;
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

        const test_case* current_case() const
        {
            return current_case_;
        }

        test_suite_type& suite()
        {
            return *static_cast<test_suite_type*>(this);
        }

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
                if (init_deinit_.setup_)
                    (suite_.*init_deinit_.setup_)();
            }

            ~suite_initializer()
            {
                if (init_deinit_.teardown_)
                    (suite_.*init_deinit_.teardown_)();
            }
        };

        void add_success(const test_case& test);
        void add_error(const test_case& test, const string_type& msg = {});
        void add_exception(const test_case& test, const string_type& msg = {});

    public:

        test_suite(const test_suite&) = delete;
        test_suite& operator=(const test_suite&) = delete;

        virtual const string_type& suite_name() const override
        {
            return name();
        }

        virtual const string_type& case_name() const override
        {
            if (!current_case_)
            {
                raise_error(
                    "[XTL UT] Error getting current test case name: "
                    "no unit test currently is running.");
            }

            return current_case_->name();
        }

        virtual void run() override
        {
            suite_initializer init(init_deinit_suite_, suite());
            for (test_case& test : base_type::storage())
            {
                try
                {
                    current_case_ = &test;
                    const auto func = test.func();
                    (suite().*func)();

                    add_success(test);
                }
                catch (fatal_error& e)
                {
                    // Re-throw fatal error to upper level.
                    add_error(test, e.what());
                    throw e;
                }
                catch (abort_exception&)
                {
                    // Nothing to do, test case just aborted.
                }
                catch (std::exception& e)
                {
                    add_exception(test, e.what());
                }
                catch (...)
                {
                    add_exception(test, "Unhandled exception");
                }
            }

            current_case_ = nullptr;
        }

    private:
        const string_type name_;
        const global_context_data& global_context_;
        const test_case* current_case_ = nullptr;
    };

    class test_suite_manager
    {
        const string_type name_ = "XTL UT Manager";

        global_context_data context_;

        using test_suites = ref_vector<test_suite_itf>;
        test_suites test_suites_;

        std::mutex lock_results_;
        std::vector<test_result> results_;

        const test_suite_itf* current_suite_ = nullptr;

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

        const test_suite_itf& current_suite() const
        {
            if (!current_suite_)
            {
                raise_error(
                    "[XTL UT] Error getting current suite: "
                    "no unit test currently is running.");
            }

            return *current_suite_;
        }

        const global_context_data& context() const
        {
            return context_;
        }

        void add_suite(test_suite_itf& suite)
        {
            test_suites_.push_back(suite);
        }

        void add_result(test_result&& result)
        {
            if (!current_suite_)
            {
                string_type msg =
                    "[XTL UT] Error adding test result: "
                    "no unit test currently is running.";
                msg += " [";
                msg += result.to_string();
                msg += "]";
                raise_error(msg);
            }

            {
                std::lock_guard<std::mutex> lock(lock_results_);
                results_.push_back(std::move(result));
            }
        }

        auto& log()
        {
            // TODO: replace by multi-target logging.
            return std::cout;
        }

        int process_results()
        {
            for (const auto& r : results_)
                log() << r.to_string() << std::endl;

            return EXIT_SUCCESS;
        }

        int run(int argc, char* argv[])
        {
            try
            {
                parse_cmd_args(argc, argv);

                for (test_suite_itf& test : test_suites_)
                {
                    current_suite_ = &test;
                    test.run();
                }

                current_suite_ = nullptr;
            }
            catch (fatal_error&)
            {
                // Nothing to do, just abort testing.
            }
            catch (...)
            {
                results_.push_back(
                    test_result::make_exception(
                        0,
                        __FILE__,
                        "test_suite_manager",
                        __FUNCTION__,
                        {},
                        "Unhandled exception"));
            }

            return process_results();
        }

        static test_suite_manager& instance()
        {
            static test_suite_manager manager;
            return manager;
        }
    };

    template <class T>
    inline test_suite<T>::test_suite(const string_type& name)
        : name_(name)
        , global_context_(test_suite_manager::instance().context())
    {
        test_suite_manager::instance().add_suite(*this);
    }

    template <class T>
    inline void test_suite<T>::add_success(const test_case& test)
    {
        test_suite_manager::instance().add_result(
            test_result::make_success(test.line(), test.file(), name(), test.name()));
    }

    template <class T>
    inline void test_suite<T>::add_error(const test_case& test, const string_type& msg)
    {
        test_suite_manager::instance().add_result(
            test_result::make_error(test.line(), test.file(), name(), test.name(), {}, msg));
    }

    template <class T>
    inline void test_suite<T>::add_exception(const test_case& test, const string_type& msg)
    {
        test_suite_manager::instance().add_result(
            test_result::make_exception(test.line(), test.file(), name(), test.name(), {}, msg));
    }

    template <class T>
    class lazy_instance
    {
        const T* const p_;
    public:
        lazy_instance(): p_(new T()) {}
        ~lazy_instance() { delete p_; }
    };

#define XTL_UT_SETUP() \
    static init_func setup_ptr() { return &setup; } \
    const assign_member<init_func> setup_init = { init_deinit_suite_.setup_, setup_ptr() }; \
    void setup()

#define XTL_UT_TEARDOWN() \
    static deinit_func teardown_ptr() { return &teardown; } \
    const assign_member<deinit_func> teardown_init = { init_deinit_suite_.teardown_, teardown_ptr() }; \
    void teardown()

#define XTL_UT_CASE(name) \
    static test_case_func name##_ptr() { return &name; } \
    const test_case name##_case = { name##_ptr(), #name, __FILE__, __LINE__ }; \
    void name()

#define XTL_UT_CASE_DECLARE(name) XTL_UT_TEST_CASE(name);

#define XTL_UT_CASE_DEFINE(suite, name) void suite::name()

#define XTL_UT_SUITE(name) \
template <class T> struct name##_test_suite: xtl::ut::test_suite<T> \
{ \
    using base_type = xtl::ut::test_suite<T>; \
    name##_test_suite(): base_type(#name) {} \
    name##_test_suite(const name##_test_suite&) = delete; \
    name##_test_suite& operator=(const name##_test_suite&) = delete; \
}; \
class name; \
xtl::ut::lazy_instance<name> name##_inst; \
class name: name##_test_suite<name> \

#define XTL_UT_RUN(argc, argv) xtl::ut::test_suite_manager::instance().run(argc, argv)

#define XTL_UT_ASSERT(exp, fatal) \
if (!(exp)) \
{ \
    using namespace xtl::ut; \
    const auto& s = test_suite_manager::instance().current_suite(); \
    test_suite_manager::instance().add_result( \
        test_result::make_fail(__LINE__, __FILE__, s.suite_name(), s.case_name(), \
            __FUNCTION__, "Assertion failed: " #exp)); \
    if ((fatal)) abort_test(); \
} \

#define XTL_UT_REQUIRE(exp) XTL_UT_ASSERT(exp, true)
#define XTL_UT_CHECK(exp) XTL_UT_ASSERT(exp, false)

} // namespace ut
} // namespace xtl
