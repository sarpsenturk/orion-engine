#include "orion/log.h"

#include <fmt/format.h>

#include <cstdio>

namespace orion
{
    namespace
    {
        class ConsoleSink final : public Sink
        {
        public:
            explicit ConsoleSink(std::FILE* handle)
                : handle_(handle)
            {
            }

        private:
            void do_log(const char* string) override
            {
                std::fputs(string, handle_);
            }

            std::FILE* handle_;
        };
    } // namespace

    const char* format_as(LogLevel log_level)
    {
        switch (log_level) {
            case LogLevel::Trace:
                return "trace";
            case LogLevel::Debug:
                return "debug";
            case LogLevel::Info:
                return "info";
            case LogLevel::Warn:
                return "warn";
            case LogLevel::Error:
                return "error";
            case LogLevel::Off:
                return "off";
        }
        // TODO: Use unreachable
        return "invalid_level";
    }

    void Sink::log(const char* string)
    {
        do_log(string);
    }

    Logger::Logger(std::string name, LogLevel active_level, std::unique_ptr<Sink> sink)
        : name_(std::move(name))
        , active_level_(active_level)
        , sink_(std::move(sink))
    {
    }

    void Logger::log(LogLevel level, fmt::string_view format, fmt::format_args args) const
    {
        if (level >= active_level_) {
            const auto string = fmt::format("[{}] [{}] {}\n", name_, level, fmt::vformat(format, args));
            sink_->log(string.c_str());
        }
    }

    std::unique_ptr<Logger> make_stdout_logger(LogLevel level, std::string name)
    {
        return std::make_unique<Logger>(std::move(name), level, std::make_unique<ConsoleSink>(stdout));
    }

    std::unique_ptr<Logger> make_stderr_logger(LogLevel level, std::string name)
    {
        return std::make_unique<Logger>(std::move(name), level, std::make_unique<ConsoleSink>(stderr));
    }
} // namespace orion
