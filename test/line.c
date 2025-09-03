#include <assert.h>
#include <limits.h>
#include <unistd.h>

#include "../ttyio.h"

#define debug_assert(expr) \
do {    \
    if (!(expr)) {          \
    tty_println("term.pos.x %zu, term.pos.y %zu", term.pos.x, term.pos.y); \
    } \
} while (0);

/* line manipulations: some tests and example usage */
int main(void)
{
    tty_init(TTY_NONCANONICAL_MODE);

    // tty_println("term.pos.x %zu, term.pos.y %zu", term.pos.x, term.pos.y);
    size_t y_start = term.pos.y;

    for (size_t i = 0; i < term.size.x - 1; ++i) {
        tty_putc('x');
        assert(term.pos.x == i + 1);
        assert(term.pos.y == y_start);
    }

    tty_putc('x');
    assert(term.pos.x == 0);
    tty_putc('x');
    assert(term.pos.x == 1);

    tty_send(&tcaps.bs);
    assert(term.pos.x == 0);
    tty_goto_prev_eol();
    assert(term.pos.x == term.size.x - 1);
    tty_send(&tcaps.bs);
    assert(term.pos.x == term.size.x - 2);
    tty_println("term.pos.x %zu, term.pos.y %zu", term.pos.x, term.pos.y);
    tty_send(&tcaps.newline);

    while (term.pos.x < term.size.x - 1) {
        tty_putc('x');
    }
    tty_putc('x');
    tty_println("term.pos.x %zu, term.pos.y %zu", term.pos.x, term.pos.y);

    y_start = term.pos.y;
    for (size_t i = 0; i < term.size.x - 1; ++i) {
        tty_putc('x');
        assert(term.pos.x == i + 1);
        assert(term.pos.y == y_start);
    }
    fflush(stdout);
    tty_putc('x');
    assert(term.pos.x == 0);
    assert(term.pos.y == y_start + 1 || term.pos.y == term.size.y - 1);
    tty_putc('x');
    fflush(stdout);
    tty_send(&tcaps.bs);
    fflush(stdout);
    tty_goto_prev_eol();
    tty_send(&tcaps.bs);
    fflush(stdout);

    // tty_println("term.size.x %zu, term.size.y %zu", term.size.x, term.size.y);
    // tty_println("term.pos.x %zu, term.pos.y %zu", term.pos.x, term.pos.y);

    char c;
    if (read(STDIN_FILENO, &c, 1) == -1)
        return 1;
    tty_send(&tcaps.newline);
    tty_deinit();
    return 0;
}
