/* Copyright ttyterm (C) by Alex Eski 2025 */
/* Licensed under GPLv3, see LICENSE for more information. */
/* ttyplatform.c: platform layer implementation for the ttyterm library */

#if !defined(__unix__) && !defined(__APPLE__) && !defined(__MINGW32__) && !defined(__MINGW64) && !defined(__CYGWIN__)

# include <windows.h>

int win_write([[maybe_unused]] const int fd, char* msg, const size_t len)
{
    DWORD written;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), msg, len, &written, NULL);
    return written;
}

#endif /* if !defined(__unix__) && !defined(__APPLE__) && !defined(__MINGW32__) && !defined(__MINGW64) && !defined(__CYGWIN__) */
