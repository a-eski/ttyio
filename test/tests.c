#include "../ttyio.h"
#include "common.h"

void tty_y_update__(const int printed);
void tty_size_update__(const int printed);

void size_update_test()
{
    term = (Terminal){
        .pos.x = 99,
        .pos.y = 0,
        .size.x = 100,
        .size.y = 10
    };

    tty_size_update__(1);

    assert_trap(term.pos.x == 100);
    assert_trap(term.pos.y == 0);

    term = (Terminal){
        .pos.x = 170,
        .pos.y = 0,
        .size.x = 171,
        .size.y = 10
    };

    tty_size_update__(1);

    assert_trap(term.pos.x == 171);
    assert_trap(term.pos.y == 0);
}

void size_update_eol_test()
{
    term = (Terminal){
        .pos.x = 100,
        .pos.y = 0,
        .size.x = 100,
        .size.y = 10
    };

    tty_size_update__(1);

    assert_trap(term.pos.x == 0);
    assert_trap(term.pos.y == 1);
}

int main(void) {
    size_update_test();
    size_update_eol_test();
}
