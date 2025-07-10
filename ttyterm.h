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

/* Output */
int term_putc(const int c);
void term_puts(const char* restrict str);
void term_print(const char* restrict fmt, ...);
void term_fprint(FILE* restrict file, const char* restrict fmt, ...);
void term_println(const char* restrict fmt, ...);
void term_line_next();
int term_send(cap* restrict c);
void term_send_n(cap* restrict c, uint_fast32_t n);
int term_color_set(int color);
int term_color_bg_set(int color);

#endif /* !TERM_GUARD_H_ */
