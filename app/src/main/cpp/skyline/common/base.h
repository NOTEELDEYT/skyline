// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <cstdint>
#include <stdexcept>
#include <variant>
#include <bitset>
#include <fmt/format.h>

namespace fmt {
    /**
     * @brief A std::bitset formatter for {fmt}
     */
    template<size_t N>
    struct formatter<std::bitset<N>> : formatter<std::string> {
        template<typename FormatContext>
        constexpr auto format(const std::bitset<N> &s, FormatContext &ctx) {
            return formatter<std::string>::format(s.to_string(), ctx);
        }
    };

    template<class T> requires std::is_enum_v<T>
    struct formatter<T> : formatter<std::underlying_type_t<T>> {
        template<typename FormatContext>
        constexpr auto format(T s, FormatContext &ctx) {
            return formatter<std::underlying_type_t<T>>::format(static_cast<std::underlying_type_t<T>>(s), ctx);
        }
    };
}

namespace skyline {
    using u128 = __uint128_t; //!< Unsigned 128-bit integer
    using u64 = __uint64_t; //!< Unsigned 64-bit integer
    using u32 = __uint32_t; //!< Unsigned 32-bit integer
    using u16 = __uint16_t; //!< Unsigned 16-bit integer
    using u8 = __uint8_t; //!< Unsigned 8-bit integer
    using i128 = __int128_t; //!< Signed 128-bit integer
    using i64 = __int64_t; //!< Signed 64-bit integer
    using i32 = __int32_t; //!< Signed 32-bit integer
    using i16 = __int16_t; //!< Signed 16-bit integer
    using i8 = __int8_t; //!< Signed 8-bit integer

    using KHandle = u32; //!< The type of a kernel handle

    namespace constant {
        // Time
        constexpr i64 NsInMicrosecond{1000}; //!< The amount of nanoseconds in a microsecond
        constexpr i64 NsInSecond{1000000000}; //!< The amount of nanoseconds in a second
        constexpr i64 NsInMillisecond{1000000}; //!< The amount of nanoseconds in a millisecond
        constexpr i64 NsInDay{86400000000000UL}; //!< The amount of nanoseconds in a day
    }

    namespace util {
        /**
         * @brief A way to apply a transformation to a paramater pack.
         * @tparam Out The output type
         * @tparam Fun The transformation to apply to the parameter pack
         */
        template<template<typename...> class Out,
            template<typename> class Fun,
            typename... Args>
        struct TransformArgs {
            using Type = Out<typename Fun<Args>::Type...>;
        };

        /**
         * @brief A way to implicitly cast all pointer types to uintptr_t, this is used for {fmt} as we use 0x{:X} to print pointers
         * @note There's the exception of signed char pointers as they represent C Strings
         * @note This does not cover std::shared_ptr or std::unique_ptr and those will have to be explicitly casted to uintptr_t or passed through fmt::ptr
         */
        template<typename T>
        struct FmtCast {
            using Type = std::conditional_t<std::is_pointer_v<T>,
                                            std::conditional_t<std::is_same_v<char, std::remove_cv_t<std::remove_pointer_t<T>>>,
                                                               char *, uintptr_t>,
                                            T>;
        };

        /**
         * @brief A way to implicitly cast all pointers to uintptr_t, this is used for {fmt} as we use 0x{:X} to print pointers
         * @note There's the exception of signed char pointers as they represent C Strings
         * @note This does not cover std::shared_ptr or std::unique_ptr and those will have to be explicitly casted to uintptr_t or passed through fmt::ptr
         */
        template<typename T>
        constexpr auto FmtCastObj(T object) {
            if constexpr (std::is_pointer_v<T>)
                if constexpr (std::is_same_v<char, typename std::remove_cv_t<typename std::remove_pointer_t<T>>>)
                    return reinterpret_cast<typename std::common_type_t<char *, T>>(object);
                else
                    return reinterpret_cast<const uintptr_t>(object);
            else
                return object;
        }

        /**
         * @brief A wrapper around fmt::format_string which casts all pointer types to uintptr_t
         */
        template<typename... Args>
        using FormatString = typename TransformArgs<fmt::format_string, FmtCast, Args...>::Type;

        /**
         * @brief {fmt}::format but with FmtCast built into it
         */
        template<typename... Args>
        constexpr auto Format(FormatString<Args...> formatString, Args... args) {
            return fmt::vformat(formatString, fmt::make_format_args(std::forward<decltype(FmtCastObj(args))>(FmtCastObj(args))...));
        }
    }

    /**
     * @brief A wrapper over std::runtime_error with {fmt} formatting
     */
    class exception : public std::runtime_error {
      public:
        template<typename... Args>
        exception(util::FormatString<Args...> formatStr, Args... args) : runtime_error(util::Format(formatStr, std::forward<Args>(args)...)) {}

        template<typename S>
        exception(S string) : runtime_error(string) {}
    };

    /**
     * @brief A deduction guide for overloads required for std::visit with std::variant
     */
    template<class... Ts>
    struct VariantVisitor : Ts ... { using Ts::operator()...; };
    template<class... Ts> VariantVisitor(Ts...) -> VariantVisitor<Ts...>;
}
