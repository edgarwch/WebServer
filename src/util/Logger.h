#pragma once

#include <string_view>
#include <sstream>
#include <mutex>

enum class LogLevel { Debug, Info, Warn, Error };

class Logger {
public:
    static Logger& instance();

    void setLevel(LogLevel level) noexcept { level_ = level; }

    class Stream {
    public:
        Stream(LogLevel lvl);
        ~Stream();
        Stream(Stream&&) = default;
        Stream& operator=(Stream&&) = default;

        template<typename T>
        Stream& operator<<(const T& val) {
            buf_ << val;
            return *this;
        }

    private:
        LogLevel level_;
        std::ostringstream buf_;
    };

    static Stream debug()   { return Stream(LogLevel::Debug); }
    static Stream info()    { return Stream(LogLevel::Info); }
    static Stream warn()    { return Stream(LogLevel::Warn); }
    static Stream error()   { return Stream(LogLevel::Error); }

private:
    Logger() = default;
    void write(LogLevel lvl, std::string_view msg);

    LogLevel level_ = LogLevel::Info;
    std::mutex mutex_;
};
