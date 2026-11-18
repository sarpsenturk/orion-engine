#pragma once

// Assertions can be enabled/disabled by defining ORION_ENABLE_ASSERTIONS to 1 or 0 respectively,
// before including this header file.
// If ORION_ENABLE_ASSERTIONS is not defined, it will be automatically set based on the build configuration:
// - In debug builds (when ORION_DEBUG is defined), assertions will be enabled (ORION_ENABLE_ASSERTIONS = 1).
// - In release builds (when ORION_DEBUG is not defined), assertions will be disabled (ORION_ENABLE_ASSERTIONS = 0).
#ifndef ORION_ENABLE_ASSERTIONS
    #ifdef ORION_DEBUG
        #define ORION_ENABLE_ASSERTIONS 1
    #else
        #define ORION_ENABLE_ASSERTIONS 0
    #endif
#endif

#if ORION_ENABLE_ASSERTIONS
    #define ORION_ASSERT(expression)                                                \
        do {                                                                        \
            if (!(expression)) {                                                    \
                ::orion::invoke_assertion_handler(#expression, __FILE__, __LINE__); \
            }                                                                       \
        } while (0)
#else
    #define ORION_ASSERT(expression) ((void)0)
#endif

#include <stdexcept>

namespace orion
{
    // Assertion handler function type
    using AssertionHandler = void (*)(const char* expression, const char* file, int line);

    // Invoke the enabled assertion handler
    void invoke_assertion_handler(const char* expression, const char* file, int line);

    // Logs the assertion failure and calls std::abort()
    struct DefaultAssertionPlugin {
        void build(class App& app);
        static void invoke(const char* expression, const char* file, int line);
    };

    class AssertionFailure : std::runtime_error
    {
    public:
        AssertionFailure(const char* expression, const char* file, int line);

        [[nodiscard]] const char* expression() const noexcept { return m_expression; }
        [[nodiscard]] const char* file() const noexcept { return m_file; }
        [[nodiscard]] int line() const noexcept { return m_line; }

    private:
        const char* m_expression;
        const char* m_file;
        int m_line;
    };

    // Logs the assertion failure and throws an AssertionFailure exception
    struct TestingAssertionPlugin {
        void build(class App& app);
        static void invoke(const char* expression, const char* file, int line);
    };
} // namespace orion
