#pragma once
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#  define TRAP() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#  define TRAP() __builtin_trap()
#else
#  include <signal.h>
#  define TRAP() raise(SIGTRAP)
#endif

#define ASSERT_TRAP(expr)                                                \
    do {                                                                 \
        if (!(expr)) {                                                   \
            fprintf(stderr, "Assertion failed: %s (%s:%d)\n",            \
                    #expr, __FILE__, __LINE__);                          \
            fflush(stderr);                                              \
            TRAP();                                                      \
        }                                                                \
    } while (0)
