/**
 * @file      catom.c
 * @author    0xFC963F18DC21 (crashmacompilers@gmail.com)
 * @brief     CAtom: A simple C test suite, inspired by JUnit.
 * @version   1.9.0
 * @date      2021-10-21
 *
 * @copyright 0xFC963F18DC21 (c) 2021
 *
 * This is CAtom. A simple, portable test suite and runner inspired by JUnit. It is used to perform unit
 * and (limited) integration testing on simple functions and pieces of data.
 *
 * As mentioned in testsuite.h, add the __VERBOSE__ flag when compiling this test suite to use verbose printing by default.
 *
 * See testsuite.h for more information. There are no comments here. This is the wild west of this test suite.
 */

#include "catom.h"

#include "libs/arrcmp.h"
#include "libs/genarrays.h"
#include "libs/hashing.h"
#include "libs/memalloc.h"
#include "libs/salloc.h"
#include "libs/tprinterr.h"
#include "libs/vbprint.h"
#include "libs/whatos.h"

#include <inttypes.h>
#include <setjmp.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#define SEP "--------------------------------------------------------------------------------"

// Helper for assertion failure message printer.
static char __last_assert_caller_file[MAX_STR_LEN] = { '\0' };
static char __last_assert_caller[MAX_STR_LEN] = { '\0' };
static char __last_assert_used[MAX_STR_LEN] = { '\0' };
static int __last_line_of_assert_caller = 0;

// Printing utilities.
static void print_obj_hashes(const char *format, const void *obj1, const void *obj2, size_t size) {
    if (get_verbose_print_status()) {
        uint64_t oh1 = obj_hash(obj1, size);
        uint64_t oh2 = obj_hash(obj2, size);
        vbprintf(stderr, format, oh1, oh2);
    }
}

// Internal assertion function.
static jmp_buf env;
static size_t failures = 0;
static bool in_benchmark = false;
static bool in_timed = false;

void reset_failures(void) {
    failures = 0u;
}

static void fail_test(void) {
    ++failures;
    longjmp(env, 1);
}

// This is defined separately at the bottom of the file.
static void terminate_timed_test(void);

#define __test_assert__(cond) if (!(cond)) {\
    if (Message.width == NARROW) {\
        fwprintf(stderr, L"\n[%s] Assertion Failed. %s failed in %s at line %d:\n%s",\
                __last_assert_caller_file,\
                __last_assert_used,\
                __last_assert_caller,\
                __last_line_of_assert_caller,\
                Message.__msg.__message\
        );\
    } else {\
        fwprintf(stderr, L"\n[%s] Assertion Failed. %s failed in %s at line %d:\n%ls",\
                __last_assert_caller_file,\
                __last_assert_used,\
                __last_assert_caller,\
                __last_line_of_assert_caller,\
                Message.__msg.__wessage\
        );\
    }\
    if (in_benchmark) {\
        fwprintf(stderr, L"\n*** [WARNING] Do not use asserts inside a benchmark or timed test! ***\n");\
    } else if (in_timed) {\
        terminate_timed_test();\
    } else {\
        fail_test();\
    }\
}

// Generalised add-one-to-int-array.
void add_one(size_t *nums, const size_t ns[], const size_t where, const size_t max) {
    if (where < max && ++nums[where] >= ns[where]) {
        nums[where] = 0;
        add_one(nums, ns, where - 1, max);
    }
}

// Comparison of arrays function.
void compare_arrays(const void *arr1, const void *arr2, const bool arr1isptp, const bool arr2isptp, const size_t size, const size_t argn, const size_t ns[], const MemoryValidator validator) {
    size_t *current = (size_t *) alloca(argn * sizeof(size_t));
    if (!current) {
        fwprintf(stderr, L"*** [WARNING] Comparison of arrays failed to allocate enough memory. ***\n");
    }

    memset(current, 0, argn * sizeof(size_t));

    size_t total_items = ns[0];
    for (size_t i = 1; i < argn; ++i) {
        total_items *= ns[i];
    }

    for (size_t i = 0; i < total_items; ++i) {
        const void *i1 = get(arr1, arr1isptp, size, ns, argn, current);
        const void *i2 = get(arr2, arr2isptp, size, ns, argn, current);

        if (!validator(i1, i2, size)) {
            __test_assert__(validator(i1, i2, size));
        }

        add_one(current, ns, argn - 1, argn);
    }
}

// Comparison of arrays function.
void compare_arrays_some(const void *arr1, const void *arr2, const bool arr1isptp, const bool arr2isptp, const size_t size, const size_t argn, const size_t ns[], const MemoryValidator validator) {
    size_t *current = (size_t *) alloca(argn * sizeof(size_t));
    if (!current) {
        fwprintf(stderr, L"*** [WARNING] Comparison of arrays failed to allocate enough memory. ***\n");
    }

    memset(current, 0, argn * sizeof(size_t));

    size_t total_items = ns[0];
    for (size_t i = 1; i < argn; ++i) {
        total_items *= ns[i];
    }

    size_t matches = 0;

    for (size_t i = 0; i < total_items; ++i) {
        const void *i1 = get(arr1, arr1isptp, size, ns, argn, current);
        const void *i2 = get(arr2, arr2isptp, size, ns, argn, current);

        if (validator(i1, i2, size)) {
            ++matches;
        }

        add_one(current, ns, argn - 1, argn);
    }

    __test_assert__(matches > 0);
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

void use_verbose_print(const bool should_use) {
    set_verbose_print_status(should_use);
}

static void __run_test(Test *test) {
    fwprintf(stderr, get_verbose_print_status() ? L"Running test \"%s\":\n\n" : L"Running test \"%s\":\n", test->name);

    clock_t time = clock();

    if (setjmp(env) == 0) {
        test->test();
        tprinterr("\nTest passed. ", true);
        test->passed = true;
    } else {
        tprinterr("\nTest failed. ", false);
        test->passed = false;
    }

    time = clock() - time;

    fwprintf(stderr, L"\"%s\" terminated in %f seconds.\n",
        test->name, (double) time / CLOCKS_PER_SEC
    );

    testfunc_freeall();
}

static clock_t __run_benchmark(const Benchmark *benchmark, const size_t warmup, const size_t times) {
    in_benchmark = true;

    fwprintf(stderr, L"Running benchmark \"%s\":\n\n", benchmark->name);

    clock_t total_time = 0;
    clock_t with_wm = 0;

    for (size_t i = 0; i < warmup + times; ++i) {
        if (i < warmup) {
            fwprintf(stderr, L"Running warmup iteration %zu / %zu. ", i + 1, warmup);
        } else {
            fwprintf(stderr, L"Running benchmark iteration %zu / %zu. ", i - warmup + 1, times);
        }

        clock_t time_taken = clock();
        benchmark->benchmark();
        time_taken = clock() - time_taken;

        if (i >= warmup) {
            total_time += time_taken;
            fwprintf(stderr, L"Finished benchmark iteration %zu / %zu in %f seconds.\n", i - warmup + 1, times, (double) time_taken / CLOCKS_PER_SEC);
        } else {
            fwprintf(stderr, L"Finished warmup iteration %zu / %zu in %f seconds.\n", i + 1, warmup, (double) time_taken / CLOCKS_PER_SEC);
        }

        with_wm += time_taken;
    }

    in_benchmark = false;

    fwprintf(stderr, L"\nBenchmark complete.\n\"%s\" finished %zu iterations (and %zu warmup iterations) in %f seconds (%f seconds with warmup).\nIt took %f seconds on average to run (%f seconds average with warmup).\n",
        benchmark->name,
        times,
        warmup,
        (double) total_time / CLOCKS_PER_SEC,
        (double) with_wm / CLOCKS_PER_SEC,
        (double) total_time / (times * CLOCKS_PER_SEC),
        (double) with_wm / ((times + warmup) * CLOCKS_PER_SEC)
    );

    return with_wm;
}

void __run_tests(Test tests[], const size_t n) {
    failures = 0;

    fwprintf(stderr, L"Running %zu test%s.\n\n", n, n != 1 ? "s" : "");

    clock_t time;
    time = clock();

    for (size_t i = 0; i < n; ++i) {
        fwprintf(stderr, L"%s\n[%zu / %zu] ", SEP, i + 1u, n);
        __run_test(tests + i);
        fwprintf(stderr, L"%s\n\n", SEP);
    }

    time = clock() - time;

    fwprintf(stderr, L"Tests completed in %f seconds with %zu / %zu passed (%zu failed).\n\n", (double) time / CLOCKS_PER_SEC, n - failures, n, failures);
}

void __run_benchmarks(const Benchmark benchmarks[], const size_t n, const size_t warmup, const size_t times) {
    fwprintf(stderr, L"Running %zu benchmark%s.\n\n", n, n != 1 ? "s" : "");

    clock_t total = 0;

    for (size_t i = 0; i < n; ++i) {
        fwprintf(stderr, L"%s\n[%zu / %zu] ", SEP, i + 1u, n);
        total += __run_benchmark(benchmarks + i, warmup, times);
        fwprintf(stderr, L"%s\n\n", SEP);
    }

    fwprintf(stderr, L"Benchmarks completed in %f seconds.\n\n", (double) total / CLOCKS_PER_SEC);
}

size_t count_failures(const Test tests[], const size_t n) {
    size_t fails = 0;
    for (size_t i = 0; i < n; ++i) {
        if (!tests[i].passed) {
            ++fails;
        }
    }

    return fails;
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

void __assert_wide_string_equals(const wchar_t *str1, const wchar_t *str2) {
    vbwprintf(stderr, L"WIDE STRING EQ: \"%ls\" == \"%ls\"?\n", str1, str2);
    __test_assert__(wcscmp(str1, str2) == 0);
}

void __assert_wide_string_not_equals(const wchar_t *str1, const wchar_t *str2) {
    vbwprintf(stderr, L"WIDE STRING NEQ: \"%ls\" != \"%ls\"?\n", str1, str2);
    __test_assert__(wcscmp(str1, str2) != 0);
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
    compare_arrays_some(arr1, arr2, arr1isptp, arr2isptp, size, argn, ns, memory_is_not_equals);
}

void __assert_not_null(const void *ptr) {
    vbprintf(stderr, "PTR not NULL: %zu != %zu?\n", (size_t) ptr, (size_t) NULL);
    __test_assert__(ptr);
}

void __assert_null(const void *ptr) {
    vbprintf(stderr, "PTR is NULL: %zu == %zu?\n", (size_t) ptr, (size_t) NULL);
    __test_assert__(!ptr);
}

void __assert_time_limit(const TestFunction func, double time_limit) {
    clock_t time = clock();
    func();
    time = clock() - time;

    __test_assert__((double) time / CLOCKS_PER_SEC <= (double) time_limit);
}

// Implementations of async time limit assertion.
#ifdef OS_WINDOWS
#include <windows.h>

static TestFunction __running_testfunc__;
static HANDLE test_thread = NULL;
static HANDLE test_semaphore = NULL;

static DWORD WINAPI __testfunc_runner__(void *param __attribute__((unused))) {
    __running_testfunc__();
    ReleaseSemaphore(test_semaphore, 1, NULL);

    return 0;
}

static void terminate_timed_test(void) {
    TerminateThread(test_thread, -1);
}

void __assert_time_limit_async(const TestFunction func, double time_limit) {
    vbprintf(stderr, "FUNCTION EXITS IN %lf SECONDS?\n", time_limit);

    // Initialise our variables and semaphores.
    __running_testfunc__ = func;
    test_semaphore = CreateSemaphore(NULL, 0, 1, NULL);

    // We're in a timed test here.
    in_timed = true;

    // Create our new thread and wait for the semaphore.
    test_thread = CreateThread(NULL, 0, __testfunc_runner__, NULL, 0, NULL);
    DWORD test_result = WaitForSingleObject(test_semaphore, time_limit * 1000);

    // Terminate thread and free up resources.
    TerminateThread(test_thread, -1);

    CloseHandle(test_semaphore);
    CloseHandle(test_thread);

    // We're no longer in a timed test.
    in_timed = false;

    // Check our result.
    switch (test_result) {
        case WAIT_OBJECT_0:
            // All good.
            break;
        case WAIT_TIMEOUT:
            // Function failed to exit.
            __test_assert__(false);
            break;
        case WAIT_FAILED:
            // Waiting on semaphore failed.
            fwprintf(stderr, L"*** Failed to wait on semaphore! ***\n");
            __test_assert__(false);
            break;
        default:
            // How did you get here?
            fwprintf(stderr, L"*** Abnormal wait return: %d. ***\n", test_result);
            __test_assert__(false);
            break;
    }
}
#else
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

static pid_t __child__ = -1;

static const struct timeval ZERO = { 0, 0 };

static const struct itimerval IT_ZERO = { { 0, 0 }, { 0, 0 } };

static __attribute__((noreturn)) void __testfunc_runner__(const TestFunction func) {
    func();
    exit(0);
    __builtin_unreachable();
}

static void __alarm_handler__(int param __attribute__((unused))) {
    if (__child__ >= 0) {
        kill(__child__, SIGKILL);
    }
}

static void terminate_timed_test(void) {
    exit(-1);
}

void __assert_time_limit_async(const TestFunction func, double time_limit) {
    vbprintf(stderr, "FUNCTION EXITS IN %lf SECONDS?\n", time_limit);

    // Create timer initialiser.
    int child_status;
    uint32_t seconds = (uint32_t) time_limit;

    struct itimerval timer_init_val = {
        .it_interval = ZERO,
        .it_value = {
            .tv_sec = seconds,
            .tv_usec = ((time_limit - seconds) * 1000000)
        }
    };

    // Fork process here, where child runs the test function, and parent monitors it.
    switch ((__child__ = fork())) {
        case -1:
            fwprintf(stderr, L"*** Failed to create child process! ***\n");
            __test_assert__(false);
            break;
        case 0:
            __testfunc_runner__(func);
            break;
        default:
            {
                // Set alarm handler signal to kill child.
                void (*prev_handler)(int) = signal(SIGALRM, __alarm_handler__);

                // We're in a timed test here.
                in_timed = true;

                // Set timer, run function, wait for child. If timer expires, child is killed.
                setitimer(ITIMER_REAL, &timer_init_val, NULL);
                waitpid(-1, &child_status, 0);
                setitimer(ITIMER_REAL, &IT_ZERO, NULL);

                // Reset the child value.
                __child__ = -1;

                // Reset the signal handler to the default.
                signal(SIGALRM, prev_handler);

                // We're no longer in a timed test.
                in_timed = false;

                // Check exit code of child process.
                __test_assert__(WIFEXITED(child_status));
            }
            break;
    }
}
#endif
