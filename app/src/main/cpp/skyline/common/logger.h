// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <fstream>
#include <mutex>
#include <thread>
#include "base.h"
#include "circular_queue.h"

namespace skyline {
    /**
     * @brief A wrapper around writing logs into a log file and logcat using Android Log APIs
     */
    class Logger {
      private:
        Logger() {}

      public:
        enum class LogLevel {
            Error,
            Warn,
            Info,
            Debug,
            Verbose,
        };

        /**
         * @brief Holds logger variables that cannot be static
         */
        struct LoggerContext {
            std::mutex mutex; //!< Synchronizes all output I/O to ensure there are no races
            std::ofstream logFile; //!< An output stream to the log file
            i64 start; //!< A timestamp in milliseconds for when the logger was started, this is used as the base for all log timestamps

            LoggerContext() {}

            /**
             * @brief Initialises a LoggerContext and opens an output file stream at the given path
             */
            void Initialize(const std::string &path);

            /**
             * @brief Closes the output file stream
             * @note After this has been called, the context must be re-initialised before any other logging operation
             */
            void Finalize();

            void Flush();

            void Write(const std::string &str);
        };
        static inline LoggerContext EmulationContext, LoaderContext;

        struct LogEntry {
            LoggerContext *context;
            LogLevel level;
            std::string str;
            std::string threadName;
        };

        static constexpr size_t LogQueueSize{1024}; //!< Maximum size of the log queue, this value is arbritary
        static inline LogLevel configLevel{LogLevel::Verbose}; //!< The minimum level of logs to write
        static inline CircularQueue<LogEntry> logQueue{LogQueueSize}; //!< The queue where all log messages are sent
        static inline std::thread thread; //!< The thread that handles writing log entries from the logger queue

        thread_local static inline std::string threadName;
        thread_local static inline LoggerContext *context{&EmulationContext}; //!< The logger context attached to the current thread

        /**
         * @note The logger thread is launched at application startup via a JNI call, and will keep running until the app is closed
         */
        static void StartLoggerThread();

        static void Run();

        /**
         * @brief Update the tag in log messages with a new thread name
         */
        static void UpdateTag();

        /**
         * @brief Gets the caller thread's LoggerContext
         */
        static LoggerContext *GetContext();

        /**
         * @brief Sets the given LoggerContext to the caller thread
         */
        static void SetContext(LoggerContext *context);

        static void WriteAndroid(const LogEntry &logEntry);

        static void Write(const LogEntry &logEntry);

        template<typename... Args>
        static void Log(LogLevel level, const char *function, util::FormatString<Args...> formatString, Args... args) {
            UpdateTag();
            if (level <= configLevel)
                logQueue.Push({
                                  .context = context,
                                  .level = level,
                                  .str = std::move(std::string(function) + ": " + util::Format(formatString, std::forward<Args>(args)...)),
                                  .threadName = threadName,
                              });
        }

        template<typename... Args>
        static void LogNoPrefix(LogLevel level, util::FormatString<Args...> formatString, Args... args) {
            UpdateTag();
            if (level <= configLevel)
                logQueue.Push({
                                  .context = context,
                                  .level = level,
                                  .str = std::move(util::Format(formatString, std::forward<Args>(args)...)),
                                  .threadName = threadName,
                              });
        }

        #define LOG(level, formatString, ...)                                                                       \
        do {                                                                                                        \
            skyline::Logger::Log(level, __func__, formatString, ##__VA_ARGS__);                                     \
        } while (false)

        #define LOG_NOPREFIX(level, formatString, ...)                                                              \
        do {                                                                                                        \
            skyline::Logger::LogNoPrefix(level, formatString, ##__VA_ARGS__);                                       \
        } while (false)

        #define LOG_ERROR(formatString, ...)                                                                        \
        do {                                                                                                        \
            LOG(LogLevel::Error, formatString, __VA_ARGS__);                                                        \
        } while (false)

        #define LOG_ERROR_NOPREFIX(formatString, ...)                                                               \
        do {                                                                                                        \
            LOG_NOPREFIX(LogLevel::Error, formatString, __VA_ARGS__);                                               \
        } while (false)

        #define LOG_WARN(formatString, ...)                                                                         \
        do {                                                                                                        \
            LOG(LogLevel::Warn, formatString, __VA_ARGS__);                                                         \
        } while (false)

        #define LOG_WARN_NOPREFIX(formatString, ...)                                                                \
        do {                                                                                                        \
            LOG_NOPREFIX(LogLevel::Warn, formatString, __VA_ARGS__);                                                \
        } while (false)

        #define LOG_INFO(formatString, ...)                                                                         \
        do {                                                                                                        \
            LOG(LogLevel::Info, formatString, __VA_ARGS__);                                                         \
        } while (false)

        #define LOG_INFO_NOPREFIX(formatString, ...)                                                                \
        do {                                                                                                        \
            LOG_NOPREFIX(LogLevel::Info, formatString, __VA_ARGS__);                                                \
        } while (false)

        #define LOG_DEBUG(formatString, ...)                                                                        \
        do {                                                                                                        \
            LOG(LogLevel::Debug, formatString, __VA_ARGS__);                                                        \
        } while (false)

        #define LOG_DEBUG_NOPREFIX(formatString, ...)                                                               \
        do {                                                                                                        \
            LOG_NOPREFIX(LogLevel::Debug, formatString, __VA_ARGS__);                                               \
        } while (false)

        #define LOG_VERBOSE(formatString, ...)                                                                      \
        do {                                                                                                        \
            LOG(LogLevel::Verbose, formatString, __VA_ARGS__);                                                      \
        } while (false)

        #define LOG_VERBOSE_NOPREFIX(formatString, ...)                                                             \
        do {                                                                                                        \
            LOG_NOPREFIX(LogLevel::Verbose, formatString, __VA_ARGS__);                                             \
        } while (false)

        template<typename... Args>
        static void Error(util::FormatString<Args...> formatString, Args... args) {
            LOG_NOPREFIX(LogLevel::Error, formatString, std::forward<Args>(args)...);
        }

        template<typename S>
        static void Error(const S &string) {
            LOG_NOPREFIX(LogLevel::Error, "{}", string);
        }

        template<typename... Args>
        static void ErrorNoPrefix(util::FormatString<Args...> formatString, Args... args) {
            LOG_NOPREFIX(LogLevel::Error, formatString, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Warn(util::FormatString<Args...> formatString, Args... args) {
            LOG_NOPREFIX(LogLevel::Warn, formatString, std::forward<Args>(args)...);
        }

        template<typename S>
        static void Warn(const S &string) {
            LOG_NOPREFIX(LogLevel::Warn, "{}", string);
        }

        template<typename... Args>
        static void WarnNoPrefix(util::FormatString<Args...> formatString, Args... args) {
            LOG_NOPREFIX(LogLevel::Warn, formatString, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Info(util::FormatString<Args...> formatString, Args... args) {
            LOG_NOPREFIX(LogLevel::Info, formatString, std::forward<Args>(args)...);
        }

        template<typename S>
        static void Info(const S &string) {
            LOG_NOPREFIX(LogLevel::Info, "{}", string);
        }

        template<typename... Args>
        static void InfoNoPrefix(util::FormatString<Args...> formatString, Args... args) {
            LOG_NOPREFIX(LogLevel::Info, formatString, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Debug(util::FormatString<Args...> formatString, Args... args) {
            LOG_NOPREFIX(LogLevel::Debug, formatString, std::forward<Args>(args)...);
        }

        template<typename S>
        static void Debug(const S &string) {
            LOG_NOPREFIX(LogLevel::Debug, "{}", string);
        }

        template<typename... Args>
        static void DebugNoPrefix(util::FormatString<Args...> formatString, Args... args) {
            LOG_NOPREFIX(LogLevel::Debug, formatString, std::forward<Args>(args)...);
        }

        template<typename... Args>
        static void Verbose(util::FormatString<Args...> formatString, Args... args) {
            LOG_NOPREFIX(LogLevel::Verbose, formatString, std::forward<Args>(args)...);
        }

        template<typename S>
        static void Verbose(const S &string) {
            LOG_NOPREFIX(LogLevel::Verbose, "{}", string);
        }

        template<typename... Args>
        static void VerboseNoPrefix(util::FormatString<Args...> formatString, Args... args) {
            LOG_NOPREFIX(LogLevel::Verbose, formatString, std::forward<Args>(args)...);
        }
    };
}
