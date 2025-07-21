#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ttyterm.h"

/* Internal testing method that reinits the term variable, doesn't free memory from unibilium or touch tcaps. */
void term_reinit__();
/* Internal method that updates x and y, used here for testing */
void term_size_update__(int printed);
/* Internal method that updates y, used here for testing */
void term_y_update__(int printed);

void term_y_update_tests()
{
    // when printed is 0, y should be incremented by 1
    [[maybe_unused]] size_t prev_y = term.pos.y;
    term_y_update__(0);
    assert(prev_y < term.pos.y);
    assert(term.pos.y == 1);

    // when printed is less than size x, y should not be updated
    prev_y = term.pos.y;
    term_y_update__(10);
    assert(prev_y == term.pos.y);
    assert(term.pos.y == 1);

    // when printed is greater than size x, y should be updated
    prev_y = term.pos.y;
    term_y_update__(term.size.x * 2.5);
    assert(prev_y < term.pos.y);
    assert(term.pos.y == 3);
    assert(!term.pos.x);

    // not super relevant for just y...
    // when going to max x (size x - 1), y should not change
    prev_y = term.pos.y;
    term_y_update__(term.size.x - 1);
    assert(prev_y == term.pos.y);
    assert(term.pos.y == 3);

    // not super relevant for just y...
    // when going from max x (size x - 1) to next x, y should increment by 1
    prev_y = term.pos.y;
    term_y_update__(0);
    assert(prev_y < term.pos.y);
    assert(term.pos.y == 4);

    // when printed is greater than the screen size, y should be size y - 1
    prev_y = term.pos.y;
    term_y_update__(term.size.x * term.size.y + 2);
    assert(prev_y < term.pos.y);
    assert(term.pos.y == term.size.y - 1);
}

void term_size_update_tests()
{
    // When nothing is printed, x should not change. 0 only has special meaning for term_y_update__
    [[maybe_unused]] size_t prev_x = term.pos.x;
    term_size_update__(0);
    assert(!term.pos.y);
    assert (prev_x == term.pos.x);
    assert(!term.pos.x);

    // When 1 character is printed, x should increment by 1
    prev_x = term.pos.x;
    term_size_update__(1);
    assert(!term.pos.y);
    assert(prev_x < term.pos.x);
    assert(term.pos.x == 1);

    // when x is on the last position, it should be equals to size x - 1
    prev_x = term.pos.x;
    assert(term.size.x - term.pos.x == term.size.x - 1);
    term_size_update__(term.size.x - term.pos.x); // term.size.x - term.pos.x AKA 1
    assert(!term.pos.y);
    assert(prev_x < term.pos.x);
    assert(term.pos.x == term.size.x - 1);

    // when x is on the last position and a char is printed, x should be back to 0
    prev_x = term.pos.x;
    term_size_update__(1);
    assert(term.pos.y == 1);
    assert(prev_x > term.pos.x);
    assert(!term.pos.x);

    // when printed is longer than size x, x should wrap around and not be greater than size x
    prev_x = term.pos.x;
    size_t printed = term.size.x * 1.5;
    term_size_update__(printed);
    assert(term.pos.y == 2);
    assert(prev_x < term.pos.x);
    assert(term.pos.x < term.size.x);
    assert(term.pos.x = printed % term.size.x);

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
    [[maybe_unused]] int printed = term_print("%s", multiline);
    free(multiline);
    // assert(term.pos.y == prev_y + (n / term.size.x));
    // assert(term.pos.x == ((size_t)printed % term.size.x) + 1);
}

void scr_clear_then_home_test()
{
    term_send(&tcaps.scr_clr);
    term_send(&tcaps.cursor_home);
    assert(!term.pos.y);
    assert(!term.pos.x);
}

void puts_test()
{
    term_puts("Cleared screen with unibi");
    assert(term.pos.y == 1);
    assert(!term.pos.x);

    term_puts("hello from newline");
    assert(term.pos.y == 2);
    assert(!term.pos.x);

    term_puts("hello from newline");
    assert(term.pos.y == 3);
    assert(!term.pos.x);
}

void print_test()
{
    term_print("hello from newline");
    assert(term.pos.y == 3);
    assert(term.pos.x = 19);
}

void bs_and_cursor_left_right_test()
{
    term_send_n(&tcaps.bs, 4);
    assert(term.pos.y == 3);
    assert(term.pos.x = 15);

    term_send(&tcaps.cursor_left);
    assert(term.pos.y == 3);
    assert(term.pos.x = 14);

    term_send(&tcaps.cursor_left);
    assert(term.pos.y == 3);
    assert(term.pos.x = 13);

    term_send_n(&tcaps.cursor_left, 2);
    assert(term.pos.y == 3);
    assert(term.pos.x = 11);

    term_send_n(&tcaps.cursor_right, 4);
    assert(term.pos.y == 3);
    assert(term.pos.x = 15);
}

void cursor_down_test()
{
    term_puts("hi");
    assert(term.pos.y == 4);
    assert(!term.pos.x);

    term_send_n(&tcaps.cursor_down, 2);
    assert(term.pos.y == 6);
    assert(!term.pos.x);

    term_puts("hey");
    assert(term.pos.y == 7);
    assert(!term.pos.x);
}

void cursor_home_test()
{
    term_send(&tcaps.cursor_home);
    assert(!term.pos.y);
    assert(!term.pos.x);

    term_print("Written");
    assert(!term.pos.y);
    assert(term.pos.x == 8);
}

void moving_around_and_rewriting_test()
{
    term_send_n(&tcaps.cursor_down, 8);
    assert(term.pos.y == 8);
    assert(term.pos.x == 8);

    term_print("term.pos.x %zu, term.pos.y %zu ", term.pos.x, term.pos.y);
    assert(term.pos.y == 8);
    assert(term.pos.x == 36);

    term_println("term.pos.x %zu, term.pos.y %zu", term.pos.x, term.pos.y);
    assert(term.pos.y == 9);
    assert(!term.pos.x);

    term_send(&tcaps.newline);
    assert(term.pos.y == 10);
    assert(!term.pos.x);

    term_print("hello world");
    assert(term.pos.y == 10);
    assert(term.pos.x == 12);

    term_send_n(&tcaps.cursor_left, 6);
    assert(term.pos.y == 10);
    assert(term.pos.x == 6);
}

void line_clear_eol_and_bol_test()
{
    term_send(&tcaps.line_clr_to_eol);
    assert(term.pos.y == 10);
    assert(term.pos.x == 6);

    term_send_n(&tcaps.newline, 2);
    assert(term.pos.y == 12);
    assert(!term.pos.x);

    term_print("hello world");
    assert(term.pos.y == 12);
    assert(term.pos.x = 12);

    term_send_n(&tcaps.cursor_left, 6);
    assert(term.pos.y == 12);
    assert(term.pos.x = 6);

    term_send(&tcaps.line_clr_to_bol);
    assert(term.pos.y == 12);
    assert(term.pos.x = 6);

    term_send(&tcaps.cursor_down);
    assert(term.pos.y == 13);
    assert(term.pos.x = 6);
}

void fg_and_bg_color_test()
{
    term_color_set(1);
    term_color_bg_set(10);
    term_println("hi");
    term_send(&tcaps.color_reset);
    assert(term.pos.y == 14);
    assert(!term.pos.x);
}

void println_test()
{
    term_println("term.pos.x %zu, term.pos.y %zu", term.pos.x, term.pos.y);
    assert(term.pos.y == 15);
    assert(!term.pos.x);

    term_println("term.pos.x %zu, term.pos.y %zu", term.pos.x, term.pos.y);
    assert(term.pos.y == 16);
    assert(!term.pos.x);
}

void line_goto_prev_eol_and_eol_test()
{
    term_goto_prev_eol();
    assert(term.pos.y == 15);
    assert(term.pos.x == term.size.x - 1);

    term_send(&tcaps.cursor_left);
    assert(term.pos.y == 15);
    assert(term.pos.x == term.size.x - 2);

    term_write("a", 1);
    assert(term.pos.y == 15);
    assert(term.pos.x == term.size.x - 1);

    term_send(&tcaps.newline);
    assert(term.pos.y == 16);
    assert(!term.pos.x);
}

void fmt_expansion_test()
{
    char test[] = "test expansion working";
    term_print("%s test", test);
    assert(term.pos.y == 16);
    assert(term.pos.x == 28);

    term_send(&tcaps.newline);
    assert(term.pos.y == 17);
    assert(!term.pos.x);
}

void multiline_test()
{
    multiline(1.5);

    term_send_n(&tcaps.newline, 3);
    assert(term.pos.y == 21);
    assert(!term.pos.x);

    multiline(3.4);
    assert(term.pos.y == 24);

    term_println("term.pos.x %zu, term.pos.y %zu", term.pos.x, term.pos.y);
    term_send_n(&tcaps.newline, 3);
    assert(term.pos.y == 28);
    assert(!term.pos.x);

    term_println("term.size.x %zu, term.size.y %zu", term.size.x, term.size.y);
    assert(term.pos.y == 29);
    assert(!term.pos.x);

    multiline(6.7);
    assert(term.pos.y == 35);
    term_send_n(&tcaps.newline, 3);
    assert(term.pos.y == 38);
    assert(!term.pos.x);

    multiline(6.7);
    assert(term.pos.y == 44);
    term_send(&tcaps.newline);
    assert(term.pos.y == 45);
    assert(!term.pos.x);
}

void last_line_test()
{
    term_println("This is the last line!");
    assert(term.pos.y == 45);
    assert(!term.pos.x);

    term_println("This is the last line!");
    assert(term.pos.y == 45);
    assert(!term.pos.x);

    term_println("This is the last line!");
    assert(term.pos.y == 45);
    assert(!term.pos.x);

    term_send_n(&tcaps.newline, 3);
    assert(term.pos.y == 45);
    assert(!term.pos.x);

    term_send(&tcaps.cursor_up);
    assert(term.pos.y == 44);
    assert(!term.pos.x);
}

void bg_colors_test()
{
    for (int i = 0; i < tcaps.color_max; ++i) {
        term_color_bg_set(i);
        term_write(" ", 1);
    }
    term_color_reset();
    assert(term.pos.y == 45);
    term_send(&tcaps.newline);
    assert(term.pos.y == 45);
}

/* example usage */
/* These tests were done with term.size.x 171, term.size.y 46
 * The tests are currently dependent on ordering.
 */
int main()
{
    term_init();

    term_y_update_tests();
    term_reinit__(); // reinit after messing with sizes for testing
    term_size_update_tests();
    term_reinit__();

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
    assert(term.pos.y == 45);
    multiline(.8);
    assert(term.pos.y == 45);
    term_print("term.pos.x %zu, term.pos.y %zu", term.pos.x, term.pos.y);
    assert(term.pos.y == 45);

    term_goto_prev_eol();
    assert(term.pos.y == 44);
    assert(term.pos.x == term.size.x - 1);
    term_print("term.pos.x %zu, term.pos.y %zu", term.pos.x, term.pos.y);

    char c;
    if (read(STDIN_FILENO, &c, 1) == -1)
        return 1;
    term_send(&tcaps.newline);
    term_reset();
    return 0;
}
