#include "assert.hpp"

#include "orion/core/logger.hpp"

#include <cstdio>
#include <cstdlib>

namespace orion
{
    // Logging channel to use for assertion failures
    static constexpr auto channel = "ASSERTIONS";

    static std::string format_assertion_msg(const char* expression, const char* file, int line)
    {
        return std::format("Assertion failed ({}:{}): '{}'", file, line, expression);
    }

    // We initialize it to a no-op function so we never end up invoking a null function pointer
    static AssertionHandler g_assertion_handler = +[](const char* expr, const char*, int) {};

    void invoke_assertion_handler(const char* expression, const char* file, int line)
    {
        g_assertion_handler(expression, file, line);
    }

    void DefaultAssertionPlugin::build(class App&)
    {
        g_assertion_handler = DefaultAssertionPlugin::invoke;
    }

    void DefaultAssertionPlugin::invoke(const char* expression, const char* file, int line)
    {
        const auto msg = format_assertion_msg(expression, file, line);
        if (orion::log::has_channel(channel)) {
            ORION_LOG_ERROR(channel, "{}", msg);
        } else {
            std::fputs(msg.c_str(), stderr);
        }
        std::abort();
    }

    AssertionFailure::AssertionFailure(const char* expression, const char* file, int line)
        : std::runtime_error(format_assertion_msg(expression, file, line))
        , m_expression(expression)
        , m_file(file)
        , m_line(line)
    {
    }

    void TestingAssertionPlugin::build(class App&)
    {
        g_assertion_handler = TestingAssertionPlugin::invoke;
    }

    void TestingAssertionPlugin::invoke(const char* expression, const char* file, int line)
    {
        throw AssertionFailure(expression, file, line);
    }
} // namespace orion
