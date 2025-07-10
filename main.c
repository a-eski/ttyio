#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "ttyterm.h"

/* example usage */
int main()
{
    term_init();

    term_send(&tcaps.scr_clr);
    term_send(&tcaps.cursor_home);
    term_puts("Cleared screen with unibi");

    term_puts("hello from newline");
    term_puts("hello from newline");
    term_print("hello from newline");
    term_send_n(&tcaps.bs, 4);
    term_puts("hi");
    term_send_n(&tcaps.cursor_down, 2);
    term_puts("hey");
    term_send(&tcaps.cursor_home);
    term_print("Written");
    term_send_n(&tcaps.cursor_down, 8);
    term_print("term.pos.x %d, term.pos.y %d ", term.pos.x, term.pos.y);
    term_println("term.pos.x %d, term.pos.y %d", term.pos.x, term.pos.y);
    term_send(&tcaps.newline);

    term_print("hello world");
    term_send_n(&tcaps.cursor_left, 6);
    term_send(&tcaps.line_clr_to_eol);
    term_send_n(&tcaps.newline, 2);

    term_print("hello world");
    term_send_n(&tcaps.cursor_left, 6);
    term_send(&tcaps.line_clr_to_bol);

    term_send(&tcaps.cursor_down);
    term_color_set(1);
    term_color_bg_set(10);
    term_println("hi");
    term_send(&tcaps.color_reset);

    char c;
    if (read(STDIN_FILENO, &c, 1) == -1)
        return 1;

    term_reset();
    return 0;
}
