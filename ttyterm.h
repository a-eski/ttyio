/* Copyright ttyterm (C) by Alex Eski 2025 */
/* Licensed under GPLv3, see LICENSE for more information. */

#pragma once

#ifndef TERM_GUARD_H_
#define TERM_GUARD_H_

#include "tcaps.h"

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

void term_init();
void term_reset();

/* Output, tracks pos of cursor for you and stores in term */
int term_write(const char* buf, const int n);
int term_puts(const char* restrict str);
int term_print(const char* restrict fmt, ...);
int term_fprint(FILE* restrict file, const char* restrict fmt, ...);
int term_println(const char* restrict fmt, ...);

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

/* Advanced Output which can have multiple fallbacks.
 * Fallback handling is in ttyterm, tcaps just determines which method to use.
 */
int term_goto_prev_eol();

#endif /* !TERM_GUARD_H_ */
