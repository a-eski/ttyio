#include <assert.h>
#include <limits.h>
#include <unistd.h>

#include "../ttyio.h"
#include "common.h"

Coordinates tty_get_pos__(void);

#define PROMPT "ttyio > "
void prompt()
{
    static_assert(sizeof(PROMPT) - 1 == 8);
    tty_write(PROMPT, sizeof(PROMPT) - 1);
    assert(term.pos.x == sizeof(PROMPT));
}

void bs()
{
    if (term.start.x == term.pos.x && term.start.y == term.pos.y)
        return;

    tty_send(&tcaps.bs);
    tty_y_adjust();
}

/* repl: some tests and example usage */
int main(void)
{
    tty_init(TTY_NONCANONICAL_MODE);
    tty_send(&tcaps.scr_clr);
    tty_send(&tcaps.cursor_home);

    char c;
    prompt();
    tty_set_start();

    while (read(STDIN_FILENO, &c, 1) > 0) {
        trap_on(c == (int)'q');
        switch (c) {
            case 127:
                bs();
                break;
            case '\r':
            case '\n':
                tty_send(&tcaps.newline);
                prompt();
                tty_set_start();
                break;
            default:
                tty_putc(c);
                // if (term.pos.x == 0)
                    // tty_putc_invis();
                break;
        }
    }

    tty_send(&tcaps.newline);
    tty_deinit();
    return 0;
}
