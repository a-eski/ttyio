#include <assert.h>
#include <limits.h>
#include <unistd.h>

#include "../ttyio.h"

// ttyio is not intended to be used as a linereader on its own, use bestline. You could use ttyio together with bestline, though. This is just an example.

#define PROMPT "ttyio > "
void prompt()
{
    static_assert(sizeof(PROMPT) - 1 == 8);
    tty_write(PROMPT, sizeof(PROMPT) - 1);
}

void bs()
{
    tty_send(&tcaps.bs);
}

/* repl: some tests and example usage */
int main(void)
{
    tty_init(TTY_NONCANONICAL_MODE);
    tty_send(&tcaps.scr_clr);
    tty_send(&tcaps.cursor_home);

    char c;
    prompt();

    while (read(STDIN_FILENO, &c, 1) > 0) {
        switch (c) {
            case 127:
                bs();
                break;
            case 'q':
                goto end;
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

end:
    tty_send(&tcaps.newline);
    tty_deinit();
    return 0;
}
