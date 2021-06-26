/**
 * @file      testsuite.c
 * @author    0xFC963F18DC21 (crashmacompilers@gmail.com)
 * @brief     CUnit: A simple C test suite, inspired by JUnit.
 * @version   1.4.1
 * @date      2021-06-26
 *
 * @copyright 0xFC963F18DC21 (c) 2021
 *
 * This is CUnit. A simple, portable test suite and runner inspired by JUnit. It is used to perform unit
 * and (limited) integration testing on simple functions and pieces of data.
 *
 * As mentioned in testsuite.h, add the __VERBOSE__ flag when compiling this test suite to use verbose printing.
 *
 * See testsuite.h for more information. There are no comments here. This is the wild west of this test suite.
 */

#include "testsuite.h"

#include <inttypes.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_STR_LEN 1024
#define SEP         "--------------------------------------------------------------------------------"

// Helper for printing coloured text for testing.
static void tprinterr(const char* str, const bool passing);

static const char *PASSING = "\x1b[32;1m%s\x1b[0m";
static const char *FAILING = "\x1b[31;1m%s\x1b[0m";

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#include <fileapi.h>

static HANDLE __stderr_handle__ = NULL;
static DWORD __stderr_mode__ = 0;

static bool stderr_isatty(void) {
    HANDLE stderr_handle = GetStdHandle(STD_ERROR_HANDLE);
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

static void tprinterr(const char* str, const bool passing) {
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
            fprintf(stderr, passing ? PASSING : FAILING, str);
        } else {
            // Get current console colour or default.
            WORD colour = DEFAULT_ATTR;
            if (!get_console_colour(__stderr_handle__, &colour)) {
                fprintf(stderr, "*** [WARNING] STDERR attribute fetching failed! ***\n");
            }

            // Legacy handling, using text attributes.
            SetConsoleTextAttribute(__stderr_handle__, passing ? PASSING_ATTR : FAILING_ATTR);
            fprintf(stderr, "%s", str);
            SetConsoleTextAttribute(__stderr_handle__, colour);
        }
    } else {
        // No colour.
        fprintf(stderr, "%s", str);
    }
}
#else
#include <unistd.h>

static void tprinterr(const char* str, const bool passing) {
    if (isatty(fileno(stderr))) {
        // Use ASCII escapes.
        fprintf(stderr, passing ? PASSING : FAILING, str);
    } else {
        // No colour.
        fprintf(stderr, "%s", str);
    }
}
#endif

// Helper for assertion failure message printer.
static char __last_assert_caller_file[MAX_STR_LEN] = { '\0' };
static char __message[MAX_STR_LEN] = { '\0' };
static char __last_assert_caller[MAX_STR_LEN] = { '\0' };
static char __last_assert_used[MAX_STR_LEN] = { '\0' };
static int __last_line_of_assert_caller = 0;

void vbprintf(FILE *stream, const char *format, ...) {
    va_list args, argcopy;

    va_start(args, format);
    va_copy(argcopy, args);

    vsnprintf(__message, MAX_STR_LEN, format, args);

#ifdef __VERBOSE__
    vfprintf(stream, format, argcopy);
#endif

    va_end(args);
    va_end(argcopy);
}

// Printing utilities.
#define HASH_CONSTANT 524287u

static uint64_t obj_hash(const void *obj, const size_t total_length) {
    uint64_t result = 0u;

    if (obj) {
        result = 1u;
        uint64_t multiplier = 1u;

        for (size_t i = 0; i < total_length; ++i) {
            uint8_t cur = ((uint8_t *) obj)[i];
            result += ((uint64_t) cur) * multiplier;

            multiplier *= HASH_CONSTANT;
        }
    }

    return result;
}

static void print_obj_hashes(const char *format, const void *obj1, const void *obj2, size_t size) {
#ifdef __VERBOSE__
    uint64_t oh1 = obj_hash(obj1, size);
    uint64_t oh2 = obj_hash(obj2, size);
    vbprintf(stderr, format, oh1, oh2);
#endif
}

// Internal assertion function.
static jmp_buf env;
static size_t failures = 0;
static bool in_benchmark = false;

static void fail_test(void) {
    ++failures;
    longjmp(env, 1);
}

#define __test_assert__(cond) if (!(cond)) {\
    fprintf(stderr, "\n[%s] Assertion Failed. %s failed in %s at line %d:\n%s",\
            __last_assert_caller_file,\
            __last_assert_used,\
            __last_assert_caller,\
            __last_line_of_assert_caller,\
            __message\
    );\
    if (!in_benchmark) {\
        fail_test();\
    } else {\
        fprintf(stderr, "\n*** [WARNING] Do not use asserts inside a benchmark! ***\n");\
    }\
}

// Comparison of arrays function.
static const void *get_from_flat(const void *arr, const size_t size, const size_t ns[], const size_t argn, const size_t *where) {
    // Assuming C arrays are in row-major order...
    size_t offset = 0;
    size_t mult = 1;
    for (size_t n = argn - 1; n < argn; --n) {
        offset += where[n] * mult;
        mult *= ns[n];
    }

    return (uint8_t *) arr + (offset * size);
}

static const void *get_from_ptp(const void *arr, const size_t size, const size_t argn, const size_t *where) {
    if (argn == 1) {
        return (uint8_t *) arr + (where[0] * size);
    } else {
        return get_from_ptp(*((uint8_t **) arr + where[0]), size, argn - 1, where + 1);
    }
}

static const void *get(const void *arr, const bool isptp, const size_t size, const size_t ns[], const size_t argn, const size_t *where) {
    if (isptp) {
        return get_from_ptp(arr, size, argn, where);
    } else {
        return get_from_flat(arr, size, ns, argn, where);
    }
}

typedef bool (*MemoryValidator)(const void *, const void *, const size_t);

static bool memory_is_equals(const void *m1, const void *m2, const size_t n) {
    return memcmp(m1, m2, n) == 0;
}

static bool memory_is_not_equals(const void *m1, const void *m2, const size_t n) {
    return !memory_is_equals(m1, m2, n);
}

static void add_one(size_t *nums, const size_t ns[], const size_t where, const size_t max) {
    if (where < max && ++nums[where] >= ns[where]) {
        nums[where] = 0;
        add_one(nums, ns, where - 1, max);
    }
}

static void compare_arrays(const void *arr1, const void *arr2, const bool arr1isptp, const bool arr2isptp, const size_t size, const size_t argn, const size_t ns[], const MemoryValidator validator) {
    size_t *current = (size_t *) calloc(argn, sizeof(size_t));
    if (!current) {
        fprintf(stderr, "*** [WARNING] Comparison of arrays failed to allocate enough memory. ***\n");
    }

    size_t total_items = ns[0];
    for (size_t i = 1; i < argn; ++i) {
        total_items *= ns[i];
    }

    for (size_t i = 0; i < total_items; ++i) {
        const void *i1 = get(arr1, arr1isptp, size, ns, argn, current);
        const void *i2 = get(arr2, arr2isptp, size, ns, argn, current);

        if (!validator(i1, i2, size)) {
            free(current);
            __test_assert__(validator(i1, i2, size));
        }

        add_one(current, ns, argn - 1, argn);
    }

    free(current);
}

// Test runner utilities.
void __set_last_file(const char *filename) {
    strncpy(__last_assert_caller_file, filename, MAX_STR_LEN);
}

void __set_last_caller(const char *caller) {
    strncpy(__last_assert_caller, caller, MAX_STR_LEN);
}

void __set_last_assert(const char *assert) {
    strncpy(__last_assert_used, assert, MAX_STR_LEN);
}

void __set_last_line(const int line) {
    __last_line_of_assert_caller = line;
}

void __run_test(const Test test) {
    fprintf(stderr, "Running test \"%s\":\n", test.name);

    clock_t time = clock();

    if (setjmp(env) == 0) {
        test.test();
        tprinterr("\nTest passed. ", true);
    } else {
        tprinterr("\nTest failed. ", false);
    }

    time = clock() - time;

    fprintf(stderr, "\"%s\" terminated in %f seconds.\n",
        test.name, (double) time / CLOCKS_PER_SEC
    );
}

clock_t __run_benchmark(const Benchmark benchmark, const size_t warmup, const size_t times) {
    in_benchmark = true;

    fprintf(stderr, "Running benchmark \"%s\":\n\n", benchmark.name);

    clock_t total_time = 0;
    clock_t with_wm = 0;

    for (size_t i = 0; i < warmup + times; ++i) {
        if (i < warmup) {
            fprintf(stderr, "Running warmup iteration %zu / %zu. ", i + 1, warmup);
        } else {
            fprintf(stderr, "Running benchmark iteration %zu / %zu. ", i - warmup + 1, times);
        }

        clock_t time_taken = clock();
        benchmark.benchmark();
        time_taken = clock() - time_taken;

        if (i >= warmup) {
            total_time += time_taken;
            fprintf(stderr, "Finished benchmark iteration %zu / %zu in %f seconds.\n", i - warmup + 1, times, (double) time_taken / CLOCKS_PER_SEC);
        } else {
            fprintf(stderr, "Finished warmup iteration %zu / %zu in %f seconds.\n", i + 1, warmup, (double) time_taken / CLOCKS_PER_SEC);
        }

        with_wm += time_taken;
    }

    in_benchmark = false;

    fprintf(stderr, "\nBenchmark complete.\n\"%s\" finished %zu iterations (and %zu warmup iterations) in %f seconds (%f seconds with warmup).\nIt took %f seconds on average to run (%f seconds average with warmup).\n",
        benchmark.name,
        times,
        warmup,
        (double) total_time / CLOCKS_PER_SEC,
        (double) with_wm / CLOCKS_PER_SEC,
        (double) total_time / (times * CLOCKS_PER_SEC),
        (double) with_wm / ((times + warmup) * CLOCKS_PER_SEC)
    );

    return with_wm;
}

void __run_tests(const Test tests[], const size_t n) {
    fprintf(stderr, "Running %zu test%s.\n\n", n, n != 1 ? "s" : "");

    clock_t time;
    time = clock();

    for (size_t i = 0; i < n; ++i) {
        fprintf(stderr, "%s\n[%zu / %zu] ", SEP, i + 1u, n);
        __run_test(tests[i]);
        fprintf(stderr, "%s\n\n", SEP);
    }

    time = clock() - time;

    fprintf(stderr, "Tests completed in %f seconds with %zu / %zu passed (%zu failed).\n\n", (double) time / CLOCKS_PER_SEC, n - failures, n, failures);
}

void __run_benchmarks(const Benchmark benchmarks[], const size_t n, const size_t warmup, const size_t times) {
    fprintf(stderr, "Running %zu benchmark%s.\n\n", n, n != 1 ? "s" : "");

    clock_t total = 0;

    for (size_t i = 0; i < n; ++i) {
        fprintf(stderr, "%s\n[%zu / %zu] ", SEP, i + 1u, n);
        total += __run_benchmark(benchmarks[i], warmup, times);
        fprintf(stderr, "%s\n\n", SEP);
    }

    fprintf(stderr, "Benchmarks completed in %f seconds.\n\n", (double) total / CLOCKS_PER_SEC);
}

// Checker functions for the test suite.
void __assert_true(const bool condition) {
    vbprintf(stderr, "BOOL is TRUE: %d?\n", condition);
    __test_assert__(condition);
}

void __assert_false(const bool condition) {
    vbprintf(stderr, "BOOL is FALSE: %d?\n", condition);
    __test_assert__(!condition);
}

void __assert_uint_equals(uint64_t a, uint64_t b) {
    vbprintf(stderr, "UINT EQ: %lu == %lu?\n", a, b);
    __test_assert__(a == b);
}

void __assert_uint_not_equals(uint64_t a, uint64_t b) {
    vbprintf(stderr, "UINT NEQ: %lu != %lu?\n", a, b);
    __test_assert__(a != b);
}

void __assert_sint_equals(int64_t a, int64_t b) {
    vbprintf(stderr, "SINT EQ: %ld == %ld?\n", a, b);
    __test_assert__(a == b);
}

void __assert_sint_not_equals(int64_t a, int64_t b) {
    vbprintf(stderr, "SINT NEQ: %ld != %ld?\n", a, b);
    __test_assert__(a != b);
}

void __assert_float_equals(float a, float b, float epsilon) {
    vbprintf(stderr, "FLOAT EQ: %f == %f (eps = %f)?\n", a, b, epsilon);
    float d = a - b;
    __test_assert__(d > -epsilon && d < epsilon);
}

void __assert_float_not_equals(float a, float b, float epsilon) {
    vbprintf(stderr, "FLOAT NEQ: %f != %f (eps = %f)?\n", a, b, epsilon);
    float d = a - b;
    __test_assert__(d <= -epsilon || d >= epsilon);
}

void __assert_double_equals(double a, double b, double epsilon) {
    vbprintf(stderr, "DOUBLE EQ: %f == %f (eps = %f)?\n", a, b, epsilon);
    double d = a - b;
    __test_assert__(d > -epsilon && d < epsilon);
}

void __assert_double_not_equals(double a, double b, double epsilon) {
    vbprintf(stderr, "DOUBLE NEQ: %f != %f (eps = %f)?\n", a, b, epsilon);
    double d = a - b;
    __test_assert__(d <= -epsilon || d >= epsilon);
}

void __assert_string_equals(const char *str1, const char *str2) {
    vbprintf(stderr, "STRING EQ: \"%s\" == \"%s\"?\n", str1, str2);
    __test_assert__(strcmp(str1, str2) == 0);
}

void __assert_string_not_equals(const char *str1, const char *str2) {
    vbprintf(stderr, "STRING NEQ: \"%s\" != \"%s\"?\n", str1, str2);
    __test_assert__(strcmp(str1, str2) != 0);
}

void __assert_equals(const void *obj1, const void *obj2, const size_t size) {
    print_obj_hashes("OBJ EQ: %"PRIx64" == %"PRIx64"?\n", obj1, obj2, size);
    __test_assert__(memcmp(obj1, obj2, size) == 0);
}

void __assert_not_equals(const void *obj1, const void *obj2, const size_t size) {
    print_obj_hashes("OBJ NEQ: %"PRIx64" == %"PRIx64"?\n", obj1, obj2, size);
    __test_assert__(memcmp(obj1, obj2, size) != 0);
}

void __assert_array_equals(const void *arr1, const void *arr2, const size_t n, const size_t size) {
    print_obj_hashes("ARR EQ: %"PRIx64" == %"PRIx64"?\n", arr1, arr2, n * size);

    for (size_t i = 0; i < n; ++i) {
        __test_assert__(memcmp((uint8_t *) arr1 + (i * size), (uint8_t *) arr2 + (i * size), size) == 0);
    }
}

void __assert_array_not_equals(const void *arr1, const void *arr2, const size_t n, const size_t size) {
    print_obj_hashes("ARR NEQ: %"PRIx64" == %"PRIx64"?\n", arr1, arr2, n * size);

    for (size_t i = 0; i < n * size; ++i) {
        if (memcmp((uint8_t *) arr1 + i, (uint8_t *) arr2 + i, size) != 0) {
            return;
        }
    }

    __test_assert__(false);
}

void __assert_deep_array_equals(const void *arr1, const void *arr2, const bool arr1isptp, const bool arr2isptp, const size_t size, const size_t argn, const size_t ns[]) {
    vbprintf(stderr, "DEEP ARR EQ: @%zx and @%zx?\n", (size_t) arr1, (size_t) arr2);
    compare_arrays(arr1, arr2, arr1isptp, arr2isptp, size, argn, ns, memory_is_equals);
}

void __assert_deep_array_not_equals(const void *arr1, const void *arr2, const bool arr1isptp, const bool arr2isptp, const size_t size, const size_t argn, const size_t ns[]) {
    vbprintf(stderr, "DEEP ARR NEQ: @%zx and @%zx?\n", (size_t) arr1, (size_t) arr2);
    compare_arrays(arr1, arr2, arr1isptp, arr2isptp, size, argn, ns, memory_is_not_equals);
}

void __assert_not_null(const void *ptr) {
    vbprintf(stderr, "PTR not NULL: %zu != %zu?\n", (size_t) ptr, (size_t) NULL);
    __test_assert__(ptr);
}

void __assert_null(const void *ptr) {
    vbprintf(stderr, "PTR is NULL: %zu == %zu?\n", (size_t) ptr, (size_t) NULL);
    __test_assert__(!ptr);
}
