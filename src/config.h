
#ifndef CONFIG_H
#define CONFIG_H
#pragma once

#include <iostream>
#include <string_view>
#include <format>

/// Debugging and behavior defines

// #define INSPECTION_DEBUG_PRINTS      /* Make print-outs to track meta inspection */
// #define SERIALIZATION_DEBUG_PRINTS   /* Make print-outs to track meta inspection */
#define COMMAND_DEBUG_PRINTS            /* Make print-outs to track command genenration and/or execution */
#define USE_COMMANDS                    /* Generate commands rather than executing inspector edits directly */

/// Misc global defines
#define EENG_NULL_INDEX -1
#define EENG_FINF std::numeric_limits<float>::max()
#define EENG_FNINF std::numeric_limits<float>::lowest()
#define EENG_FEPSILON std::numeric_limits<float>::epsilon()
#define EENG_PRINT_LOG_TO_COUT

/// Rendering defines
#define EENG_MSAA
#define EENG_MSAA_SAMPLES 4
#define EENG_ANISO
#define EENG_ANISO_SAMPLES 8

/// Platform
#ifdef _WIN32
#define EENG_PLATFORM_WINDOWS
#else
#define EENG_PLATFORM_UNIX
#define EENG_PLATFORM_LINUX
#define EENG_PLATFORM_APPLE
#endif

/// CPP versions
#if __cplusplus >= 201103L
#define CPP11_SUPPORTED
#endif
#if __cplusplus >= 201402L
#define CPP14_SUPPORTED
#endif
#if __cplusplus >= 201703L
#define CPP17_SUPPORTED
#endif
#if __cplusplus >= 202002L
#define CPP20_SUPPORTED
#endif

/// Compiler
#ifdef _MSC_VER
#define EENG_COMPILER_MSVC
#endif
#ifdef __clang__
#define EENG_COMPILER_CLANG
#endif
#ifdef __GNUC__
#define EENG_COMPILER_GCC
#endif

/// Debug
#if !defined(NDEBUG) || defined(_DEBUG)
#define EENG_DEBUG
#define EENG_ENABLE_ASSERTS
#endif

/// Debug break
#ifdef EENG_PLATFORM_WINDOWS
#define EENG_DEBUG_BREAK() __debugbreak()
#else
#if defined(__ARM_ARCH) || defined(__aarch64__)
#define EENG_DEBUG_BREAK() __asm__ volatile("svc #0")
#elif defined(__x86_64__) || defined(__i386__)
#define EENG_DEBUG_BREAK() __asm__ volatile("int $0x03")
#else
#error "Unsupported architecture"
#endif
#endif

/// Assert
#if defined(EENG_ENABLE_ASSERTS) && defined(CPP20_SUPPORTED)
template <class... Args>
static void EENG_ERROR(std::string_view fmt, Args &&...args)
{
    auto msg = std::vformat(fmt, std::make_format_args(args...));
    std::cerr << "Error: " << msg << std::endl;
}
#define EENG_ASSERT(x, ...)          \
    {                                \
        if (!(x))                    \
        {                            \
            EENG_ERROR(__VA_ARGS__); \
            EENG_DEBUG_BREAK();      \
        }                            \
    }
#elif defined(EENG_ENABLE_ASSERTS)
#include <cassert>
#define EENG_ASSERT(x, ...) assert(x);
#else
#define EENG_ASSERT(x, ...)
#endif

/// GL version
#ifdef EENG_PLATFORM_APPLE
#define EENG_GLVERSION_MAJOR 4
#define EENG_GLVERSION_MINOR 1
#else
#define EENG_GLVERSION_MAJOR 4
#define EENG_GLVERSION_MINOR 3
#endif
//
#if EENG_GLVERSION_MAJOR >= 4
#if EENG_GLVERSION_MINOR >= 1
#define EENG_GLVERSION_41
#endif
#if EENG_GLVERSION_MINOR >= 3
#define EENG_GLVERSION_43
#endif
#endif
#ifndef EENG_GLVERSION_41
static_assert(false, "OpenGL 4.1 is required");
#endif

/// Print defines
static void LOG_DEFINES(auto& LogFunc)
{
#ifdef EENG_DEBUG
    LogFunc("Mode DEBUG");
#else
    LogFunc("Mode RELEASE");
#endif

#ifdef EENG_COMPILER_MSVC
    LogFunc("Compiler MSVC");
#elif defined(EENG_COMPILER_CLANG)
    LogFunc("Compiler Clang");
#elif defined(EENG_COMPILER_GCC)
    LogFunc("Compiler GCC");
#endif

#ifdef CPP20_SUPPORTED
    LogFunc("C++ version 20");
#elif defined(CPP17_SUPPORTED)
    LogFunc("C++ version 17");
#elif defined(CPP14_SUPPORTED)
    LogFunc("C++ version: 14");
#elif defined(CPP11_SUPPORTED)
    LogFunc("C++ version 11");
#endif
}

/// Convert an enum class to its underlying type
template <typename E>
constexpr auto to_integral(E e) -> typename std::underlying_type<E>::type
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

#endif
