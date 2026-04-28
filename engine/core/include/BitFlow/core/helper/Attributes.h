#pragma once

// BF_DEPRECATED(msg)
// Use before a declaration to mark it deprecated with a message.
// Example:
//   BF_DEPRECATED("Use Expr") struct ExprOld { ... };
#if defined(_MSC_VER)
#define BF_DEPRECATED(msg) __declspec(deprecated(msg))
#elif defined(__has_cpp_attribute)
#if __has_cpp_attribute(deprecated)
#define BF_DEPRECATED(msg) [[deprecated(msg)]]
#else
#if defined(__GNUC__) || defined(__clang__)
#define BF_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#define BF_DEPRECATED(msg)
#endif
#endif
#elif defined(__GNUC__) || defined(__clang__)
#define BF_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#define BF_DEPRECATED(msg)
#endif