#include "Logger.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

Logger::Stream::Stream(LogLevel lvl) : level_(lvl) {}

Logger::Stream::~Stream() {
    Logger::instance().write(level_, buf_.str());
}

void Logger::write(LogLevel lvl, std::string_view msg) {
    if (lvl < level_) return;

    const char* label = "???";
    switch (lvl) {
        case LogLevel::Debug: label = "DEBUG"; break;
        case LogLevel::Info:  label = "INFO "; break;
        case LogLevel::Warn:  label = "WARN "; break;
        case LogLevel::Error: label = "ERROR"; break;
    }

    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::lock_guard lock(mutex_);
    std::cerr << std::put_time(std::localtime(&t), "%F %T")
              << '.' << std::setw(3) << std::setfill('0') << ms.count()
              << " [" << label << "] " << msg;
    if (!msg.empty() && msg.back() != '\n') std::cerr << '\n';
}
