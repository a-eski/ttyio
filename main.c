#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ttyterm.h"

void multiline(double multiplier)
{
    // size_t prev_y = term.pos.y;
    size_t n = term.size.x * multiplier;
    char* multiline = malloc(n);
    memset(multiline, 0, n);
    for (size_t i = 0; i < n; ++i) {
        multiline[i] = 'a' + i % 26;
    }
    term_print(multiline);
    free(multiline);
    // assert(term.pos.y == prev_y + (n / term.size.x));
    assert(term.pos.x == (n % term.size.x) + 1);
}

/* example usage */
/* These tests were done with term.size.x 171, term.size.y 46 */
int main()
{
    term_init();

    term_send(&tcaps.scr_clr);
    term_send(&tcaps.cursor_home);
    term_puts("Cleared screen with unibi");
    assert(term.pos.y == 1);
    assert(!term.pos.x);

    term_puts("hello from newline");
    assert(term.pos.y == 2);
    assert(!term.pos.x);

    term_puts("hello from newline");
    assert(term.pos.y == 3);
    assert(!term.pos.x);

    term_print("hello from newline");
    assert(term.pos.y == 3);
    assert(term.pos.x = 19);

    term_send_n(&tcaps.bs, 4);
    assert(term.pos.y == 3);
    assert(term.pos.x = 15);

    term_puts("hi");
    assert(term.pos.y == 4);
    assert(!term.pos.x);

    term_send_n(&tcaps.cursor_down, 2);
    assert(term.pos.y == 6);
    assert(!term.pos.x);

    term_puts("hey");
    assert(term.pos.y == 7);
    assert(!term.pos.x);

    term_send(&tcaps.cursor_home);
    assert(!term.pos.y);
    assert(!term.pos.x);

    term_print("Written");
    assert(!term.pos.y);
    assert(term.pos.x == 8);

    term_send_n(&tcaps.cursor_down, 8);
    assert(term.pos.y == 8);
    assert(term.pos.x == 8);

    term_print("term.pos.x %d, term.pos.y %d ", term.pos.x, term.pos.y);
    assert(term.pos.y == 8);
    assert(term.pos.x == 36);

    term_println("term.pos.x %d, term.pos.y %d", term.pos.x, term.pos.y);
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

    term_color_set(1);
    term_color_bg_set(10);
    term_println("hi");
    term_send(&tcaps.color_reset);
    assert(term.pos.y == 14);
    assert(!term.pos.x);

    term_println("term.pos.x %d, term.pos.y %d", term.pos.x, term.pos.y);
    assert(term.pos.y == 15);
    assert(!term.pos.x);

    term_println("term.pos.x %d, term.pos.y %d", term.pos.x, term.pos.y);
    assert(term.pos.y == 16);
    assert(!term.pos.x);

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

    char test[] = "test expansion working";
    term_print("%s test", test);
    assert(term.pos.y == 16);
    assert(term.pos.x == 28);

    term_send(&tcaps.newline);
    assert(term.pos.y == 17);
    assert(!term.pos.x);

    multiline(1.5);

    term_send_n(&tcaps.newline, 3);
    assert(term.pos.y == 21);
    assert(!term.pos.x);

    multiline(3.4);
    assert(term.pos.y == 24);

    term_println("term.pos.x %d, term.pos.y %d", term.pos.x, term.pos.y);
    term_send_n(&tcaps.newline, 3);
    assert(term.pos.y == 28);
    assert(!term.pos.x);

    term_println("term.size.x %d, term.size.y %d", term.size.x, term.size.y);
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

    multiline(.8);
    assert(term.pos.y == 44);
    multiline(.8);
    assert(term.pos.y == 44);
    term_println("term.pos.x %d, term.pos.y %d", term.pos.x, term.pos.y);
    assert(term.pos.y == 45);

    char c;
    if (read(STDIN_FILENO, &c, 1) == -1)
        return 1;

    term_send(&tcaps.newline);
    term_reset();
    return 0;
}
