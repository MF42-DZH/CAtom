#include "tprinterr.h"
#include "whatos.h"

#include <stdio.h>

#ifdef OS_WINDOWS
#include <windows.h>
#include <fileapi.h>

static HANDLE __stderr_handle__ = NULL;
static DWORD __stderr_mode__ = 0;

static bool stderr_isatty(void) {
    HANDLE stderr_handle = GetStdHandle(STD_ERROR_HANDLE);

    if (!stderr_handle) {
        // stderr does not exist.
        return false;
    }

    // Get the file type of stderr.
    DWORD typeof_stderr = GetFileType(stderr_handle);

    if (typeof_stderr == FILE_TYPE_CHAR) {
        // Potentially console?
        DWORD mode = 0;
        if (GetConsoleMode(stderr_handle, &mode)) {
            // Is a console!
            __stderr_handle__ = stderr_handle;
            __stderr_mode__ = mode;

            return true;
        } else {
            // Is a printer or something.
            return false;
        }
    } else {
        // Nope.
        return false;
    }
}

static bool get_console_colour(HANDLE console, WORD *out) {
    CONSOLE_SCREEN_BUFFER_INFO info;

    if (!GetConsoleScreenBufferInfo(console, &info)) {
        return false;
    }

    *out = info.wAttributes;
    return true;
}

void tprinterr(const char* str, const bool passing) {
    // Constant console colour attributes.
    static const WORD DEFAULT_ATTR = 7;
    static const WORD PASSING_ATTR = 10;
    static const WORD FAILING_ATTR = 12;

    // Check if console.
    static bool set = false;
    static bool istty = false;

    if (!set) {
        set = true;
        istty = stderr_isatty();
    }

    // Handle different scenarios depending on if console or not.
    if (istty) {
        if (__stderr_mode__ & ENABLE_VIRTUAL_TERMINAL_PROCESSING) {
            // Use ASCII escapes; console has VT-compat.
            fwprintf(stderr, passing ? PASSING : FAILING, str);
        } else {
            // Get current console colour or default.
            WORD colour = DEFAULT_ATTR;
            if (!get_console_colour(__stderr_handle__, &colour)) {
                fwprintf(stderr, L"*** [WARNING] STDERR attribute fetching failed! ***\n");
            }

            // Legacy handling, using text attributes.
            SetConsoleTextAttribute(__stderr_handle__, passing ? PASSING_ATTR : FAILING_ATTR);
            fwprintf(stderr, L"%s", str);
            SetConsoleTextAttribute(__stderr_handle__, colour);
        }
    } else {
        // No colour.
        fwprintf(stderr, L"%s", str);
    }
}
#else
#include <unistd.h>

void tprinterr(const char* str, const bool passing) {
    if (isatty(fileno(stderr))) {
        // Use ASCII escapes.
        fwprintf(stderr, passing ? PASSING : FAILING, str);
    } else {
        // No colour.
        fwprintf(stderr, L"%s", str);
    }
}
#endif
