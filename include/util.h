#pragma once

#define STATIC_ASSERT_SIMPLE(EXPR)   _Static_assert(EXPR, "unspecified message")
#define STATIC_ASSERT_MSG(EXPR, MSG) _Static_assert(EXPR, MSG)

#define _SELECT_ASSERT_FUNC(x, EXPR, MSG, ASSERT_MACRO, ...) ASSERT_MACRO

/**
 * @brief   Static (i.e. compile time) assert macro.
 *
 * @note The output of STATIC_ASSERT can be different across compilers.
 *
 * Usage:
 * STATIC_ASSERT(expression);
 * STATIC_ASSERT(expression, message);
 *
 * @hideinitializer
 */
#define STATIC_ASSERT(...)                                                                                             \
  _SELECT_ASSERT_FUNC(x, ##__VA_ARGS__, STATIC_ASSERT_MSG(__VA_ARGS__), STATIC_ASSERT_SIMPLE(__VA_ARGS__))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif
