/* Copyright ttyterm (C) by Alex Eski 2025 */
/* Licensed under GPLv3, see LICENSE for more information. */
/* ttyterm.h: public interface for the ttyterm library */

#pragma once

#ifndef TERM_GUARD_H_
#define TERM_GUARD_H_

/* WARN: current implementation is not tested to be threadsafe,
 * despite unibilium being threadsafe.
 */

#include "tcaps.h"

#define TERM_RED_ERROR 196

typedef struct {
    size_t x;
    size_t y;
} Coordinates;

typedef struct {
    Coordinates pos;
    Coordinates size;
    Coordinates saved_pos;
} Terminal;

extern termcaps tcaps;
extern Terminal term;

/* enum input_type
 * Canonical: read line by line, only get the line after user presses enter. a lot of programs work this way.
 * Noncanonical: read character by character. programs who need control over each input need to use this.
 */
enum input_type: char {
    TTY_CANONICAL_MODE,
    TTY_NONCANONICAL_MODE
};

void term_init(enum input_type input_type);
void term_reset();

/* Output, tracks pos of cursor for you and stores in term */
int term_putc(const char c);
int term_fputc(FILE* restrict file, const char c);
int term_dputc(int fd, const char c);

int term_write(const char* restrict buf, const size_t n);
int term_writeln(const char* restrict buf, const size_t n);
int term_fwrite(const int fd, const char* restrict buf, const size_t n);
int term_fwriteln(const int fd, const char* restrict buf, const size_t n);
// TODO: term_dwrite, term_dwriteln?

int term_puts(const char* restrict str);
int term_fputs(const char* restrict str, FILE* restrict file);

int term_print(const char* restrict fmt, ...)
    __attribute__ ((__format__ (__printf__, 1, 2)));
int term_println(const char* restrict fmt, ...)
    __attribute__ ((__format__ (__printf__, 1, 2)));
int term_fprint(FILE* restrict file, const char* restrict fmt, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));
int term_fprintln(FILE* restrict file, const char* restrict fmt, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));
int term_dprint(const int fd, const char* restrict fmt, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));
int term_dprintln(const int fd, const char* restrict fmt, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));

int term_perror(const char* restrict msg);

/* Output using tcaps, fallsback to ASCII control characters if cap not found */
int term_send(cap* restrict c);
int term_dsend(const int fd, cap* restrict c);
int term_fsend(cap* restrict c, FILE* restrict file);
void term_send_n(cap* restrict c, const size_t n);
void term_dsend_n(const int fd, cap* restrict c, const size_t n);
void term_fsend_n(cap* restrict c, const size_t n, FILE* restrict file);

/* Colors */
/* Colors don't have a fallback. If tcaps.color_max is 0, no color is set.
* TODO: If int color is greater than tcaps.color_max, it tries to use a reasonably similar color,
 * that is less than tcaps.color_max. */
int term_color_set(const int color);
int term_color_bg_set(const int color);
#define term_color_reset() term_send(&tcaps.color_reset)

/* Advanced Output which can have multiple fallbacks.
 * Fallback handling is in ttyterm, tcaps just determines which method to use.
 */
int term_goto_prev_eol();

#endif /* !TERM_GUARD_H_ */
