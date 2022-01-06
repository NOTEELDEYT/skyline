// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <android/log.h>
#include "utils.h"
#include "logger.h"

namespace skyline {
    void Logger::LoggerContext::Initialize(const std::string &path) {
        start = util::GetTimeNs() / constant::NsInMillisecond;
        logFile.open(path, std::ios::trunc);
    }

    void Logger::LoggerContext::Finalize() {
        logFile.close();
    }

    void Logger::LoggerContext::Flush() {
        logFile.flush();
    }

    Logger::LoggerContext *Logger::GetContext() {
        return context;
    }

    void Logger::SetContext(LoggerContext *pContext) {
        context = pContext;
    }

    void Logger::StartLoggerThread() {
        if (!thread.joinable())
            thread = std::thread(&Logger::Run);
    }

    void Logger::Run() {
        pthread_setname_np(pthread_self(), "Logger");

        // We don't need to setup signal exception handling here because a SIGINT will never be issued to the logger thread to stop, since its lifetime is equivalent to the process
        // Any other signal crashes are used to log a stack trace, but if the logger thread crashes then we cannot ensure that trying to log the stack trace won't cause recursive crashes
        logQueue.Process([&](const LogEntry &next) {
            Write(next);
        });
    }

    void Logger::UpdateTag() {
        if (!threadId) {
            std::array<char, 16> name;
            if (!pthread_getname_np(pthread_self(), name.data(), name.size())) {
                threadNamePool.emplace_back(name.data());
                threadId = threadNamePool.size() - 1;
            } else {
                threadId = 0;
            }
        }
    }

    void Logger::WriteAndroid(const LogEntry &logEntry) {
        constexpr std::array<int, 5> levelAlog{ANDROID_LOG_ERROR, ANDROID_LOG_WARN, ANDROID_LOG_INFO, ANDROID_LOG_DEBUG, ANDROID_LOG_VERBOSE}; // This corresponds to LogLevel and provides its equivalent for NDK Logging

        std::string tag = "emu-cpp-" + threadNamePool[logEntry.threadId];
        __android_log_write(levelAlog[static_cast<u8>(logEntry.level)], tag.c_str(), logEntry.str.c_str());
    }

    void Logger::Write(const LogEntry &logEntry) {
        constexpr std::array<char, 5> levelCharacter{'E', 'W', 'I', 'D', 'V'}; // The LogLevel as written out to a file
        WriteAndroid(logEntry);

        if (logEntry.context)
            // We use RS (\036) and GS (\035) as our delimiters
            logEntry.context->Write(fmt::format("\036{}\035{}\035{}\035{}\n", levelCharacter[static_cast<u8>(logEntry.level)], (util::GetTimeNs() / constant::NsInMillisecond) - logEntry.context->start, threadNamePool[logEntry.threadId], logEntry.str));
    }

    void Logger::LoggerContext::Write(const std::string &str) {
        std::lock_guard guard(mutex);
        logFile << str;
    }
}