#include <assert.h>
#include <limits.h>
#include <unistd.h>

#include "../ttyio.h"
#include "common.h"

#define PROMPT "alex alex/ttyio > "
void prompt()
{
    static_assert(sizeof(PROMPT) - 1 == 18);
    tty_write(PROMPT, sizeof(PROMPT) - 1);
    fflush(stdout);
    assert(term.pos.x == sizeof(PROMPT));
}

/* repl: some tests and example usage */
int main(void)
{
    tty_init(TTY_NONCANONICAL_MODE);

    char c;
    bool reprompt = false;
    prompt();

    while (read(STDIN_FILENO, &c, 1) > 0) {
        if (reprompt) {
            prompt();
            reprompt = false;
        }

        trap_on(c == (int)'q');
        switch (c) {
            case 127:
                tty_y_adjust();
                tty_send(&tcaps.bs);
                fflush(stdout);
                break;
            case '\n':
                reprompt = true;
                tty_send(&tcaps.newline);
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
