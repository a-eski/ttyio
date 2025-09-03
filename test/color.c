#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../ttyio.h"

void bg_colors_test(void)
{
    for (int i = 0; i < tcaps.color_max; ++i) {
        tty_color_bg_set(i);
        tty_write(" ", 1);
    }
    tty_color_reset();
    assert(term.pos.y == 45);
    tty_send(&tcaps.newline);
    assert(term.pos.y == 45);
}

int main()
{
    tty_init(TTY_NONCANONICAL_MODE);

    int curr_color = 16;
    for (size_t y = 0; y < term.size.y; ++y) {
        for (size_t x = 0; x < term.size.x; ++x) {
            tty_color_bg_set(curr_color);
            tty_putc(' ');
        }
        ++curr_color;
    }
    tty_color_reset();
    tty_send(&tcaps.newline);

    tty_deinit();
}
