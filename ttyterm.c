/* Copyright ttyterm (C) by Alex Eski 2025 */
/* Licensed under GPLv3, see LICENSE for more information. */

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

#include "terminfo.h"

#if !defined(_WIN32) && !defined(_WIN64)

#   include <sys/ioctl.h>

#   include <termios.h>

static struct termios otios;
#else

#   include <windows.h>

#   ifndef unreachable
#       define unreachable()
#   endif /* ifndef unreachable */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
int win_vdprintf(const int fd, const char* restrict format, va_list args) {
    char buffer[4096];
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    if (len < 0)
        return EOF;

    // write only the bytes vsnprintf produced
    return write(fd, buffer, (unsigned int)len);
}
#pragma GCC diagnostic pop

#   define vdprintf(fd, fmt, args) win_vdprintf(fd, fmt, args)

#   define setenv(name, value, replace) _putenv_s(name, value)

static DWORD omode;
#endif /* if !defined(_WIN32) && !defined(_WIN64) */

#include "lib/unibilium.h"
#include "tcaps.h"
#include "ttyterm.h"

unibi_term* uterm;
termcaps tcaps;
Terminal term;


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"

void fatal__(const char* restrict fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    // WARN: Don't use term_fprint here, if term size not set can cause infinite recursion.
    fprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);

    abort();
}
#pragma GCC diagnostic pop

Coordinates term_size_get__()
{
#if !defined(_WIN32) && !defined(_WIN64)
    struct winsize window;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);
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

    int columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    return (Coordinates){.x = columns, .y = rows};
#endif
}

void term_init()
{
    term = (Terminal){0};
    term.size = term_size_get__();
    assert(term.size.x && term.size.y);

    char* term_name = getenv("TERM");
    if (term_name) {
        uterm = unibi_from_term(term_name);
    }

    if (!uterm) {
        char* term_type;
        uterm = terminfo_from_builtin(term_name, &term_type);
        setenv("TERM", term_type, 1);
    }

    if (!uterm) {
        fatal__("\nCan't find TERM from environment variables. Specify a terminal type with `setenv TERM <yourtype>'.\n");
    }

    tcaps_init();

#if !defined(_WIN32) && !defined(_WIN64)
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
    omode = mode;

    // Disable line input and echo input
    mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);

    // mode &= ~(ENABLE_PROCESSED_INPUT);

    if (!SetConsoleMode(hStdin, mode)) {
        perror("Error setting console mode");
        fatal__("\nCould not set terminal info to set to noncanonical mode.\n");
    }
#endif
}

/* Just reinits the term variable, doesn't free memory from unibilium or touch tcaps. */
void term_reinit__()
{
    term = (Terminal){0};
    term.size = term_size_get__();
    assert(term.size.x && term.size.y);
}

void term_reset()
{
    fflush(stdout);

#if !defined(_WIN32) && !defined(_WIN64)
    if (tcsetattr(STDIN_FILENO, TCSANOW, &otios) != 0) {
        perror("Could not restore terminal settings");
    }
#else
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(hStdin, omode);
#endif /* if !defined(_WIN32) && !defined(_WIN64) */

    unibi_destroy(uterm);
}

void term_y_update__(const int printed)
{
    if (term.pos.y < term.size.y - 1) {
        if (!printed)  {
            ++term.pos.y;
        }
        else if (term.pos.x == term.size.x - 1 && printed == 1) {
            ++term.pos.y;
        }
        else if (term.pos.x + printed - 1 >= term.size.x) {
            double new_y = term.pos.x + printed / (double)term.size.x;
            if (new_y < 1) {
                return;
            }
            term.pos.y += (size_t)new_y;
            if (term.pos.y > term.size.y) {
                term.pos.y = term.size.y - 1;
            }
        }
    }
    assert(term.pos.y < term.size.y);
}

void term_size_update__(const int printed)
{
    assert(printed != EOF);
#ifndef NDEBUG
    if (!term.size.x || !term.size.y) {
        fatal__("\nFatal error: term size not set.\n");
    }
#endif
    if (!printed) {
        return;
    }

    if (printed + term.pos.x > term.size.x - 1) {
        term_y_update__(printed);
        term.pos.x = printed == 1 ? 0 : (printed % term.size.x);
    }
    else {
        term.pos.x += printed == 1 ? 1 : printed + 1;
    }
    assert(term.pos.x < term.size.x);
}

int term_putc(const char c)
{
    [[maybe_unused]] int printed = write(STDOUT_FILENO, &c, 1);
    assert(printed != EOF && printed == 1);
    term_size_update__(1);
    return 1;
}

int term_fputc(FILE* restrict file, const char c)
{
    [[maybe_unused]] int printed = write(fileno(file), &c, 1);
    assert(printed != EOF && printed == 1);
    term_size_update__(1);
    return 1;
}

int term_dputc(const int fd, const char c)
{
    [[maybe_unused]] int printed = write(fd, &c, 1);
    assert(printed != EOF && printed == 1);
    term_size_update__(1);
    return 1;
}

int term_write(const char* restrict buf, const size_t n)
{
    int printed = write(STDOUT_FILENO, buf, n);
    assert(printed != EOF && (size_t)printed == n);
    term_size_update__(printed);
    return printed;
}

int term_writeln(const char* restrict buf, const size_t n)
{
    int printed = write(STDOUT_FILENO, buf, n);
    assert(printed != EOF && (size_t)printed == n);
    term_size_update__(printed);
    term_send(&tcaps.newline);
    return printed;
}

int term_fwrite(const int fd, const char* restrict buf, const size_t n)
{
    int printed = write(fd, buf, n);
    assert(printed != EOF && (size_t)printed == n);
    term_size_update__(printed);
    return printed;
}

int term_fwriteln(const int fd, const char* restrict buf, const size_t n)
{
    int printed = write(fd, buf, n);
    assert(printed != EOF && (size_t)printed == n);
    term_size_update__(printed);
    term_dsend(fd, &tcaps.newline);
    return printed;
}

int term_puts(const char* restrict str)
{
    int printed = puts(str);
    term.pos.x = 0;
    term_y_update__(0);
    return printed;
}

int term_fputs(const char* restrict str, FILE* restrict file)
{
    int printed = fputs(str, file);
    term.pos.x = 0;
    term_send(&tcaps.newline);
    return printed;
}

int term_print(const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vfprintf(stdout, fmt, args);
    va_end(args);
    fflush(stdout);

    term_size_update__(printed);
    return printed;
}

int term_println(const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vfprintf(stdout, fmt, args);
    va_end(args);
    fflush(stdout);

    term_size_update__(printed);
    term_send(&tcaps.newline);
    return printed;
}

int term_fprint(FILE* restrict file, const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vfprintf(file, fmt, args);
    va_end(args);
    fflush(stdout);

    term_size_update__(printed);
    return printed;
}

int term_fprintln(FILE* restrict file, const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vfprintf(file, fmt, args);
    va_end(args);
    fflush(stdout);

    term_size_update__(printed);
    return printed;
}

int term_dprint(const int fd, const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vdprintf(fd, fmt, args);
    va_end(args);
    fflush(stdout);

    term_size_update__(printed);
    return printed;
}

int term_dprintln(const int fd, const char* restrict fmt, ...)
{
    int printed;
    va_list args;
    va_start(args, fmt);
    printed = vdprintf(fd, fmt, args);
    va_end(args);
    fflush(stdout);

    term_size_update__(printed);
    term_send(&tcaps.newline);
    return printed;
}

int term_perror(const char* restrict msg)
{
    char* err_str = strerror(errno);
    term_color_set(TERM_RED_ERROR);
    int printed = term_fprint(stderr, "%s: ", msg);
    term_color_reset();
    printed += term_fprint(stderr, "%s", err_str);

    term_size_update__(printed);
    term_fsend(&tcaps.newline, stderr);
    return printed;
}

void term_send_update__(cap* restrict c);
inline void term_send_update__(cap* restrict c)
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
        term_y_update__(0);
        break;
    case CAP_LINE_GOTO_BOL:
        term.pos.x = 0;
        break;
    default:
        break;
    }

    if (term.pos.x >= term.size.x) {
        term.pos.x = term.size.x % term.pos.x;
    }
}

int term_send(cap* restrict c)
{
    assert(c && c->len);
    if (write(STDOUT_FILENO, c->val, c->len) == -1)
        return 1;
    fflush(stdout);

    term_send_update__(c);
    assert(term.pos.y <= term.size.y);
    assert(term.pos.x <= term.size.x);
    return 0;
}

int term_fsend(cap* restrict c, FILE* restrict file)
{
    assert(c && c->len);
    fwrite(c->val, sizeof(char), c->len, file);
    fflush(file);

    term_send_update__(c);

    assert(term.pos.y < term.size.y);
    assert(term.pos.x < term.size.x);
    return 0;
}

int term_dsend(const int fd, cap* restrict c)
{
    assert(c && c->len);
    if (write(fd, c->val, c->len) == -1)
        return 1;
    fflush(stdout);

    term_send_update__(c);

    return 0;
}

void term_send_n(cap* restrict c, const size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        term_send(c);
    }
}

void term_fsend_n(cap* restrict c, const size_t n, FILE* restrict file)
{
    for (size_t i = 0; i < n; ++i) {
        term_fsend(c, file);
    }
}

void term_dsend_n(const int fd, cap* restrict c, const size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        term_dsend(fd, c);
    }
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

/*int term_color_reset()
{
    return term_send(&tcaps.color_reset);
}*/

int term_goto_prev_eol()
{
    if (tcaps.line_goto_prev_eol.fallback == FB_NONE) {
        constexpr size_t size = 64;
        char buf[size] = {0};
        assert(term.pos.y > 0);
        // if y is 0 scroll up?
        size_t len = unibi_run(tcaps.cursor_pos.val,
                        (unibi_var_t[9]){
                            [0] = unibi_var_from_num(term.pos.y == 0 ? 0 : term.pos.y - 1),
                            [1] = unibi_var_from_num(term.size.x - 1)
                        },
                        buf, size);

        if (write(STDOUT_FILENO, buf, len) == -1)
            return 1;
        fflush(stdout);
        term.pos.x = term.size.x - 1;
        --term.pos.y;

        assert(term.pos.y < term.size.y);
        assert(term.pos.x < term.size.x);
        return 0;
    }
    if (tcaps.line_goto_prev_eol.fallback >= FB_NONE) {
        term_send(&tcaps.cursor_up);
        term_send_n(&tcaps.cursor_right, term.size.x - term.pos.x - 1);
        fflush(stdout);
        term.pos.x = term.size.x - 1;
        --term.pos.y;

        assert(term.pos.y < term.size.y);
        assert(term.pos.x < term.size.x);
        return 0;
    }

    unreachable();
}

#pragma GCC diagnostic pop
