# ttyio

A lightweight wrapper library for unibilium.

Useful for writing CLI/REPL/shell applications.

Still in very very early stages of development.

## Features

* Cross-platform
* Easy access to terminal capabilities.
* Simple output interfaces.
* Tracks cursor position, terminal size, and saved cursor position automatically.
* Initialize terminal for canonical or noncanonical input.
* Falls back to ASCII control characters when can't load capabilities.
* Advanced capabilities with multiple fallbacks.
* Compilable with C99 but uses C23 features when available.

## Supported Platforms

* Linux/WSL
  * gcc
  * clang
* Windows
  * MSYS2 (ucrt64, clang64, mingw32, clangarm64)
  * Cygwin
  * w64devkit
* Apple
  * *only tested compilation*
  * Tested compilation for both x84_64 and aarch64.
* BSDs
  * *only tested in VM*
  * FreeBSD
  * OpenBSD
  * NetBSD

### Building

Here is a quick guide on building the main example in different environments.

#### Legend

* If inside '[]', then you have to choose one.
* If inside '{}', then it is optional.
* Builds default to C23. Can specify a standard like 'STD=c99'.
* *If using the clang64 MSYS2 environment, make sure to pass 'CC=clang'.*

#### Overview

* Linux/WSL
  * make [debug/release] {CC=clang} {STD=c99}
* Windows
  * ARM64
    * *tested with zig cross compilation, targeting aarch64-windows-gnu*
    * make zig
  * MSYS2
    * make debug SAN=0 {CC=clang} {STD=c99}
    * make release {CC=clang} {STD=c99}
  * Cygwin
    * make debug SAN=0 {CC=clang} {STD=c99}
    * make release {CC=clang} {STD=c99}
  * w64devkit
    * make debug SAN=0 {CC=clang} {STD=c99}
    * make release LTO=0 {CC=clang} {STD=c99}
* Apple
  * *tested with zig cross compilation*
  * make zig ZIG_TARGET=aarch64_macos
  * make zig ZIG_TARGET=x86_64-macos
* BSDs
  * gmake -f makefile.bsd

## Example

Example is in main.c.
It has assertions but you can disable them.
To run on any size terminal to see the example, use:

``` sh
make DEFINES=-DNDEBUG
# or do a release build
make release
```

## Compile-time Options

You can define 'TTY_USE_NEWLINE_FB' so that ttyio always use "\n" instead of what is defined in the terminfo database.
May be useful to shells or applications writing to output to files.

## Code Definitions

ttyio contains some shorthands/abbreviations in code and in the API:

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
* canonical: recieve input line by line
* noncanonical: recieve input one input at a time

## API

ttyio provides a set of functions to interact with the terminal and terminal capabilities.

### Understanding the APIs

The functions follow C stdlib conventions generally, but there are some adjustments for improving ergnomics.

The functions mostly follow the same pattern:

* There is usually a version that prints your output (formatted or not), and then a version that adds a newline for you.
* There is usually a version which defaults to stdout, a version which accepts a file pointer, and a version which accepts a file descriptor.
* Functions or objects ending in '__' are internal.

For example, there is tty_print which defaults to stdout and prints your output formatted.
Then, tty_fprint has similar semantics but prints the formatted output to the specified file pointer (FILE* restrict file).
Lastly, there is tty_dprint which has similar semantics but prints the formatted output to the specified file descriptor (int fd).
All have a version which handles newline for you as well, tty_println, tty_fprintln, and tty_dprintln.

### Input modes

ttyio provides 2 input modes. It initializes the terminal with these input modes when necessary.
The input modes are optional.

 * Canonical: read line by line, only get the line after user presses enter. a lot of programs work this way.
 * Noncanonical: read character by character. programs who need control over each input need to use this.

### Output Functions

* tty_putc: similar to putchar
* tty_fputc: similar to tty_putc, but accepts a file pointer
* tty_dputc: similar to tty_dputc, but accepts a file descriptor

* tty_write: similar to stdlib's write, but defaults to using stdout
* tty_writeln: same as tty_write, but adds a newline for you
* tty_fwrite: similar to tty_write, but accepts a file descriptor instead of using stdout
* tty_fwriteln: similar to tty_fwrite, but adds a newline for you

* tty_puts: similar to puts, same semantics as puts
* tty_fputs: similar to fputs, same semantics as fputs

* tty_print: similar to printf, same semantics as printf
* tty_println: similar to tty_print, but adds a newline for you
* tty_fprint: similar to tty_print, but accepts a file descriptor. This is inconsistent with other methods like tty_d*, which accept file descriptors, because this is how fprintf works.
* tty_fprintln: similar to tty_fprint, but adds a newline for you.
* tty_dprint(const int fd, const char* restrict fmt, ...)
* tty_dprintln(const int fd, const char* restrict fmt, ...)

* tty_perror: similar to perror, same semantics as perror: adds a red color to the passed in message, then prints ": " and the corresponding errno string.

* tty_send: send the terminal capability to stdout
* tty_dsend: send the terminal capability to the passed in file descriptor
* tty_fsend: send the terminal capability to the passed in file pointer
* tty_send_n: call tty_send n times
* tty_dsend_n: call tty_dsend n times
* tty_fsend_n: call tty_fsend n times

## Unibilium

Props to Neovim maintainers and [unibilium](https://github.com/neovim/unibilium/tree/master).

## Todos

* Make fallback to ASCII control characters configurable.
* If color is greater than tcaps.color_max, try to use a reasonably similar color less than the current max color.
