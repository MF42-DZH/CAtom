/**
 * @file      catom.h
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
 * Like JUnit, all output goes to stderr. Also like JUnit, all tests are guaranteed to run, provided that
 * the system does not run out of memory when allocating strings to print verbose output.
 *
 * It also provides rudimentary benchmarking capabilities, using clock_t in time.h.
 *
 * Compile-time options:
 * - If you want verbose output by default, add -D__VERBOSE__ to your compile flags when compiling this test suite.
 *
 * User guide:
 * - Create a series of test functions (preferably prefixed with test_), that conform to the TestFunction
 *   function pointer type.
 * - Create a static array of Test structs, which hold a pointer to the test to run, and a user-friendly
 *   identifier that shows up in logs.
 * - Use the run_tests macro to run all the tests in the array.
 *
 * The library then iterates through the array, running each test using the run_test function. If all assertions
 * pass, the test exits normally without a failure. If an assertion fails, control long-jumps back to run_test
 * and a failure is counted. A helpful message is also shown in stderr showing which function in which test file
 * has a failed assertion (and in what line the call occurs).
 *
 * The many assertion functions to use are listed below, with documentation comments to help you in using them.
 *
 * Notes:
 * - setjmp / longjmp may not be the most stable method of doing this. I am not responsible for damage to your
 *   device, as using this test suite is at your own risk.
 * - As in HOWTO.md, DO NOT memory allocate inside a test unless you are freeing the memory before using an
 *   assertion. The test suite WILL LEAK MEMORY if the assertion fails.
 * - The same warning applies to file operations. Close all files before making any assertions.
 * - There is no concurrency. Tests will always run sequentially in the order defined in your test array.
 * - Only one assert may fail in a test at a time.
 * - As in HOWTO.md, DO NOT USE ANY FUNCTION PREFIXED WITH __ IN YOUR TEST FILES.
 */

#ifndef __TESTSUITE_H__
#define __TESTSUITE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <locale.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

/**
 * Test functions are essentially void functions that take no arguments.
 *
 * They usually will contain at least one assert.
 */
typedef void (*TestFunction)(void);

/**
 * Benchmarking functions are also void funtions with no arguments.
 *
 * They do not contain asserts, and are used to check the performance of a function.
 */
typedef void (*BenchmarkFunction)(void);

#define NAME_MAX_LENGTH 512

// Helpers for printing assertion violations.

/**
 * Set the name of the last file that used an assert.
 *
 * @param filename Name of user file.
 */
void __set_last_file(const char *filename);

/**
 * Set the name of the last function that used an assert.
 *
 * @param caller Name of assert caller.
 */
void __set_last_caller(const char *caller);

/**
 * Set the last name of used assert.
 *
 * @param assert Name of last used assert.
 */
void __set_last_assert(const char *assert);

/**
 * Set the last line where an assert was used.
 *
 * @param line Line where assert was called.
 */
void __set_last_line(const int line);

/**
 * Set the verbose printing status of the test suite.
 *
 * @param should_use Should the test suite use verbose printing?
 */
void use_verbose_print(const bool should_use);

/**
 * A triplet struct holding a test function and the test's name.
 * E.g. test: test_ints_equal | name: "test if two returned ints are equal"
 */
typedef struct {
    TestFunction test;          /**< Pointer to test function. */
    char name[NAME_MAX_LENGTH]; /**< Name or objective of function. */
    bool passed;                /**< Has this test passed? */
} Test;

/**
 * Create a template for an untimed test.
 *
 * @param test_name   Desired identifier for the test.
 * @param description Description of test which will be printed out when running.
 */
#define UNTIMED_TEST(test_name, description) \
    static void __ ## test_name(void);\
    static Test test_name = { __ ## test_name, description, false };\
    static void __ ## test_name(void)

/**
 * Create a template for a timed test.
 *
 * @param test_name   Desired identifier for the test.
 * @param description Description of test which will be printed out when running.
 * @param time_limit  A decimal number of seconds time limit.
 * @param early_exit  Should the function exit early if the time limit expires?
 */
#define TIMED_TEST(test_name, description, time_limit, early_exit) \
    static void __timed_ ## test_name(void);\
    static void __ ## test_name(void) {\
        if (early_exit) assert_time_limit_async(__timed_ ## test_name, timeout);\
        else assert_time_limit(__timed_ ## test_name, timeout);\
    }\
    static Test test_name = { __ ## test_name, description, false };\
    static void __timed_ ## test_name(void)

/**
 * A pair struct holding a benchmark function and the benchmark's name.
 * E.g. test: benchmark_ints_equal | name: "benchmark performance of equality check for ints"
 */
typedef struct {
    BenchmarkFunction benchmark; /**< Pointer to benchmark function. */
    char name[NAME_MAX_LENGTH];  /**< Benchmark name or description. */
} Benchmark;

/**
 * Create a template for an untimed test.
 *
 * @param test_name   Desired identifier for the test.
 * @param description Description of test which will be printed out when running.
 */
#define BENCHMARK(benchmark_name, description) \
    static void __ ## benchmark_name(void);\
    static Benchmark benchmark_name = { __ ## benchmark_name, description };\
    static void __ ## benchmark_name(void)

/**
 * Run an array of tests.
 *
 * @param tests Array of tests to run.
 * @param n     How many tests are in that array.
 */
void __run_tests(Test tests[], const size_t n);
#define run_tests(tests, n) {\
    setlocale(LC_ALL, "");\
    fwprintf(stderr, L"--- TESTS: %s ---\n\n", __FILE__);\
    __run_tests(tests, n);\
}

/**
 * Run an array of benchmarks with the given settings.
 *
 * @param benchmarks Array of benchmarks to run.
 * @param n          How many benchmarks are in that array.
 * @param warmup     Number of warmup iterations for each benchmark.
 * @param times      Number of real iterations for each benchmark.
 */
void __run_benchmarks(const Benchmark benchmarks[], const size_t n, const size_t warmup, const size_t times);
#define run_benchmarks(benchmarks, n, warmup, times) {\
    setlocale(LC_ALL, "");\
    fwprintf(stderr, L"--- BENCHMARKS: %s ---\n\n", __FILE__);\
    __run_benchmarks(benchmarks, n, warmup, times);\
}

/**
 * Reset the number of recorded test failures.
 */
void reset_failures(void);

/**
 * Count the number of test failures in an array of tests .
 * Can only be called *after* running tests on an array of tests.
 *
 * @param  tests Array of tests that have been executed.
 * @param  n     Number of tests.
 * @return       Number of test failures.
 */
size_t count_failures(const Test tests[], const size_t n);

/*
 * These assertion functions are used to test parts of the code inside a test function.
 *
 * For assertion functions that take void * parameters, it is expected that the size of the object being compared is
 * given, and that at the very least, both objects are of the same size.
 *
 * Please be warned that if a struct contains a pointer, the equality check is not a deep check and will not check the
 * value that the pointer holds. Same goes for arrays. An array of pointers will only check the pointer addresses, not
 * the values being pointed to by the pointers in the arrays.
 */

// The general assert template for this test suite.
#define __gen_assert__(name, ...) {\
    const char *__fn_name = #name;\
    __set_last_file(__FILE__);\
    __set_last_caller(__func__);\
    __set_last_line(__LINE__);\
    __set_last_assert(__fn_name + 2);\
    name(__VA_ARGS__);\
}

/**
 * Assert that a condition holds.
 *
 * @param condition The condition to check.
 */
#define assert_true(condition) __gen_assert__(__assert_true, condition)
void __assert_true(const bool condition);

/**
 * Assert that a condition does not hold.
 *
 * @param condition The condition to check.
 */
#define assert_false(condition) __gen_assert__(__assert_false, condition)
void __assert_false(const bool condition);

/**
 * Assert that two unsigned integers are equal.
 * Smaller unsigned integers should be casted (sign-extended) to 64 bits.
 *
 * @param a First unsigned integer.
 * @param b Second unsigned integer.
 */
#define assert_uint_equals(a, b) __gen_assert__(__assert_uint_equals, a, b)
void __assert_uint_equals(uint64_t a, uint64_t b);

/**
 * Assert that two unsigned integers are not equal.
 * Smaller unsigned integers should be casted (sign-extended) to 64 bits.
 *
 * @param a First unsigned integer.
 * @param b Second unsigned integer.
 */
#define assert_uint_not_equals(a, b) __gen_assert__(__assert_uint_not_equals, a, b)
void __assert_uint_not_equals(uint64_t a, uint64_t b);

/**
 * Assert that two signed integers are equal.
 * Smaller signed integers should be casted (sign-extended) to 64 bits.
 *
 * @param a First signed integer.
 * @param b Second signed integer.
 */
#define assert_sint_equals(a, b) __gen_assert__(__assert_sint_equals, a, b)
void __assert_sint_equals(int64_t a, int64_t b);

/**
 * Assert that two signed integers are not equal.
 * Smaller signed integers should be casted (sign-extended) to 64 bits.
 *
 * @param a First signed integer.
 * @param b Second signed integer.
 */
#define assert_sint_not_equals(a, b) __gen_assert__(__assert_sint_not_equals, a, b)
void __assert_sint_not_equals(int64_t a, int64_t b);

/**
 * Assert that two floats are "close enough to be equal".
 * Where abs(a - b) < epsilon.
 *
 * @param a       First single-precision floating point number.
 * @param b       Second single-precision floating point number.
 * @param epsilon Maximum difference (exclusive) between the two numbers.
 */
#define assert_float_equals(a, b, epsilon) __gen_assert__(__assert_float_equals, a, b, epsilon)
void __assert_float_equals(float a, float b, float epsilon);

/**
 * Assert that two floats are not "close enough to be equal".
 * Where abs(a - b) >= epsilon.
 *
 * @param a       First single-precision floating point number.
 * @param b       Second single-precision floating point number.
 * @param epsilon Minimum difference (inclusive) between the two numbers.
 */
#define assert_float_not_equals(a, b, epsilon) __gen_assert__(__assert_float_not_equals, a, b, epsilon)
void __assert_float_not_equals(float a, float b, float epsilon);

/**
 * Assert that two doubles are "close enough to be equal".
 * Where abs(a - b) < epsilon.
 *
 * @param a       First double-precision floating point number.
 * @param b       Second double-precision floating point number.
 * @param epsilon Maximum difference (exclusive) between the two numbers.
 */
#define assert_double_equals(a, b, epsilon) __gen_assert__(__assert_double_equals, a, b, epsilon)
void __assert_double_equals(double a, double b, double epsilon);

/**
 * Assert that two doubles are not "close enough to be equal".
 * Where abs(a - b) >= epsilon.
 *
 * @param a       First double-precision floating point number.
 * @param b       Second double-precision floating point number.
 * @param epsilon Minimum difference (inclusive) between the two numbers.
 */
#define assert_double_not_equals(a, b, epsilon) __gen_assert__(__assert_double_not_equals, a, b, epsilon)
void __assert_double_not_equals(double a, double b, double epsilon);

/**
 * Assert that two (null-terminated) strings are equal (contain the same characters).
 *
 * @param str1 First string.
 * @param str2 String to compare with first.
 */
#define assert_string_equals(str1, str2) __gen_assert__(__assert_string_equals, str1, str2)
void __assert_string_equals(const char *str1, const char *str2);

/**
 * Assert that two (null-terminated) strings are not equal (have differences in their characters).
 *
 * @param str1 First string.
 * @param str2 String to compare with first.
 */
#define assert_string_not_equals(str1, str2) __gen_assert__(__assert_string_not_equals, str1, str2)
void __assert_string_not_equals(const char *str1, const char *str2);

/**
 * Assert that two (null-terminated) wide strings are equal (contain the same characters).
 *
 * @param str1 First wide string.
 * @param str2 String to compare with first.
 */
#define assert_wide_string_equals(str1, str2) __gen_assert__(__assert_wide_string_equals, str1, str2)
void __assert_wide_string_equals(const wchar_t *str1, const wchar_t *str2);

/**
 * Assert that two (null-terminated) wide strings are not equal (have differences in their characters).
 *
 * @param str1 First wide string.
 * @param str2 String to compare with first.
 */
#define assert_wide_string_not_equals(str1, str2) __gen_assert__(__assert_wide_string_not_equals, str1, str2)
void __assert_wide_string_not_equals(const wchar_t *str1, const wchar_t *str2);

/**
 * Assert that two objects are equal.
 *
 * PRE: no object pointer is null.
 * PRE: both objects are the same size.
 *
 * @param obj1 Pointer to the first object to compare.
 * @param obj2 Pointer to the second object to compare.
 * @param size Size of both objects.
 */
#define assert_equals(obj1, obj2, size) __gen_assert__(__assert_equals, obj1, obj2, size)
void __assert_equals(const void *obj1, const void *obj2, const size_t size);

/**
 * Assert that two objects are not equal.
 *
 * PRE: no object pointer is null.
 * PRE: both objects are the same size.
 *
 * @param obj1 Pointer to the first object to compare.
 * @param obj2 Pointer to the second object to compare.
 * @param size Size of both objects.
 */
#define assert_not_equals(obj1, obj2, size) __gen_assert__(__assert_not_equals, obj1, obj2, size)
void __assert_not_equals(const void *obj1, const void *obj2, const size_t size);

/**
 * Assert that two arrays are equal (that is, they contain the same items).
 *
 * PRE: no pointer is null.
 * PRE: both arrays contain objects that are the same size.
 * PRE: both arrays contain at least n items.
 *
 * @param arr1 Pointer to the first array to compare.
 * @param arr2 Pointer to the second array to compare.
 * @param n    Number of items to check.
 * @param size Size of objects in both arrays.
 */
#define assert_array_equals(arr1, arr2, n, size) __gen_assert__(__assert_array_equals, arr1, arr2, n, size)
void __assert_array_equals(const void *arr1, const void *arr2, const size_t n, const size_t size);

/**
 * Assert that two arrays are not equal (that is, they do not contain the same items).
 *
 * PRE: no pointer is null.
 * PRE: both arrays contain objects that are the same size.
 * PRE: both arrays contain at least n items.
 *
 * @param arr1 Pointer to the first array to compare.
 * @param arr2 Pointer to the second array to compare.
 * @param n    Number of items to check.
 * @param size Size of objects in both arrays.
 */
#define assert_array_not_equals(arr1, arr2, n, size) __gen_assert__(__assert_array_not_equals, arr1, arr2, n, size)
void __assert_array_not_equals(const void *arr1, const void *arr2, const size_t n, const size_t size);

/**
 * Assert the deep equality of an n-dimensional array (that is, they contain the same items).
 * You must give each dimension's lengths as individual arguments after the size argument.
 *
 * PRE: no pointer is null.
 * PRE: both arrays contain objects that are the same size.
 * PRE: both arrays have dimensions that match the specification in ns.
 * PRE: both arrays are either flat n-dimensional arrays (arr[][][]...) or pointers to pointers (arr***...).
 *
 * @param arr1      Pointer to the first array to compare.
 * @param arr2      Pointer to the second array to compare.
 * @param arr1isptp Is the first array a pointer-to-pointer array?
 * @param arr2isptp Is the second array a pointer-to-pointer array?
 * @param size      Sizes of objects in both arrays.
 * @param argn      Number of dimensions in the array.
 * @param ns[]      Sizes of each dimension of both arrays.
 */
#define assert_deep_array_equals(arr1, arr2, arr1isptp, arr2isptp, size, ...) {\
    size_t ns[] = { __VA_ARGS__ };\
    __gen_assert__(__assert_deep_array_equals, arr1, arr2, arr1isptp, arr2isptp, size, sizeof(ns) / sizeof(size_t), ns);\
}
void __assert_deep_array_equals(const void *arr1, const void *arr2, const bool arr1isptp, const bool arr2isptp, const size_t size, const size_t argn, const size_t ns[]);

/**
 * Assert the deep inequality of an n-dimensional array (that is, they do not contain the same items).
 * You must give each dimension's lengths as individual arguments after the size argument.
 *
 * PRE: no pointer is null.
 * PRE: both arrays contain objects that are the same size.
 * PRE: both arrays have dimensions that match the specification in ns.
 * PRE: both arrays are either flat n-dimensional arrays (arr[][][]...) or pointers to pointers (arr***...).
 *
 * @param arr1      Pointer to the first array to compare.
 * @param arr2      Pointer to the second array to compare.
 * @param arr1isptp Is the first array a pointer-to-pointer array?
 * @param arr2isptp Is the second array a pointer-to-pointer array?
 * @param size      Sizes of objects in both arrays.
 * @param argn      Number of dimensions in the array.
 * @param ns[]      Sizes of each dimension of both arrays.
 */
#define assert_deep_array_not_equals(arr1, arr2, arr1isptp, arr2isptp, size, ...) {\
    size_t ns[] = { __VA_ARGS__ };\
    __gen_assert__(__assert_deep_array_not_equals, arr1, arr2, arr1isptp, arr2isptp, size, sizeof(ns) / sizeof(size_t), ns);\
}
void __assert_deep_array_not_equals(const void *arr1, const void *arr2, const bool arr1isptp, const bool arr2isptp, const size_t size, const size_t argn, const size_t ns[]);

/**
 * Assert that a pointer is not null.
 *
 * @param ptr Pointer to check.
 */
#define assert_not_null(ptr) __gen_assert__(__assert_not_null, ptr)
void __assert_not_null(const void *ptr);

/**
 * Assert that a pointer is null.
 *
 * @param ptr Pointer to check.
 */
#define assert_null(ptr) __gen_assert__(__assert_null, ptr)
void __assert_null(const void *ptr);

/**
 * Assert that a function exits in under a given amount of time.
 *
 * DO NOT USE ASSERTIONS IN TIME-LIMITED FUNCTIONS.
 *
 * @param func       Function to test.
 * @param time_limit Maximum number of seconds to wait until the assertion fails.
 */
#define assert_time_limit(func, time_limit) __gen_assert__(__assert_time_limit, func, time_limit)
void __assert_time_limit(const TestFunction func, double time_limit);

/**
 * Assert that a function exits in under a given amount of time.
 * This version of the function will terminate early if the function finishes early or if the time limit expires.
 *
 * DO NOT USE ASSERTIONS IN TIME-LIMITED FUNCTIONS.
 *
 * @param func       Function to test.
 * @param time_limit Maximum number of seconds to wait until the assertion fails.
 */
#define assert_time_limit_async(func, time_limit) __gen_assert__(__assert_time_limit_async, func, time_limit)
void __assert_time_limit_async(const TestFunction func, double time_limit);

/**
 * A safe memory allocation function that allows a lightly garbage collected allocation of heap memory.
 * This allows the use of heap memory inside a test function with asserts where asserts can occur before memory is freed.
 *
 * WARNING: Do not free the allocated memory using free. Use testfunc_free to do so if you need to.
 *
 * @param  bytes Number of bytes to allocate on the heap.
 *
 * @return       If allocation is successful, returns the pointer to the start of the allocated location. Returns NULL otherwise.
 */
void *testfunc_malloc(const size_t bytes);

/**
 * A safe memory allocation function that allows a lightly garbage collected allocation of heap memory.
 * This allows the use of heap memory inside a test function with asserts where asserts can occur before memory is freed.
 * This function zeroes out the allocated memory first.
 *
 * WARNING: Do not free the allocated memory using free. Use testfunc_free to do so if you need to.
 *
 * @param  n    Number of objects being allocated.
 * @param  size Size of each individual object.
 *
 * @return      If allocation is successful, returns the pointer to the start of the allocated location. Returns NULL otherwise.
 */
void *testfunc_calloc(const size_t n, const size_t size);

/**
 * A safe memory reallocation function that allows a lightly garbage collected allocation of heap memory.
 * This allows the use of heap memory inside a test function with asserts where asserts can occur before memory is freed.
 * Use a NULL pointer for the ptr argument to make this function act like testfunc_malloc.
 *
 * WARNING: Do not free the allocated memory using free. Use testfunc_free to do so if you need to.
 *
 * @param  ptr   Pointer to memory location to reallocate.
 * @param  bytes Number of intended bytes in reallocated space.
 *
 * @return       If reallocation is successful, returns the pointer to the start of the allocated location. Returns NULL otherwise.
 */
void *testfunc_realloc(void *ptr, const size_t bytes);

/**
 * Free a piece of memory allocated by one of the testfunc_?alloc functions.
 * Putting a NULL pointer in the ptr argument makes the function do nothing.
 *
 * @param ptr Pointer to memory in order to free. NULL pointers or pointers to memory allocated not by testfunc_?alloc functions are ignored.
 */
void testfunc_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif  // __TESTSUITE_H__
