#pragma once

#include <fmt/base.h>

#include <memory>
#include <string>

namespace orion
{
    enum class LogLevel {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Off,
    };

    const char* format_as(LogLevel log_level);

    class Sink
    {
    public:
        Sink() = default;
        virtual ~Sink() = default;

        void log(const char* string);

    protected:
        Sink(const Sink&) = default;
        Sink(Sink&&) = default;
        Sink& operator=(const Sink&) = default;
        Sink& operator=(Sink&&) = default;

    private:
        virtual void do_log(const char* string) = 0;
    };

    class Logger
    {
    public:
        Logger(std::string name, LogLevel active_level, std::unique_ptr<Sink> sink);

        template<typename... Args>
        void trace(fmt::string_view format, Args&&... args) const
        {
            log(LogLevel::Trace, format, fmt::make_format_args(std::forward<Args>(args)...));
        }

        template<typename... Args>
        void debug(fmt::string_view format, Args&&... args) const
        {
            log(LogLevel::Debug, format, fmt::make_format_args(std::forward<Args>(args)...));
        }

        template<typename... Args>
        void info(fmt::string_view format, Args&&... args) const
        {
            log(LogLevel::Info, format, fmt::make_format_args(std::forward<Args>(args)...));
        }

        template<typename... Args>
        void warn(fmt::string_view format, Args&&... args) const
        {
            log(LogLevel::Warn, format, fmt::make_format_args(std::forward<Args>(args)...));
        }

        template<typename... Args>
        void error(fmt::string_view format, Args&&... args) const
        {
            log(LogLevel::Error, format, fmt::make_format_args(std::forward<Args>(args)...));
        }

    private:
        void log(LogLevel level, fmt::string_view format, fmt::format_args args) const;

        std::string name_;
        LogLevel active_level_;
        std::unique_ptr<Sink> sink_;
    };

    std::unique_ptr<Logger> make_stdout_logger(LogLevel level, std::string name = "stdout");
    std::unique_ptr<Logger> make_stderr_logger(LogLevel level, std::string name = "stderr");
} // namespace orion
