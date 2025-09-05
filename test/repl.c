#include <assert.h>
#include <limits.h>
#include <unistd.h>

#include "../ttyio.h"
#include "common.h"

#define PROMPT "ttyio > "
void prompt()
{
    static_assert(sizeof(PROMPT) - 1 == 8);
    tty_write(PROMPT, sizeof(PROMPT) - 1);
    assert(term.pos.x == sizeof(PROMPT));
}

/* repl: some tests and example usage */
int main(void)
{
    tty_init(TTY_NONCANONICAL_MODE);

    char c;
    prompt();

    while (read(STDIN_FILENO, &c, 1) > 0) {
        trap_on(c == (int)'q');
        switch (c) {
            case 127:
                tty_y_adjust();
                tty_send(&tcaps.bs);
                fflush(stdout);
                break;
            case '\r':
            case '\n':
                tty_send(&tcaps.newline);
                prompt();
                break;
            default:
                tty_putc(c);
                break;
        }
    }

    tty_send(&tcaps.newline);
    tty_deinit();
    return 0;
}
