// SPDX-FileCopyrightText: 2024-2026 Filipe Coelho <falktx@darkglass.com>
// SPDX-License-Identifier: ISC

#pragma once

#include <assert.h>
#include <stdio.h>

// --------------------------------------------------------------------------------------------------------------------
// always valid assert even in release builds, returns a value on failure

static inline
void _assert_print(const char* const expr, const char* const file, const int line)
{
    fprintf(stderr, "assertion failure: \"%s\" in file %s line %d\n", expr, file, line);
}

#ifndef NDEBUG
#define assert_continue(expr) { assert(expr); }
#define assert_return(expr, ret) { assert(expr); }
#elif defined(__GNUC__)
#define assert_continue(expr) \
    { if (__builtin_expect(!(expr),0)) { _assert_print(#expr, __FILE__, __LINE__); continue; } }
#define assert_return(expr, ret) \
    { if (__builtin_expect(!(expr),0)) { _assert_print(#expr, __FILE__, __LINE__); return ret; } }
#else
#define assert_continue(expr) \
    { if (!(expr)) { _assert_print(#expr, __FILE__, __LINE__); continue; } }
#define assert_return(expr, ret) \
    { if (!(expr)) { _assert_print(#expr, __FILE__, __LINE__); return ret; } }
#endif

// --------------------------------------------------------------------------------------------------------------------
