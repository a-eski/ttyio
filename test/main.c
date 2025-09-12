#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../ttyio.h"
#include "../ttyplatform.h"

/* main: a variety of tests and example usage */
/* These tests were done with term.size.x 171, term.size.y 46
 * The x size doesn't need to be exact for the tests, but the y does if NDEBUG not defined.
 * The tests are currently dependent on ordering.
 */

void multiline(double multiplier)
{
    #define n 150
    char* multiline = malloc(n * multiplier);
    memset(multiline, 0, n * multiplier);
    for (size_t i = 0; i < n * multiplier - 1; ++i) {
        multiline[i] = 'a' + i % 26;
    }
    #undef n
    _MAYBE_UNUSED_ int printed = tty_print("%s", multiline);
    free(multiline);
}

void scr_clear_then_home_test(void)
{
    tty_send(&tcaps.scr_clr);
    tty_send(&tcaps.cursor_home);
}

void puts_test(void)
{
    tty_puts("Cleared screen with unibi");

    tty_puts("hello from newline");

    tty_puts("hello from newline");
}

void print_test(void)
{
    tty_print("hello from newline");
}

void bs_and_cursor_left_right_test(void)
{
    tty_send_n(&tcaps.bs, 4);
    tty_send(&tcaps.cursor_left);
    tty_send(&tcaps.cursor_left);
    tty_send_n(&tcaps.cursor_left, 2);
    tty_send_n(&tcaps.cursor_right, 4);
}

void cursor_down_test(void)
{
    tty_puts("hi");
    tty_send_n(&tcaps.cursor_down, 2);
    tty_puts("hey");
}

void cursor_home_test(void)
{
    tty_send(&tcaps.cursor_home);
    tty_print("Written");
}

void moving_around_and_rewriting_test(void)
{
    tty_send_n(&tcaps.cursor_down, 8);
    tty_send(&tcaps.newline);
    tty_print("hello world");
    tty_send_n(&tcaps.cursor_left, 6);
}

void line_clear_eol_and_bol_test(void)
{
    tty_send(&tcaps.line_clr_to_eol);
    tty_send_n(&tcaps.newline, 2);
    tty_print("hello world");
    tty_send_n(&tcaps.cursor_left, 6);
    tty_send(&tcaps.line_clr_to_bol);
    tty_send(&tcaps.cursor_down);
}

void fg_and_bg_color_test(void)
{
    tty_color_set(1);
    tty_color_bg_set(10);
    tty_println("hi");
    tty_send(&tcaps.color_reset);
}

void println_test(void)
{
    tty_println("welcome to %s", "ttyio");
}

void fmt_expansion_test(void)
{
    char test[] = "test expansion working";
    tty_print("%s test", test);
}

void multiline_test(void)
{
    multiline(1.5);

    tty_send_n(&tcaps.newline, 3);
    multiline(3.4);
    tty_send_n(&tcaps.newline, 3);
    multiline(6.7);
    tty_send_n(&tcaps.newline, 3);
    multiline(6.7);
    tty_send(&tcaps.newline);
}

void last_line_test(void)
{
    tty_println("This is the last line!");
    tty_println("This is the last line!");
    tty_println("This is the last line!");
    tty_send_n(&tcaps.newline, 3);
    tty_send(&tcaps.cursor_up);
}

void bg_colors_test(void)
{
    for (int i = 0; i < tcaps.color_max; ++i) {
        tty_color_bg_set(i);
        tty_write(" ", 1);
    }
    tty_color_reset();
    tty_send(&tcaps.newline);
}

int main(void)
{
    tty_init(TTY_NONCANONICAL_MODE);

    scr_clear_then_home_test();
    puts_test();
    bs_and_cursor_left_right_test();
    cursor_down_test();
    cursor_home_test();
    moving_around_and_rewriting_test();
    line_clear_eol_and_bol_test();
    fg_and_bg_color_test();
    println_test();
    fmt_expansion_test();
    multiline_test();
    last_line_test();
    bg_colors_test();

    multiline(.8);
    multiline(.8);

    char c;
    if (read(STDIN_FILENO, &c, 1) == -1)
        return 1;
    tty_send_n(&tcaps.newline, 2);
    tty_deinit();
    return 0;
}
