#pragma once

#ifndef TCAPS_GUARD_H_
#define TCAPS_GUARD_H_

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

enum caps {
    CAP_BS,
    CAP_DEL,
    CAP_NEWLINE,

    CAP_SCR_CLR,
    CAP_SCR_CLR_TO_EOS,

    CAP_CURSOR_HOME,
    CAP_CURSOR_LEFT,
    CAP_CURSOR_RIGHT,
    CAP_CURSOR_UP,
    CAP_CURSOR_DOWN,
    CAP_CURSOR_SHOW,
    CAP_CURSOR_HIDE,
    CAP_CURSOR_SAVE,
    CAP_CURSOR_RESTORE,

    CAP_LINE_CLR_TO_EOL,
    CAP_LINE_CLR_TO_BOL,

    CAP_COLOR_RESET,
    CAP_COLOR_SET,
    CAP_COLOR_BG_SET
};

typedef struct {
    enum caps type;
    size_t len;
    const char* val;
} cap;

#define cap_New(s, t)                                                                                                  \
    (cap)                                                                                                              \
    {                                                                                                                  \
        .val = (s), .len = strlen((s)), .type = (t)                                                                    \
    }
#define cap_New_Lit(s, t)                                                                                              \
    (cap)                                                                                                              \
    {                                                                                                                  \
        .val = (s), .len = sizeof((s)), .type = (t)                                                                    \
    }
#define cap_New_s(s, n, t)                                                                                             \
    (cap)                                                                                                              \
    {                                                                                                                  \
        .val = (s), .len = (n), .type = (t)                                                                            \
    }

typedef struct {
    cap bs; /* Keys */
    cap del;
    cap newline;

    cap scr_clr; /* Screen */
    cap scr_clr_to_eos;

    cap cursor_home; /* Cursor */
    cap cursor_left;
    cap cursor_right;
    cap cursor_up;
    cap cursor_down;
    cap cursor_show;
    cap cursor_hide;
    cap cursor_save;
    cap cursor_restore;

    cap line_clr_to_eol; /* Line */
    cap line_clr_to_bol;

    int color_max; /* Colors */
    cap color_reset;
    cap color_set;
    cap color_bg_set;
} termcaps;

void tcaps_init();

#endif // !TCAPS_GUARD_H_
