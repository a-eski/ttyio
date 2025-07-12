/* Copyright ttyterm (C) by Alex Eski 2025 */
/* Licensed under GPLv3, see LICENSE for more information. */

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "lib/unibilium.h"
#include "tcaps.h"
#include "ttyterm.h"

unibi_term* uterm;
termcaps tcaps;
Terminal term;
static struct termios otios;

void fatal(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    // WARN: Don't use term_fprint here, if term size not set can cause infinite recursion.
    fprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);
    abort();
}

Coordinates term_size_get()
{
    struct winsize window;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);
    return (Coordinates){.x = window.ws_col, .y = window.ws_row};
}

void term_init()
{
    term = (Terminal){0};
    term.size = term_size_get();

    uterm = unibi_from_term(getenv("TERM"));
    if (!uterm)
        fatal("Can't find TERM from environment variables. Specify a terminal type with `setenv TERM <yourtype>'.\n");

    tcaps_init();

    if (!isatty(STDIN_FILENO)) {
        term_fprint(stderr, "Not running in a terminal.\n");
        exit(EXIT_FAILURE);
    }

    if (tcgetattr(STDIN_FILENO, &otios) != 0) {
        perror("Could not get terminal settings");
        exit(EXIT_FAILURE);
    }

    // mouse support? investigate
    // printf("\x1b[?1049h\x1b[0m\x1b[2J\x1b[?1003h\x1b[?1015h\x1b[?1006h\x1b[?25l");

    struct termios tios = otios;
    tios.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
    tios.c_cc[VMIN] = 1;
    tios.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tios) != 0) {
        perror("Could not set terminal settings");
    }

    // print("term.size.x %d, term.size.y %d\n", term.size.x, term.size.y);
}

void term_reset()
{
    fflush(stdout);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &otios) != 0) {
        perror("Could not restore terminal settings");
    }
    unibi_destroy(uterm);
}

void term_size_update(int printed)
{
    assert(printed != EOF);
    if (!term.size.x)
        fatal("Term size not set.\n");

    if (printed + term.pos.x + 1 > term.size.x) {
        // term.pos.y += (int)(printed / term.size.x);
        // printf("term.pos.y %d\n", term.pos.y);
        term.pos.x = (printed % term.size.x) + 1;
        // printf("term.pos.x %d\n", term.pos.x);
    }
    else {
        term.pos.x += printed + 1;
        // printf("term.pox.x %d\n", term.pos.x);
    }
}

int term_putc(const int c)
{
    int rv = putchar(c);
    term_size_update(1);
    return rv;
}

int term_write(const char* buf, const int n)
{
    int printed = write(STDOUT_FILENO, buf, n);
    assert(printed == n);
    term_size_update(printed);
    return printed;
}

void term_puts(const char* restrict str)
{
    puts(str);
    ++term.pos.y;
    term.pos.x = 0;
}

void term_print(const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vfprintf(stdout, fmt, args);
    va_end(args);
    fflush(stdout);

    term_size_update(printed);
}

void term_println(const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vfprintf(stdout, fmt, args);
    va_end(args);
    fflush(stdout);

    term_size_update(printed);
    term_send(&tcaps.newline);
}

void term_fprint(FILE* restrict file, const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vfprintf(file, fmt, args);
    va_end(args);
    fflush(stdout);

    term_size_update(printed);
}

int term_send(cap* c)
{
    assert(c && c->len);
    if (write(STDOUT_FILENO, c->val, c->len) == -1)
        return 1;
    fflush(stdout);

    switch (c->type) {
    case CAP_BS:
        --term.pos.x;
        break;
    case CAP_CURSOR_HOME: {
        term.pos.x = 0;
        term.pos.y = 0;
        break;
    }
    case CAP_CURSOR_RIGHT:
        ++term.pos.x;
        break;
    case CAP_CURSOR_LEFT:
        --term.pos.x;
        break;
    case CAP_CURSOR_UP:
        --term.pos.y;
        break;
    case CAP_CURSOR_DOWN:
        ++term.pos.y;
        break;
    case CAP_CURSOR_SAVE:
        term.saved_pos.x = term.pos.x;
        term.saved_pos.y = term.pos.y;
        break;
    case CAP_CURSOR_RESTORE:
        term.pos.x = term.saved_pos.x;
        term.pos.y = term.saved_pos.y;
        break;
    case CAP_NEWLINE:
        term.pos.x = 0;
        ++term.pos.y; // y handled in term_size_update currently.
    default:
        break;
    }

    return 0;
}

void term_send_n(cap* c, uint_fast32_t n)
{
    for (uint_fast32_t i = 0; i < n; ++i)
        term_send(c);
}

int term_color_set(int color)
{
    if (!tcaps.color_max)
        return 0;

    constexpr size_t size = 64;
    char buf[size] = {0};
    size_t len = unibi_run(tcaps.color_set.val, (unibi_var_t[9]){[0] = unibi_var_from_num(color)}, buf, size);

    if (write(STDOUT_FILENO, buf, len) == -1)
        return 1;
    fflush(stdout);
    return 0;
}

int term_color_bg_set(int color)
{
    if (!tcaps.color_max)
        return 0;

    constexpr size_t size = 64;
    char buf[size] = {0};
    size_t len = unibi_run(tcaps.color_bg_set.val, (unibi_var_t[9]){[0] = unibi_var_from_num(color)}, buf, size);

    if (write(STDOUT_FILENO, buf, len) == -1)
        return 1;
    fflush(stdout);
    return 0;
}

int term_goto_prev_eol()
{
    /*if (tcaps.line_goto_prev_eol.fallback == FB_NONE) {
        constexpr size_t size = 64;
        char buf[size] = {0};
        assert(term.pos.y > 0);
        size_t len = unibi_run(tcaps.cursor_pos.val,
                        (unibi_var_t[9]){
                            [0] = unibi_var_from_num(term.size.x - term.pos.x - 1),
                            [1] = unibi_var_from_num(--term.pos.y)
                        },
                        buf, size);

        if (write(STDOUT_FILENO, buf, len) == -1)
            return 1;
        term.pos.x = term.size.x - term.pos.x - 1;
        // --term.pos.y;
        return 0;
    }*/
    if (tcaps.line_goto_prev_eol.fallback >= FB_NONE) {
        term_send(&tcaps.cursor_up);
        // term_send(&tcaps.line_goto_bol);
        term_send_n(&tcaps.cursor_right, term.size.x - term.pos.x - 1);
        fflush(stdout);
        return 0;
    }

    unreachable();
}
