STD = -std=c2x
CC = clang
DESTDIR ?= /bin
RELEASE ?= 0
DEFINES ?=

main_flags = -Wall -Wextra -Werror -pedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-all -Wundef -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wnested-externs -Winline -Wdisabled-optimization -Wunreachable-code -Wchar-subscripts

debug_flags = $(main_flags) -D_FORTIFY_SOURCE=2
# -fsanitize=address,undefined,leak -g
# -fprofile-arcs -ftest-coverage

test_flags =  $(debug_flags)

release_flags = $(main_flags) -flto -O3 -ffast-math -march=native -DNDEBUG
# -flot=6 -s

fuzz_flags = $(debug_flags) -fsanitize=fuzzer -DNDEBUG

objects = obj/main.o obj/ttyterm.o obj/terminfo.o obj/tcaps.o obj/unibilium.o obj/uninames.o obj/uniutil.o
# objects = obj/ttyterm.o obj/tcaps.o obj/unibilium.o obj/uninames.o obj/uniutil.o
target = u

ifeq ($(CC), gcc)
	release_flags += -s
endif

ifeq ($(RELEASE), 1)
	CFLAGS ?= $(release_flags)
	cc_with_flags = $(CC) $(STD) $(CFLAGS) $(DEFINES)
else
	CFLAGS ?= $(debug_flags)
	cc_with_flags = $(CC) $(STD) $(CFLAGS) $(DEFINES)
endif

ifneq ($(OS),Windows_NT)
  	TERMINFO="$(shell ncursesw6-config --terminfo 2>/dev/null || \
                         ncurses6-config  --terminfo 2>/dev/null || \
                         ncursesw5-config --terminfo 2>/dev/null || \
                         ncurses5-config  --terminfo 2>/dev/null || \
                         echo "/usr/share/terminfo")"
  	TERMINFO_DIRS="$(shell ncursesw6-config --terminfo-dirs 2>/dev/null || \
                         ncurses6-config  --terminfo-dirs 2>/dev/null || \
                         ncursesw5-config --terminfo-dirs 2>/dev/null || \
                         ncurses5-config  --terminfo-dirs 2>/dev/null || \
                         echo "/etc/terminfo:/lib/terminfo:/usr/share/terminfo:/usr/lib/terminfo:/usr/local/share/terminfo:/usr/local/lib/terminfo")"
else
  	TERMINFO_DIRS=""
  	TERMINFO=""
endif
TTYTERM_DEFINES ?= -DTERMINFO='$(TERMINFO)' -DTERMINFO_DIRS='$(TERMINFO_DIRS)'

$(target) : $(objects)
	$(cc_with_flags) -o $(target) $(objects)

obj/%.o: lib/%.c
	$(cc_with_flags) $(TTYTERM_DEFINES) -c $< -o $@

obj/%.o: %.c
	$(cc_with_flags) -c $< -o $@

# Release build
release:
	make RELEASE=1

# Debug build
debug :
	make -B RELEASE=0

# Format the project
clang_format :
	find . -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;
cf :
	make clang_format

# Perform static analysis on the project
.PHONY: scan_build
scan_build:
	scan-build-19 -analyze-headers make

# Clean-up
.PHONY: clean
clean :
	rm $(target) $(objects)
