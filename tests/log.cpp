#include "orion/log.h"

#include <gtest/gtest.h>

#include <string>

namespace orion
{
    namespace
    {
        class TestSink final : public Sink
        {
        public:
            auto& result() const { return result_; }

        private:
            void do_log(const char* string) override { result_ = string; }

            std::string result_;
        };

        Logger make_test_logger(std::string name, LogLevel level)
        {
            return Logger{std::move(name), level, std::make_unique<TestSink>()};
        }

        auto& log_result(const Logger& logger)
        {
            return static_cast<const TestSink*>(logger.sink())->result();
        }

        TEST(Log, Format)
        {
            auto logger = make_test_logger("test", LogLevel::Trace);
            logger.trace("Hello {}", "world");
            EXPECT_EQ(log_result(logger), "[test] [trace] Hello world\n");
        }

        TEST(Log, LogLevelLess)
        {
            auto logger = make_test_logger("", LogLevel::Error);
            logger.trace("Hello world");
            EXPECT_TRUE(log_result(logger).empty());
        }

        TEST(Log, LogLevelEqual)
        {
            auto logger = make_test_logger("", LogLevel::Warn);
            logger.warn("warn");
            EXPECT_FALSE(log_result(logger).empty());
        }

        TEST(Log, LogLevelGreater)
        {
            auto logger = make_test_logger("", LogLevel::Warn);
            logger.error("warn");
            EXPECT_FALSE(log_result(logger).empty());
        }
    } // namespace
} // namespace orion
