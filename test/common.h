#ifndef COMMON_GUARD_H_
#define COMMON_GUARD_H_

#if defined(NDEBUG) || defined(TTYIO_NDEBUG)
#   define assert_trap(expr)
#   define trap_on(expr)
#else
#if defined(_MSC_VER)
#  define TRAP() __debugbreak()
// #elif defined(__GNUC__) || defined(__clang__)
// #  define TRAP() __builtin_trap()
#else
#  include <signal.h>
#  define TRAP() raise(SIGTRAP)
#endif

#define assert_trap(expr)                                                  \
    do {                                                                   \
        if (!(expr)) {                                                     \
            tty_println("triggered at %zu, %zu", term.pos.x, term.pos.y);  \
            tty_println("%s %s %d", __FILE__, __func__, __LINE__);         \
            tty_println("size is %zu, %zu", term.size.x, term.size.y);     \
            TRAP();                                                        \
        }                                                                  \
    } while (0)

#if defined(_MSC_VER)
#  define TRAP() __debugbreak()
// #elif defined(__GNUC__) || defined(__clang__)
// #  define TRAP() __builtin_trap()
#else
#  include <signal.h>
#  define TRAP() raise(SIGTRAP)
#endif

#define trap_on(expr)                                                      \
    do {                                                                   \
        if (expr) {                                                        \
            tty_println("triggered at %zu, %zu", term.pos.x, term.pos.y);  \
            tty_println("%s %s %d", __FILE__, __func__, __LINE__);         \
            tty_println("size is %zu, %zu", term.size.x, term.size.y);     \
            TRAP();                                                        \
        }                                                                  \
    } while (0)

#endif // NDEBUG
#endif
