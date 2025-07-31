STD = c2x
STDFLAG= -std=$(STD)
CC ?= gcc
RELEASE ?= 0
DEFINES ?=
# can use to disable santiziers
SAN ?= 1
# can use to disable LTO
LTO ?= 1

main_flags = -Wall -Wextra -Werror -pedantic -pedantic-errors -Wsign-conversion -Wformat=2 -Wshadow -Wvla -fstack-protector-strong -fPIC -fPIE -Wundef -Wbad-function-cast -Wcast-align -Wstrict-prototypes -Wnested-externs -Winline -Wdisabled-optimization -Wunreachable-code -Wchar-subscripts

debug_flags = $(main_flags) -D_FORTIFY_SOURCE=3 -g

release_flags = $(main_flags) -O3 -ffast-math -march=native -DNDEBUG

objects = obj/main.o obj/ttyio.o obj/terminfo.o obj/tcaps.o obj/unibilium.o obj/uninames.o obj/uniutil.o
target = u

ifeq ($(CC), gcc)
	release_flags += -s
endif

ifeq ($(SAN), 1)
	debug_flags += -fsanitize=address,undefined,leak
endif

ifeq ($(LTO), 1)
	release_flags += -flto
endif

ifeq ($(RELEASE), 1)
	CFLAGS ?= $(release_flags)
	cc_with_flags = $(CC) $(STDFLAG) $(CFLAGS) $(DEFINES)
else
	CFLAGS ?= $(debug_flags)
	cc_with_flags = $(CC) $(STDFLAG) $(CFLAGS) $(DEFINES)
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
TTYIO_DEFINES ?= -DTERMINFO='$(TERMINFO)' -DTERMINFO_DIRS='$(TERMINFO_DIRS)'

$(target) : $(objects)
	$(cc_with_flags) -o $(target) $(objects)

obj/%.o: lib/%.c
	$(cc_with_flags) $(TTYIO_DEFINES) -c $< -o $@

obj/%.o: test/%.c
	$(cc_with_flags) $(TTYIO_DEFINES) -c $< -o $@

obj/%.o: %.c
	$(cc_with_flags) -c $< -o $@

# Release build
release:
	make RELEASE=1

# Debug build
debug :
	make -B RELEASE=0

# Cross compilation
ZIG_TARGET ?= aarch64-windows-gnu
zig:
	zig cc -target $(ZIG_TARGET) $(TTYIO_DEFINES) test/main.c ttyio.c terminfo.c tcaps.c lib/unibilium.c lib/uninames.c lib/uniutil.c

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
	rm $(target) $(objects) a.out a.exe a.pdb
