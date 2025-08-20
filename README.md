# ttyio

A low level, cross-platform, terminal output library for writing CLIs, REPLs, and shells.

ttyio is a lightweight wrapper library for unibilium.

## Features

* Cross-platform
* Easy access to terminal capabilities.
* Simple output interfaces.
* Tracks cursor position, terminal size, and saved cursor position automatically.
* Can initialize the terminal for canonical or noncanonical input.
* Falls back to ASCII control characters when can't load capabilities from terminfo.
* Advanced capabilities with multiple fallbacks.
* Compilable with C99, but uses C23 features when available.

## Why?

I had been working on a shell for just under a year. I didn't want to use ncurses, termbox2, notcurses, pdcurses, etc., some of the most common suggestions for handling terminal output in C. Why? They just seemed like overkill for my use case. It's a shell, it's a REPL, not a TUI or some other complex interface. Let modern terminals handle the scrollback and that other stuff, they already do. I don't need to track all of that in memory and rerender all of the time, it seemed wasteful.

So not wanting to take on those dependencies, at first I made a custom implementation using ASCII control characters. Its not great, but it worked on most 256 color terminals. It has some issues, including not being portable (not reliably working on terminals less that 256 colors or older terminals), but you don't need to track scrollback or the exact position on the screen. It only tracked relative position. It had some bugs with restore cursor when the screen scrolled down (because it wasn't updating the saved cursor position), but besides that it worked for multiline and all of those kinds of inputs tracking relative position. However, I wanted something more portable and maintainable.

After experiencing some of the issues with the custom ASCII implementation over the past almost year, I went looking for another solution. I tried GNU readline, bestline, linenoise, GNU termcaps, ncurses, termbox2, etc. GNU readline is the default choice in cases like this, although if you want something out of the box I really recommend bestline. GNU readline (and other line readers) didn’t support the kind of autocompletions I wanted, and would have been a lot of effort to add those autocompletions in a fork. ncurses is great for TUI's, but I didn't want to deal with the overhead from it or the idioms it forces. Termbox2 isn't purpose built for shells/REPLs, but it's awesome for TUIs. GNU termcaps would work fine, but you do need to do a lot to get it working correctly portably, and it is obsolete. GNU now recommends using lib/tinfo from ncurses instead of GNU termcap. Linenoise had some of the same issues as readline.

Then, I found unibilium. I use neovim, and was searching through the repo, wondering how they handled terminal output, and I noticed unibilium. I thought that neovim used ncurses or lib/tinfo (and maybe they did in the past), but it seems they started maintaining a fork of unibilium for their own purposes and using that. Unibilium was a dream compared to GNU termcap, so I started experimenting with it. Neovim unibilium Fork: https://github.com/neovim/unibilium/tree/master

After a while of messing around with unibilium, I decided to incorporate it into my shell. However, I didn't want to couple output everywhere in the terminal to unibilium, so I ended up writing this wrapper for unibilium called ttyio. It isn't a line reader, but a way to portably send output to stdin while respecting terminfo on different platforms and terminals. 

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
May be useful to shells or applications writing to output to files. If you aren't redirecting stdin to write to files, no need to worry about it.

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
