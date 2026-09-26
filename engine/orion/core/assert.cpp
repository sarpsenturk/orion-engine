#include "assert.hpp"

#include "orion/core/logger.hpp"

#include <cstdio>
#include <cstdlib>

namespace orion
{
    static void default_assertion_handler(const char* expression, const char* file, int line)
    {
        const auto msg = fmt::format("Assertion failed ({}:{}): '{}'", file, line, expression);
        if (auto logger = orion::Logger::get(LogChannel::Assertions)) {
            logger->error(msg);
        } else {
            std::fputs(msg.c_str(), stderr);
        }
        std::abort();
    }

    static void (*g_assertion_handler)(const char*, const char*, int) = default_assertion_handler;

    void set_assertion_handler(void (*handler)(const char*, const char*, int))
    {
        g_assertion_handler = handler;
    }

    void invoke_assertion_handler(const char* expression, const char* file, int line)
    {
        g_assertion_handler(expression, file, line);
    }
} // namespace orion
