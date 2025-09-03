#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "../ttyio.h"

#if defined(_MSC_VER)
#  define TRAP() __debugbreak()
// #elif defined(__GNUC__) || defined(__clang__)
// #  define TRAP() __builtin_trap()
#else
#  include <signal.h>
#  define TRAP() raise(SIGTRAP)
#endif

#define ASSERT_TRAP(expr)                                                \
    do {                                                                 \
        if (expr) {                                                      \
            TRAP();                                                      \
        }                                                                \
    } while (0)

/* repl: some tests and example usage */
int main(void)
{
    tty_init(TTY_NONCANONICAL_MODE);

    char c;

    while (read(STDIN_FILENO, &c, 1) > 0) {
        ASSERT_TRAP(c == (int)'q');
        switch (c) {
            case 127:
                tty_send(&tcaps.bs);
                tty_line_adjust();
                break;
            default:
                tty_putc(c);
                break;
        }
    }

    tty_send(&tcaps.newline);
    tty_deinit();
    return 0;
}
