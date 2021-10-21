#ifndef __TPRINTERR_H__
#define __TPRINTERR_H__

#include <stdbool.h>
#include <wchar.h>

// Colour strings used for printing.
#define PASSING L"\x1b[32;1m%s\x1b[0m"
#define FAILING L"\x1b[31;1m%s\x1b[0m"

// Helper for printing coloured text for testing.
void tprinterr(const char *str, const bool passing);

#endif  // __TPRINTERR_H__
