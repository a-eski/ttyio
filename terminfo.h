#pragma once

#ifndef TERMINFO_GUARD_H_
#define TERMINFO_GUARD_H_

#include "lib/unibilium.h"

unibi_term* terminfo_from_builtin(const char *term, char **termname);

#endif // !TERMINFO_GUARD_H_
