#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// #define TTYIO_NDEBUG
#include "common.h"
#include "../ttyio.h"
#include "../ttyplatform.h"

/* main: a variety of tests and example usage */
/* These tests were done with term.size.x 171, term.size.y 46
 * The x size doesn't need to be exact for the tests, but the y does if NDEBUG not defined.
 * The tests are currently dependent on ordering.
 */

/* Internal testing method that reinits the term variable, doesn't free memory from unibilium or touch tcaps. */
void tty_reinit__(void);
/* Internal method that updates x and y, used here for testing */
void tty_size_update__(int printed);
/* Internal method that updates y, used here for testing */
void tty_y_update__(int printed);

void tty_y_update_tests(void)
{
    // when printed is 0, y should be incremented by 1
    _MAYBE_UNUSED_ size_t prev_y = term.pos.y;
    tty_y_update__(0);
    assert_trap(prev_y < term.pos.y);
    assert_trap(term.pos.y == 1);

    // when printed is less than size x, y should not be updated
    prev_y = term.pos.y;
    tty_y_update__(10);
    assert_trap(prev_y == term.pos.y);
    assert_trap(term.pos.y == 1);

    // when printed is greater than size x, y should be updated
    prev_y = term.pos.y;
    tty_y_update__(term.size.x * 2.5);
    assert_trap(prev_y < term.pos.y);
    assert_trap(term.pos.y == 3);
    assert_trap(!term.pos.x);

    // not super relevant for just y...
    // when going to max x (size x), y should not change
    prev_y = term.pos.y;
    tty_y_update__((int)(term.size.x));
    assert_trap(prev_y == term.pos.y - 1);
    assert_trap(term.pos.y == 4);

    // not super relevant for just y...
    // when going from max x (size x) to next x, y should increment by 1
    prev_y = term.pos.y;
    tty_y_update__(0);
    assert_trap(prev_y < term.pos.y);
    assert_trap(term.pos.y == 5);

    // when printed is greater than the screen size, y should be size y
    prev_y = term.pos.y;
    size_t printed = term.size.x * term.size.y + 2;
    assert_trap(printed > 0 && printed < INT_MAX);
    tty_y_update__((int)printed);
    assert_trap(prev_y < term.pos.y);
    assert_trap(term.pos.y == term.size.y);
}

void tty_size_update_tests(void)
{
    // When nothing is printed, x should not change. 0 only has special meaning for tty_y_update__
    _MAYBE_UNUSED_ size_t prev_x = term.pos.x;
    tty_size_update__(0);
    assert_trap(!term.pos.y);
    assert (prev_x == term.pos.x);
    assert_trap(!term.pos.x);

    // When 1 character is printed, x should increment by 1
    prev_x = term.pos.x;
    tty_size_update__(1);
    assert_trap(!term.pos.y);
    assert_trap(prev_x < term.pos.x);
    assert_trap(term.pos.x == 1);

    // when x is on the last position, it should be equals to size x
    prev_x = term.pos.x;
    // assert_trap(term.size.x - term.pos.x == term.size.x);
    assert_trap(term.size.x > term.pos.x);
    tty_size_update__((int)(term.size.x - term.pos.x)); // term.size.x - term.pos.x AKA 1
    assert_trap(term.pos.y = 1);
    assert_trap(prev_x < term.pos.x);
    // assert_trap(term.pos.x == term.size.x);

    // when x is on the last position and a char is printed, x should be back to 0
    prev_x = term.pos.x;
    tty_size_update__(1);
    // assert_trap(term.pos.y == 1);
    assert_trap(prev_x > term.pos.x);
    assert_trap(!term.pos.x);

    // when printed is longer than size x, x should wrap around and not be greater than size x
    prev_x = term.pos.x;
    size_t printed = term.size.x * 1.5;
    assert_trap(printed > 0 && printed < INT_MAX);
    tty_size_update__((int)printed);
    // assert_trap(term.pos.y == 2);
    assert_trap(prev_x < term.pos.x);
    assert_trap(term.pos.x < term.size.x);
    assert_trap(term.pos.x = printed % term.size.x);

}

void multiline(double multiplier)
{
    // size_t prev_y = term.pos.y;
    size_t n = term.size.x * multiplier + 1;
    char* multiline = malloc(n);
    memset(multiline, 0, n);
    for (size_t i = 0; i < n - 1; ++i) {
        multiline[i] = 'a' + i % 26;
    }
    _MAYBE_UNUSED_ int printed = tty_print("%s", multiline);
    free(multiline);
    // assert_trap(term.pos.y == prev_y + (n / term.size.x));
    // assert_trap(term.pos.x == ((size_t)printed % term.size.x) + 1);
}

void scr_clear_then_home_test(void)
{
    tty_send(&tcaps.scr_clr);
    tty_send(&tcaps.cursor_home);
    assert_trap(!term.pos.y);
    assert_trap(!term.pos.x);
}

void puts_test(void)
{
    tty_puts("Cleared screen with unibi");
    assert_trap(term.pos.y == 1);
    assert_trap(!term.pos.x);

    tty_puts("hello from newline");
    assert_trap(term.pos.y == 2);
    assert_trap(!term.pos.x);

    tty_puts("hello from newline");
    assert_trap(term.pos.y == 3);
    assert_trap(!term.pos.x);
}

void print_test(void)
{
    tty_print("hello from newline");
    assert_trap(term.pos.y == 3);
    assert_trap(term.pos.x = 19);
}

void bs_and_cursor_left_right_test(void)
{
    tty_send_n(&tcaps.bs, 4);
    assert_trap(term.pos.y == 3);
    assert_trap(term.pos.x = 15);

    tty_send(&tcaps.cursor_left);
    assert_trap(term.pos.y == 3);
    assert_trap(term.pos.x = 14);

    tty_send(&tcaps.cursor_left);
    assert_trap(term.pos.y == 3);
    assert_trap(term.pos.x = 13);

    tty_send_n(&tcaps.cursor_left, 2);
    assert_trap(term.pos.y == 3);
    assert_trap(term.pos.x = 11);

    tty_send_n(&tcaps.cursor_right, 4);
    assert_trap(term.pos.y == 3);
    assert_trap(term.pos.x = 15);
}

void cursor_down_test(void)
{
    tty_puts("hi");
    assert_trap(term.pos.y == 4);
    assert_trap(!term.pos.x);

    tty_send_n(&tcaps.cursor_down, 2);
    assert_trap(term.pos.y == 6);
    assert_trap(!term.pos.x);

    tty_puts("hey");
    assert_trap(term.pos.y == 7);
    assert_trap(!term.pos.x);
}

void cursor_home_test(void)
{
    tty_send(&tcaps.cursor_home);
    assert_trap(!term.pos.y);
    assert_trap(!term.pos.x);

    tty_print("Written");
    assert_trap(!term.pos.y);
    assert_trap(term.pos.x == 8);
}

void moving_around_and_rewriting_test(void)
{
    tty_send_n(&tcaps.cursor_down, 8);
    assert_trap(term.pos.y == 8);
    assert_trap(term.pos.x == 8);

    tty_print("term.pos.x " SIZE_T_FMT ", term.pos.y " SIZE_T_FMT " ", term.pos.x, term.pos.y);
    assert_trap(term.pos.y == 8);
    assert_trap(term.pos.x == 36);

    tty_println("term.pos.x " SIZE_T_FMT ", term.pos.y " SIZE_T_FMT, term.pos.x, term.pos.y);
    assert_trap(term.pos.y == 9);
    assert_trap(!term.pos.x);

    tty_send(&tcaps.newline);
    assert_trap(term.pos.y == 10);
    assert_trap(!term.pos.x);

    tty_print("hello world");
    assert_trap(term.pos.y == 10);
    assert_trap(term.pos.x == 12);

    tty_send_n(&tcaps.cursor_left, 6);
    assert_trap(term.pos.y == 10);
    assert_trap(term.pos.x == 6);
}

void line_clear_eol_and_bol_test(void)
{
    tty_send(&tcaps.line_clr_to_eol);
    assert_trap(term.pos.y == 10);
    assert_trap(term.pos.x == 6);

    tty_send_n(&tcaps.newline, 2);
    assert_trap(term.pos.y == 12);
    assert_trap(!term.pos.x);

    tty_print("hello world");
    assert_trap(term.pos.y == 12);
    assert_trap(term.pos.x = 12);

    tty_send_n(&tcaps.cursor_left, 6);
    assert_trap(term.pos.y == 12);
    assert_trap(term.pos.x = 6);

    tty_send(&tcaps.line_clr_to_bol);
    assert_trap(term.pos.y == 12);
    assert_trap(term.pos.x = 6);

    tty_send(&tcaps.cursor_down);
    assert_trap(term.pos.y == 13);
    assert_trap(term.pos.x = 6);
}

void fg_and_bg_color_test(void)
{
    tty_color_set(1);
    tty_color_bg_set(10);
    tty_println("hi");
    tty_send(&tcaps.color_reset);
    assert_trap(term.pos.y == 14);
    assert_trap(!term.pos.x);
}

void println_test(void)
{
    tty_println("term.pos.x " SIZE_T_FMT ", term.pos.y " SIZE_T_FMT, term.pos.x, term.pos.y);
    assert_trap(term.pos.y == 15);
    assert_trap(!term.pos.x);

    tty_println("term.pos.x " SIZE_T_FMT ", term.pos.y " SIZE_T_FMT, term.pos.x, term.pos.y);
    assert_trap(term.pos.y == 16);
    assert_trap(!term.pos.x);
}

void line_goto_prev_eol_and_eol_test(void)
{
    tty_goto_prev_eol();
    assert_trap(term.pos.y == 15);
    // assert_trap(term.pos.x == term.size.x);

    tty_send(&tcaps.cursor_left);
    assert_trap(term.pos.y == 15);
    // assert_trap(term.pos.x == term.size.x - 2);

    tty_write("a", 1);
    assert_trap(term.pos.y == 16);
    // assert_trap(term.pos.x == term.size.x);

    tty_send(&tcaps.newline);
    assert_trap(term.pos.y == 17);
    assert_trap(!term.pos.x);
}

void fmt_expansion_test(void)
{
    char test[] = "test expansion working";
    tty_print("%s test", test);
    assert_trap(term.pos.y == 17);
    assert_trap(term.pos.x == 28);

    tty_send(&tcaps.newline);
    assert_trap(term.pos.y == 18);
    assert_trap(!term.pos.x);
}

void multiline_test(void)
{
    multiline(1.5);

    tty_send_n(&tcaps.newline, 3);
    assert_trap(term.pos.y == 22);
    assert_trap(!term.pos.x);

    multiline(3.4);
    assert_trap(term.pos.y == 25);

    tty_println("term.pos.x " SIZE_T_FMT ", term.pos.y " SIZE_T_FMT, term.pos.x, term.pos.y);
    tty_send_n(&tcaps.newline, 3);
    assert_trap(term.pos.y == 29);
    assert_trap(!term.pos.x);

    tty_println("term.size.x " SIZE_T_FMT ", term.size.y " SIZE_T_FMT, term.size.x, term.size.y);
    assert_trap(term.pos.y == 30);
    assert_trap(!term.pos.x);

    multiline(6.7);
    assert_trap(term.pos.y == 36);
    tty_send_n(&tcaps.newline, 3);
    assert_trap(term.pos.y == 39);
    assert_trap(!term.pos.x);

    multiline(6.7);
    assert_trap(term.pos.y == 45);
    tty_send(&tcaps.newline);
    assert_trap(term.pos.y == 46);
    assert_trap(!term.pos.x);
}

void last_line_test(void)
{
    tty_println("This is the last line!");
    assert_trap(term.pos.y == 45);
    assert_trap(!term.pos.x);

    tty_println("This is the last line!");
    assert_trap(term.pos.y == 45);
    assert_trap(!term.pos.x);

    tty_println("This is the last line!");
    assert_trap(term.pos.y == 45);
    assert_trap(!term.pos.x);

    tty_send_n(&tcaps.newline, 3);
    assert_trap(term.pos.y == 45);
    assert_trap(!term.pos.x);

    tty_send(&tcaps.cursor_up);
    assert_trap(term.pos.y == 44);
    assert_trap(!term.pos.x);
}

void bg_colors_test(void)
{
    for (int i = 0; i < tcaps.color_max; ++i) {
        tty_color_bg_set(i);
        tty_write(" ", 1);
    }
    tty_color_reset();
    assert_trap(term.pos.y == 45);
    tty_send(&tcaps.newline);
    assert_trap(term.pos.y == 45);
}

int main(void)
{
    tty_init(TTY_NONCANONICAL_MODE);

    scr_clear_then_home_test();
    tty_y_update_tests();
    tty_reinit__(); // reinit after messing with sizes for testing
    tty_size_update_tests();
    tty_reinit__();

    scr_clear_then_home_test();
    puts_test();
    bs_and_cursor_left_right_test();
    cursor_down_test();
    cursor_home_test();
    moving_around_and_rewriting_test();
    line_clear_eol_and_bol_test();
    fg_and_bg_color_test();
    println_test();
    line_goto_prev_eol_and_eol_test();
    fmt_expansion_test();
    multiline_test();
    last_line_test();
    bg_colors_test();

    multiline(.8);
    assert_trap(term.pos.y == 45);
    multiline(.8);
    assert_trap(term.pos.y == 45);
    tty_print("term.pos.x " SIZE_T_FMT ", term.pos.y " SIZE_T_FMT, term.pos.x, term.pos.y);
    assert_trap(term.pos.y == 45);

    tty_goto_prev_eol();
    assert_trap(term.pos.y == 44);
    assert_trap(term.pos.x == term.size.x);
    tty_print("term.pos.x " SIZE_T_FMT ", term.pos.y " SIZE_T_FMT, term.pos.x, term.pos.y);

    char c;
    if (read(STDIN_FILENO, &c, 1) == -1)
        return 1;
    tty_send_n(&tcaps.newline, 2);
    tty_deinit();
    return 0;
}
