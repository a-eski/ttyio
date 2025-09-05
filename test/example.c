#include <assert.h>
#include <limits.h>
#include <unistd.h>

// #define TTYIO_NDEBUG
#include "common.h"
#include "../ttyio.h"
#include "../ttyplatform.h"

int main(void)
{
    tty_init(TTY_NONCANONICAL_MODE);
    tty_send(&tcaps.scr_clr);
    tty_send(&tcaps.cursor_home);
    assert_trap(term.pos.x == 0 && term.pos.y == 0);

    tty_print("term.pos.x " SIZE_T_FMT ", term.pos.y " SIZE_T_FMT, term.pos.x, term.pos.y);
    assert_trap(term.pos.y == 0);
    assert_trap(term.pos.x == 27);
    tty_print("term.pos.x " SIZE_T_FMT ", term.pos.y " SIZE_T_FMT, term.pos.x, term.pos.y);
    assert_trap(term.pos.y == 0);
    assert_trap(term.pos.x == 55);
    tty_send(&tcaps.newline);
    assert_trap(term.pos.y == 1);
    assert_trap(term.pos.x == 0);
    tty_send(&tcaps.bs);
    tty_y_adjust();
    assert_trap(term.pos.y == 0);
    assert_trap(term.pos.x == term.size.x);

    char c;
    if (read(STDIN_FILENO, &c, 1) == -1)
        return 1;
    tty_send_n(&tcaps.newline, 2);
    tty_deinit();
    return 0;
}
