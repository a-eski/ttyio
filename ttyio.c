/* Copyright ttyio (C) by Alex Eski 2025 */
/* Licensed under GPLv3, see LICENSE for more information. */
/* ttyio.h: implementation of public interfaces of the ttyio library */

#ifndef _POXIC_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* ifndef _POXIC_C_SOURCE */

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lib/unibilium.h"
#include "terminfo.h"
#include "tcaps.h"
#include "ttyio.h"
#include "ttyplatform.h" // used for macros

#define TTY_BUF_SIZE 64

unibi_term* uterm;
termcaps tcaps;
Terminal term;

static enum input_type tty_input_mode__;

// For unix like systems
#if !defined(_WIN32) && !defined(_WIN64)

#   include <sys/ioctl.h>

#   include <termios.h>

static struct termios otios__;
// For windows
#else

#   include <windows.h>

#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wformat-nonliteral"

int win_vdprintf__(int fd, const char* restrict fmt, ...) {
    char buffer[4096];

    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len < 0)
        return EOF;

    // write only the bytes vsnprintf produced
    return write(fd, buffer, (unsigned int)len);
}

#   pragma GCC diagnostic pop

#   define vdprintf(fd, fmt, args) win_vdprintf__(fd, fmt, args)

static DWORD omode__;

#endif /* if !defined(_WIN32) && !defined(_WIN64) */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"

void fatal__(const char* restrict fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    // WARN: Don't use tty_fprint here, if term size not set can cause infinite recursion.
    fprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);

    abort();
}

#pragma GCC diagnostic pop

Coordinates tty_size_get__(void)
{
#if !defined(_WIN32) && !defined(_WIN64)
    struct winsize window;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);
    assert(window.ws_col > 0);
    assert(window.ws_row > 0);
    return (Coordinates){.x = window.ws_col, .y = window.ws_row};
#else
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout == INVALID_HANDLE_VALUE) {
        perror("Error loading stdout handle");
        fatal__("\nCould not get screen info to determine size.\n");
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hStdout, &csbi)) {
        fatal__("\nCould not get screen info to determine size.\n");
    }

    int col = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int row = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    assert(col > 0);
    assert(row > 0);

    return (Coordinates){.x = (size_t)col, .y = (size_t)row};
#endif
}

inline Coordinates tty_get_pos__(void);
Coordinates tty_get_pos__(void)
{
    // NOTE: unibilium doesnt have a way to query the cursor position.
    // Need to query the terminal for the position on start,
    // so ttyio's tracking is accurate.
    // response format from terminal is "/033[{row};{col}R"
    if (write(STDOUT_FILENO, "\033[6n", 4) == -1)
        return (Coordinates){0};

    char buf[TTY_BUF_SIZE];
    int i = 0;
    while (i < (int)sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1)
            break;
        if (buf[i] == 'R')
            break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\033' || buf[1] != '[')
        return (Coordinates){0};


    int row, col;
    if (sscanf(buf + 2, "%d;%d", &row, &col) != 2)
        return (Coordinates){0};

    return (Coordinates){.x = col > 0 ? (size_t)col : 0, .y = row > 0 ? (size_t)row - 1 : 0};
}

void tty_init_pos__(void)
{
    term.pos = tty_get_pos__();
    // tty_println("init pos %zu %zu", term.pos.x, term.pos.y);

    assert(term.pos.x == 0 || term.pos.x <= term.size.x);
    assert(term.pos.y == 0 || term.pos.y <= term.size.y);
}

void tty_init_caps(void)
{
    term = (Terminal){0};
    term.size = tty_size_get__();
    assert(term.size.x && term.size.y);

    char* term_name = getenv("TERM");
    if (term_name) {
        uterm = unibi_from_term(term_name);
    }

    if (!uterm) {
        char* term_type;
        uterm = terminfo_from_builtin(term_name, &term_type);
        free(term_type);
    }

    if (!uterm) {
        fatal__("\nCan't find TERM from environment variables. Specify a terminal type with `setenv TERM <yourtype>'.\n");
    }

    tcaps_init();
}

void tty_init_input_mode(enum input_type input_type)
{
    tty_input_mode__ = input_type;
    if (input_type == TTY_CANONICAL_MODE) {
        return;
    }

#if !defined(_WIN32) && !defined(_WIN64)
    if (!isatty(STDIN_FILENO)) {
        tty_fprint(stderr, "Not running in a terminal.\n");
        exit(EXIT_FAILURE);
    }

    if (tcgetattr(STDIN_FILENO, &otios__) != 0) {
        perror("Could not get terminal settings");
        exit(EXIT_FAILURE);
    }
    // TODO: mouse support? investigate
    // printf("\x1b[?1049h\x1b[0m\x1b[2J\x1b[?1003h\x1b[?1015h\x1b[?1006h\x1b[?25l");

    struct termios tios = otios__;
    tios.c_iflag &= (tcflag_t) ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    tios.c_lflag &= (tcflag_t) ~(ECHO | ICANON | IEXTEN | ISIG);
    tios.c_iflag |= IUTF8;
    tios.c_cflag |= CS8;
    tios.c_cc[VMIN] = 1;
    tios.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tios) != 0) {
        perror("Could not set terminal settings");
    }
#else
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE) {
        perror("Error loading stdin handle");
        fatal__("\nCould not get terminal info to set to noncanonical mode.\n");
    }

    DWORD mode;
    if (!GetConsoleMode(hStdin, &mode)) {
        perror("Error loading console mode");
        fatal__("\nCould not get terminal info to set to noncanonical mode.\n");
    }
    omode__ = mode;

    // Disable line input and echo input
    mode &= (DWORD)~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);

    // mode &= ~(ENABLE_PROCESSED_INPUT);

    if (!SetConsoleMode(hStdin, mode)) {
        perror("Error setting console mode");
        fatal__("\nCould not set terminal info to set to noncanonical mode.\n");
    }
#endif

    tty_init_pos__();
}

void tty_init(enum input_type input_type)
{
    tty_init_caps();
    tty_init_input_mode(input_type);
}

/* tty_reinit__ *internal*
 * Just reinits the term variable, doesn't free memory from unibilium or touch tcaps.
 * Useful for testing.
 */
void tty_reinit__(void)
{
    term = (Terminal){0};
    term.size = tty_size_get__();
    assert(term.size.x && term.size.y);
}

void tty_deinit_caps(void)
{
    fflush(stdout);
    unibi_destroy(uterm);
}

void tty_deinit_input_mode(void)
{
    if (tty_input_mode__ == TTY_CANONICAL_MODE) {
        return;
    }

#if !defined(_WIN32) && !defined(_WIN64)
    if (tcsetattr(STDIN_FILENO, TCSANOW, &otios__) != 0) {
        perror("Could not restore terminal settings");
    }
#else
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(hStdin, omode__);
#endif /* if !defined(_WIN32) && !defined(_WIN64) */
}

void tty_deinit(void)
{
    tty_deinit_caps();
    tty_deinit_input_mode();
}

void tty_y_update__(const int printed)
{
    if (term.pos.y < term.size.y) {
        if (!printed)  {
            ++term.pos.y;
        }
        else if (term.pos.x == term.size.x && printed == 1) {
            ++term.pos.y;
        }
        // else if (term.pos.x + (size_t)printed > term.size.x) {
        else if (term.pos.x + (size_t)printed >= term.size.x) {
            double new_y = term.pos.x + printed / (double)term.size.x;
            if (new_y < 1) {
                return;
            }
            term.pos.y += (size_t)new_y;
            if (term.pos.y > term.size.y) {
                term.pos.y = term.size.y;
            }
        }
    }
    assert(term.pos.y <= term.size.y);
}

void tty_size_update__(const int printed)
{
    assert(printed != EOF);
    assert(term.size.x > 0);
    assert(term.size.y > 0);
#ifndef NDEBUG
    if (!term.size.x || !term.size.y) {
        fatal__("\nFatal error: term size not set.\n");
    }
#endif
    if (!printed) {
        return;
    }

    if ((size_t)printed + term.pos.x > term.size.x) {
        tty_y_update__(printed);
        term.pos.x = (size_t)(printed == 1 ? 0 : ((size_t)printed % term.size.x));
    }
    else {
        term.pos.x += (size_t)(printed == 1 ? 1 : printed + 1);
    }

    assert(term.pos.x <= term.size.x);
}

int tty_putc_invis()
{
    char c = '\n';
    _MAYBE_UNUSED_ int printed = write(STDOUT_FILENO, &c, 1);
    assert(printed != EOF && printed == 1);
    return 1;
}

int tty_putc(char c)
{
    _MAYBE_UNUSED_ int printed = write(STDOUT_FILENO, &c, 1);
    assert(printed != EOF && printed == 1);
    tty_size_update__(1);
    return 1;
}

int tty_fputc(FILE* restrict file, char c)
{
    _MAYBE_UNUSED_ int printed = write(fileno(file), &c, 1);
    assert(printed != EOF && printed == 1);
    tty_size_update__(1);
    return 1;
}

int tty_dputc(int fd, char c)
{
    _MAYBE_UNUSED_ int printed = write(fd, &c, 1);
    assert(printed != EOF && printed == 1);
    tty_size_update__(1);
    return 1;
}

int tty_write(const char* restrict buf, size_t n)
{
    int printed = write(STDOUT_FILENO, buf, n);
    assert(printed != EOF && (size_t)printed == n);
    tty_size_update__(printed);
    return printed;
}

int tty_writeln(const char* restrict buf, size_t n)
{
    int printed = write(STDOUT_FILENO, buf, n);
    assert(printed != EOF && (size_t)printed == n);
    tty_size_update__(printed);
    tty_send(&tcaps.newline);
    return printed;
}

int tty_fwrite(FILE* restrict file, const char* restrict buf, size_t n)
{
    int printed = write(fileno(file), buf, n);
    assert(printed != EOF && (size_t)printed == n);
    tty_size_update__(printed);
    return printed;
}

int tty_fwriteln(FILE* restrict file, const char* restrict buf, size_t n)
{
    int fd = fileno(file);
    int printed = write(fd, buf, n);
    assert(printed != EOF && (size_t)printed == n);
    tty_size_update__(printed);
    tty_dsend(fd, &tcaps.newline);
    return printed;
}

int tty_dwrite(int fd, const char* restrict buf, size_t n)
{
    int printed = write(fd, buf, n);
    assert(printed != EOF && (size_t)printed == n);
    tty_size_update__(printed);
    return printed;
}

int tty_dwriteln(int fd, const char* restrict buf, size_t n)
{
    int printed = write(fd, buf, n);
    assert(printed != EOF && (size_t)printed == n);
    tty_size_update__(printed);
    tty_dsend(fd, &tcaps.newline);
    return printed;
}

int tty_puts(const char* restrict str)
{
    int printed = puts(str);
    term.pos.x = 0;
    tty_y_update__(0);
    return printed;
}

int tty_fputs(const char* restrict str, FILE* restrict file)
{
    int printed = fputs(str, file);
    term.pos.x = 0;
    tty_send(&tcaps.newline);
    return printed;
}

int tty_print(const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vfprintf(stdout, fmt, args);
    va_end(args);
    fflush(stdout);

    tty_size_update__(printed);
    return printed;
}

int tty_println(const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vfprintf(stdout, fmt, args);
    va_end(args);
    fflush(stdout);

    tty_size_update__(printed);
    tty_send(&tcaps.newline);
    return printed;
}

int tty_fprint(FILE* restrict file, const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vfprintf(file, fmt, args);
    va_end(args);
    fflush(stdout);

    tty_size_update__(printed);
    return printed;
}

int tty_fprintln(FILE* restrict file, const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vfprintf(file, fmt, args);
    va_end(args);
    fflush(stdout);

    tty_size_update__(printed);
    tty_fsend(&tcaps.newline, file);
    return printed;
}

int tty_dprint(int fd, const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vdprintf(fd, fmt, args);
    va_end(args);
    fflush(stdout);

    tty_size_update__(printed);
    return printed;
}

int tty_dprintln(int fd, const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vdprintf(fd, fmt, args);
    va_end(args);
    fflush(stdout);

    tty_size_update__(printed);
    tty_dsend(fd, &tcaps.newline);
    return printed;
}

int tty_perror(const char* restrict msg)
{
    char* err_str = strerror(errno);
    tty_color_set(TTYIO_RED_ERROR);
    int printed = tty_fprint(stderr, "%s: ", msg);
    tty_color_reset();
    printed += tty_fprint(stderr, "%s", err_str);

    tty_size_update__(printed);
    tty_fsend(&tcaps.newline, stderr);
    return printed;
}

void tty_send_update__(cap* restrict c);
inline void tty_send_update__(cap* restrict c)
{
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
        if (term.pos.y > 0)
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
        tty_y_update__(0);
        break;
    case CAP_LINE_GOTO_BOL:
        term.pos.x = 0;
        break;
    default:
        break;
    }

    if (term.pos.x >= term.size.x && term.pos.x > 0) {
        term.pos.x = term.size.x % term.pos.x;
    }
    assert(term.pos.x <= term.size.x);
    assert(term.pos.y <= term.size.y);
}

int tty_send(cap* restrict c)
{
    assert(c && c->len);
    if (write(STDOUT_FILENO, c->val, c->len) == -1)
        return 1;
    fflush(stdout);

    tty_send_update__(c);
    assert(term.pos.y <= term.size.y);
    assert(term.pos.x <= term.size.x);
    return 0;
}

int tty_fsend(cap* restrict c, FILE* restrict file)
{
    assert(c && c->len);
    fwrite(c->val, sizeof(char), c->len, file);
    fflush(file);

    tty_send_update__(c);

    assert(term.pos.y <= term.size.y);
    assert(term.pos.x < term.size.x);
    return 0;
}

int tty_dsend(int fd, cap* restrict c)
{
    assert(c && c->len);
    if (write(fd, c->val, c->len) == -1)
        return 1;
    fflush(stdout);

    tty_send_update__(c);

    return 0;
}

void tty_send_n(cap* restrict c, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        tty_send(c);
    }
}

void tty_fsend_n(cap* restrict c, size_t n, FILE* restrict file)
{
    for (size_t i = 0; i < n; ++i) {
        tty_fsend(c, file);
    }
}

void tty_dsend_n(int fd, cap* restrict c, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        tty_dsend(fd, c);
    }
}

int tty_color_set(int color)
{
    if (!tcaps.color_max)
        return 0;

    char buf[TTY_BUF_SIZE] = {0};
    size_t len = unibi_run(tcaps.color_set.val, (unibi_var_t[9]){[0] = unibi_var_from_num(color)}, buf, TTY_BUF_SIZE);

    if (write(STDOUT_FILENO, buf, len) == -1)
        return 1;
    fflush(stdout);
    return 0;
}

int tty_color_bg_set(int color)
{
    if (!tcaps.color_max)
        return 0;

    char buf[TTY_BUF_SIZE] = {0};
    size_t len = unibi_run(tcaps.color_bg_set.val, (unibi_var_t[9]){[0] = unibi_var_from_num(color)}, buf, TTY_BUF_SIZE);

    if (write(STDOUT_FILENO, buf, len) == -1)
        return 1;
    fflush(stdout);
    return 0;
}

int tty_goto_prev_eol(void)
{
    // TODO: review this and rework, it scrolls up multiple times instead of 1 in some cases.
    /*if (tcaps.line_goto_prev_eol.fallback == FB_NONE) {
        char buf[TTY_BUF_SIZE] = {0};
        assert(term.pos.y > 0);
        // if y is 0 scroll up?
        size_t len = unibi_run(tcaps.cursor_pos.val,
                        (unibi_var_t[9]){
                            [0] = unibi_var_from_num(term.pos.y == 0 ? 0 : term.pos.y - 1),
                            [1] = unibi_var_from_num(term.size.x - 1)
                        },
                        buf, TTY_BUF_SIZE);

        if (write(STDOUT_FILENO, buf, len) == -1)
            return 1;
        fflush(stdout);
        term.pos.x = term.size.x - 1;
        --term.pos.y;

        assert(term.pos.y <= term.size.y);
        assert(term.pos.x < term.size.x);
        return 0;
    }*/
    if (tcaps.line_goto_prev_eol.fallback >= FB_NONE) {
        tty_send(&tcaps.cursor_up);
        tty_send_n(&tcaps.cursor_right, term.size.x);
        fflush(stdout);

        assert(term.pos.y <= term.size.y);
        assert(term.pos.x <= term.size.x);
        return 0;
    }

    unreachable();
    return -1;
}

int tty_y_adjust(void)
{
    if (term.pos.x == 0) {
        tty_goto_prev_eol();
        tty_send(&tcaps.scr_clr_to_eos);
        return -1;
    }

    return 0;
}
