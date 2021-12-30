// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <fstream>
#include <mutex>
#include "base.h"

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

        static inline LogLevel configLevel{LogLevel::Verbose}; //!< The minimum level of logs to write

        /**
         * @brief Holds logger variables that cannot be static
         */
        struct LoggerContext {
            std::mutex mutex; //!< Synchronizes all output I/O to ensure there are no races
            std::ofstream logFile; //!< An output stream to the log file
            i64 start; //!< A timestamp in milliseconds for when the logger was started, this is used as the base for all log timestamps

            LoggerContext() {}

            void Initialize(const std::string &path);

            void Finalize();

            void Flush();

            void Write(const std::string &str);
        };
        static inline LoggerContext EmulationContext, LoaderContext;

        /**
         * @brief Update the tag in log messages with a new thread name
         */
        static void UpdateTag();

        static LoggerContext *GetContext();

        static void SetContext(LoggerContext *context);

        static void WriteAndroid(LogLevel level, const std::string &str);

        static void Write(LogLevel level, const std::string &str);

        /**
         * @brief A wrapper around a string which captures the calling function using Clang source location builtins
         * @note A function needs to be declared for every argument template specialization as CTAD cannot work with implicit casting
         * @url https://clang.llvm.org/docs/LanguageExtensions.html#source-location-builtins
         */
        template<typename... Args>
        struct FunctionString {
            fmt::format_string<Args...> string;
            const char *function;

            constexpr FunctionString(fmt::format_string<Args...> string, const char *function = __builtin_FUNCTION()) : string(std::move(string)), function(function) {}

            constexpr fmt::format_string<Args...> operator*() {
                return util::Format("{}: {}", function, string);
            }
        };

        template<typename... Args>
        static void Error(util::FormatString<Args...> formatString, Args... args) {
            if (LogLevel::Error <= configLevel)
                Write(LogLevel::Error, util::Format(formatString, std::forward<Args>(args)...));
        }

        template<typename S>
        static void Error(const S &string) {
            if (LogLevel::Error <= configLevel)
                Write(LogLevel::Error, string);
        }

        template<typename... Args>
        static void ErrorNoPrefix(util::FormatString<Args...> formatString, Args... args) {
            if (LogLevel::Error <= configLevel)
                Write(LogLevel::Error, util::Format(formatString, std::forward<Args>(args)...));
        }

        template<typename... Args>
        static void Warn(util::FormatString<Args...> formatString, Args... args) {
            if (LogLevel::Warn <= configLevel)
                Write(LogLevel::Warn, util::Format(formatString, std::forward<Args>(args)...));
        }

        template<typename S>
        static void Warn(const S &string) {
            if (LogLevel::Warn <= configLevel)
                Write(LogLevel::Warn, string);
        }

        template<typename... Args>
        static void WarnNoPrefix(util::FormatString<Args...> formatString, Args... args) {
            if (LogLevel::Warn <= configLevel)
                Write(LogLevel::Warn, util::Format(formatString, std::forward<Args>(args)...));
        }

        template<typename... Args>
        static void Info(util::FormatString<Args...> formatString, Args... args) {
            if (LogLevel::Info <= configLevel)
                Write(LogLevel::Info, util::Format(formatString, std::forward<Args>(args)...));
        }

        template<typename S>
        static void Info(const S &string) {
            if (LogLevel::Info <= configLevel)
                Write(LogLevel::Info, string);
        }

        template<typename... Args>
        static void InfoNoPrefix(util::FormatString<Args...> formatString, Args... args) {
            if (LogLevel::Info <= configLevel)
                Write(LogLevel::Info, util::Format(formatString, std::forward<Args>(args)...));
        }

        template<typename... Args>
        static void Debug(util::FormatString<Args...> formatString, Args... args) {
            if (LogLevel::Debug <= configLevel)
                Write(LogLevel::Debug, util::Format(formatString, std::forward<Args>(args)...));
        }

        template<typename S>
        static void Debug(const S &string) {
            if (LogLevel::Debug <= configLevel)
                Write(LogLevel::Debug, string);
        }

        template<typename... Args>
        static void DebugNoPrefix(util::FormatString<Args...> formatString, Args... args) {
            if (LogLevel::Debug <= configLevel)
                Write(LogLevel::Debug, util::Format(formatString, std::forward<Args>(args)...));
        }

        template<typename... Args>
        static void Verbose(util::FormatString<Args...> formatString, Args... args) {
            if (LogLevel::Verbose <= configLevel)
                Write(LogLevel::Verbose, util::Format(formatString, std::forward<Args>(args)...));
        }

        template<typename S>
        static void Verbose(const S &string) {
            if (LogLevel::Verbose <= configLevel)
                Write(LogLevel::Verbose, string);
        }

        template<typename... Args>
        static void VerboseNoPrefix(util::FormatString<Args...> formatString, Args... args) {
            if (LogLevel::Verbose <= configLevel)
                Write(LogLevel::Verbose, util::Format(formatString, std::forward<Args>(args)...));
        }
    };
}
