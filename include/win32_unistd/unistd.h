// FLEX insists on including unistd.h and calling isatty() even on Windows.
// This hack fixes this.

#pragma once

#define isatty(fd) 0
