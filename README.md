# ttyterm

A lightweight wrapper library for unibilium.

Useful for writing CLI/REPL/shell applications.

Still in very very early stages of development.

## Features

* Cross-platform
* Easy access to terminal capabilities.
* Simple output interfaces.
* Tracks cursor position, terminal size, and saved cursor position automatically.
* Falls back to ASCII control characters when can't load capabilities.
* Advanced capabilities with multiple fallbacks.

## Unibilium

Props to Neovim maintainers and [unibilium](https://github.com/neovim/unibilium/tree/master).

## Supported Platforms

* Linux/WSL
  * gcc
  * clang
* Windows
  * MSYS2 (ucrt64, clang64, mingw32, clangarm64)
  * Cygwin
  * w64devkit
* Apple
  * *only tested compilation, don't have a Mac.*
  * Tested compilation for both x84_64 and aarch64.

## Todos

* Have configurable for whether to fall back to ASCII control/escape characters or not.
* Make all output interfaces consistent (i.e. dprint, dwrite, d* is always fd, fprint, f* always takes a file pointer)
* If int color is greater than tcaps.color_max, it tries to use a reasonably similar color, that is less than current max color (tcaps.color_max).

## Example

Example is in main.c.

It has assertions hardcoded from testing, but you can run on any size terminal.

To run on any size terminal to see the example, use:

``` sh
make DEFINES=-DNDEBUG
```

## Code Definitions

The API for ttyterm contains some abbreviations which are fairly common.

Some of these are obvious, but put here just in case they aren't!

* scr: screen
* clr: clear
* eos: end of screen
* eol: end of line
* bol: beginning of line
* bs: backspace
* del: delete
* bg: background, referring to color
* fg: foreground, referring to color
* prev: previous

Functions or objects ending in '__' are internal and not meant to be used outside of ttyterm.
Of course they can be, they are just more likely to change or break your code in future versions if you depend on them.

## Function definitions

### Overview

The functions follow C stdlib conventions generally, but there are some adjustments for improving ergnomics.

The functions mostly follow the same pattern:

* There is usually a version that prints your output (formatted or not), and then a version that adds a newline for you.
* There is usually a version which defaults to stdout, a version which accepts a file pointer, and a version which accepts a file descriptor.

For example, there is term_print which defaults to stdout and prints your output formatted.
Then, term_fprint has similar semantics but prints the formatted output to the specified file pointer (FILE* restrict file).
Lastly, there is term_dprint which has similar semantics but prints the formatted output to the specified file descriptor (int fd).
All have a version which handles newline for you as well, term_println, term_fprintln, and term_dprintln.

### Functions

* term_putc: similar to putchar
* term_fputc: similar to term_putc, but accepts a file pointer
* term_dputc: similar to term_dputc, but accepts a file descriptor

* term_write: similar to stdlib's write, but defaults to using stdout
* term_writeln: same as term_write, but adds a newline for you
* term_fwrite: similar to term_write, but accepts a file descriptor instead of using stdout
* term_fwriteln: similar to term_fwrite, but adds a newline for you

* term_puts: similar to puts, same semantics as puts
* term_fputs: similar to fputs, same semantics as fputs

* term_print: similar to printf, same semantics as printf
* term_println: similar to term_print, but adds a newline for you
* term_fprint: similar to term_print, but accepts a file descriptor. This is inconsistent with other methods like term_d*, which accept file descriptors, because this is how fprintf works.
* term_fprintln: similar to term_fprint, but adds a newline for you.
* term_dprint(const int fd, const char* restrict fmt, ...)
* term_dprintln(const int fd, const char* restrict fmt, ...)

* term_perror: similar to perror, same semantics as perror: adds a red color to the passed in message, then prints ": " and the corresponding errno string.

* term_send: send the terminal capability to stdout
* term_dsend: send the terminal capability to the passed in file descriptor
* term_fsend: send the terminal capability to the passed in file pointer
* term_send_n: call term_send n times
* term_dsend_n: call term_dsend n times
* term_fsend_n: call term_fsend n times
