/* Copyright ttyterm (C) by Alex Eski 2025 */
/* Licensed under GPLv3, see LICENSE for more information. */

#pragma once

#ifndef TERM_GUARD_H_
#define TERM_GUARD_H_

#include "tcaps.h"

typedef struct {
    int x;
    int y;
} Coordinates;

typedef struct {
    Coordinates pos;
    Coordinates size;
    Coordinates saved_pos;
} Terminal;

extern termcaps tcaps;
extern Terminal term;

void term_init();
void term_reset();

/* Output, tracks pos of cursor and stores in term */
int term_putc(const int c);
int term_write(const char* buf, const int n);
void term_puts(const char* restrict str);
void term_print(const char* restrict fmt, ...);
void term_fprint(FILE* restrict file, const char* restrict fmt, ...);
void term_println(const char* restrict fmt, ...);
void term_line_next();

/* Output using tcaps, fallsback to ASCII control characters if cap not found */
int term_send(cap* restrict c);
void term_send_n(cap* restrict c, uint_fast32_t n);

/* Colors */
/* Colors don't have a fallback. If tcaps.color_max is 0, no color is set.
* If int color is greater than tcaps.color_max, it tries to use a reasonably similar color,
 * that is less than tcaps.color_max. */
int term_color_set(int color);
int term_color_bg_set(int color);
// reset colors with 'term_send(&tcaps.color_reset)'

/* Advanced Output with multiple fallbacks.
 * Fallback handling is in ttyterm, tcaps just determines which method to use.
 */
int term_goto_prev_eol();

#endif /* !TERM_GUARD_H_ */
