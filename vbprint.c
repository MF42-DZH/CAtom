#include "vbprint.h"

#include <stdarg.h>

#ifdef __VERBOSE__
static bool __use_verbose_printing = true;
#else
static bool __use_verbose_printing = false;
#endif

MessageMeta Message;

void vbprintf(FILE *stream, const char *format, ...) {
    va_list args, argcopy;

    va_start(args, format);
    va_copy(argcopy, args);

    vsnprintf(Message.__msg.__message, MAX_STR_LEN, format, args);
    Message.width = NARROW;

    if (__use_verbose_printing) {
        wchar_t wide_format[MAX_STR_LEN];
        swprintf(wide_format, MAX_STR_LEN, L"%s", format);
        vfwprintf(stream, wide_format, argcopy);
    }

    va_end(args);
    va_end(argcopy);
}

void vbwprintf(FILE *stream, const wchar_t *format, ...) {
    va_list args, argcopy;

    va_start(args, format);
    va_copy(argcopy, args);

    vswprintf(Message.__msg.__wessage, MAX_STR_LEN, format, args);
    Message.width = WIDE;

    if (__use_verbose_printing) {
        vfwprintf(stream, format, argcopy);
    }

    va_end(args);
    va_end(argcopy);
}

bool get_verbose_print_status(void) {
    return __use_verbose_printing;
}

void set_verbose_print_status(const bool status) {
    __use_verbose_printing = status;
}
