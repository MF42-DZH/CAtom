/**
 * @file      testsuite.h
 * @author    0xFC963F18DC21 (crashmacompilers@gmail.com)
 * @brief     CUnit: A simple C test suite, inspired by JUnit.
 * @version   1.1.0
 * @date      2021-06-18
 *
 * @copyright 0xFC963F18DC21 (c) 2021
 *
 * This is CUnit. A simple, portable test suite and runner inspired by JUnit. It is used to perform unit
 * and (limited) integration testing on simple functions and pieces of data.
 *
 * Like JUnit, all output goes to stderr. Also like JUnit, all tests are guaranteed to run, provided that
 * the system does not run out of memory when allocating strings to print verbose output.
 *
 * It also provides rudimentary benchmarking capabilities, using clock_t in time.h.
 *
 * Compile-time options:
 * - If you want verbose output, add -D__VERBOSE__ to your compile flags when compiling this test suite.
 *
 * User guide:
 * - Create a series of test functions (preferably prefixed with test_), that conform to the TestFunction
 *   function pointer type.
 * - Create a static const array of Test structs, which hold a pointer to the test to run, and a user-friendly
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
 * - As in HOWTO.md, DO NOT USE ANY FUNCTION PREFIXED WITH __ IN YOUR TEST FILES.
 */

#ifndef __TESTSUITE_H__
#define __TESTSUITE_H__

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
void __set_last_file(const char *filename);

void __set_last_caller(const char *caller);

void __set_last_line(const int line);

/**
 * A pair struct holding a test function and the test's name.
 * E.g. test: test_ints_equal | name: "test if two returned ints are equal"
 */
typedef struct {
    TestFunction test;          /**< Pointer to test function. */
    char name[NAME_MAX_LENGTH]; /**< Name or objective of function. */
} Test;

typedef struct {
    BenchmarkFunction benchmark; /**< */
    char name[NAME_MAX_LENGTH];  /**< */
} Benchmark;

/**
 * Run a single test.
 *
 * @param test Test to run.
 */
void run_test(const Test test);

/**
 * Run a single benchmark 'times' amount of times.
 * Before running a benchmark, the function is ran 'warmup' times.
 * Only the time used for the non-warmup runs are usd in the average time calculation.
 *
 * @param  benchmark Benchmark to run.
 * @param  warmup    Number of warmup iterations for benchmark.
 * @param  times     Number of real iterations for benchmark.
 * @return           Time duration of benchmark (all iterations + warmup iterations).
 */
clock_t run_benchmark(const Benchmark benchmark, const size_t warmup, const size_t times);

/**
 * Run an array of tests.
 *
 * @param tests Array of tests to run.
 * @param n     How many tests are in that array.
 */
void __run_tests(const Test tests[], const size_t n);
#define run_tests(tests, n) {\
    fprintf(stderr, "--- TESTS: %s ---\n\n", __FILE__);\
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
    fprintf(stderr, "--- BENCHMARKS: %s ---\n\n", __FILE__);\
    __run_benchmarks(benchmarks, n, warmup, times);\
}

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

#define __gen_assert__(name, ...) {\
    __set_last_file(__FILE__);\
    __set_last_caller(__func__);\
    __set_last_line(__LINE__);\
    name(__VA_ARGS__);\
}

/**
 * Assert that a condition holds.
 *
 * @param condition The condition to check.
 */
void __assert_true(const bool condition);
#define assert_true(condition) __gen_assert__(__assert_true, condition)

/**
 * Assert that a condition does not hold.
 *
 * @param condition The condition to check.
 */
void __assert_false(const bool condition);
#define assert_false(condition) __gen_assert__(__assert_false, condition)

/**
 * Assert that two unsigned integers are equal.
 * Smaller unsigned integers should be casted (sign-extended) to 64 bits.
 *
 * @param a First unsigned integer.
 * @param b Second unsigned integer.
 */
void __assert_uint_equals(uint64_t a, uint64_t b);
#define assert_uint_equals(a, b) __gen_assert__(__assert_uint_equals, a, b)

/**
 * Assert that two unsigned integers are not equal.
 * Smaller unsigned integers should be casted (sign-extended) to 64 bits.
 *
 * @param a First unsigned integer.
 * @param b Second unsigned integer.
 */
void __assert_uint_not_equals(uint64_t a, uint64_t b);
#define assert_uint_not_equals(a, b) __gen_assert__(__assert_uint_not_equals, a, b)

/**
 * Assert that two signed integers are equal.
 * Smaller signed integers should be casted (sign-extended) to 64 bits.
 *
 * @param a First signed integer.
 * @param b Second signed integer.
 */
void __assert_sint_equals(int64_t a, int64_t b);
#define assert_sint_equals(a, b) __gen_assert__(__assert_sint_equals, a, b)

/**
 * Assert that two signed integers are not equal.
 * Smaller signed integers should be casted (sign-extended) to 64 bits.
 *
 * @param a First signed integer.
 * @param b Second signed integer.
 */
void __assert_sint_not_equals(int64_t a, int64_t b);
#define assert_sint_not_equals(a, b) __gen_assert__(__assert_sint_not_equals, a, b)

/**
 * Assert that two floats are "close enough to be equal".
 * Where abs(a - b) < epsilon.
 *
 * @param a       First single-precision floating point number.
 * @param b       Second single-precision floating point number.
 * @param epsilon Maximum difference (exclusive) between the two numbers.
 */
void __assert_float_equals(float a, float b, float epsilon);
#define assert_float_equals(a, b, epsilon) __gen_assert__(__assert_float_equals, a, b, epsilon)

/**
 * Assert that two floats are not "close enough to be equal".
 * Where abs(a - b) >= epsilon.
 *
 * @param a       First single-precision floating point number.
 * @param b       Second single-precision floating point number.
 * @param epsilon Minimum difference (inclusive) between the two numbers.
 */
void __assert_float_not_equals(float a, float b, float epsilon);
#define assert_float_not_equals(a, b, epsilon) __gen_assert__(__assert_float_not_equals, a, b, epsilon)

/**
 * Assert that two doubles are "close enough to be equal".
 * Where abs(a - b) < epsilon.
 *
 * @param a       First double-precision floating point number.
 * @param b       Second double-precision floating point number.
 * @param epsilon Maximum difference (exclusive) between the two numbers.
 */
void __assert_double_equals(double a, double b, double epsilon);
#define assert_double_equals(a, b, epsilon) __gen_assert__(__assert_double_equals, a, b, epsilon)

/**
 * Assert that two doubles are not "close enough to be equal".
 * Where abs(a - b) >= epsilon.
 *
 * @param a       First double-precision floating point number.
 * @param b       Second double-precision floating point number.
 * @param epsilon Minimum difference (inclusive) between the two numbers.
 */
void __assert_double_not_equals(double a, double b, double epsilon);
#define assert_double_not_equals(a, b, epsilon) __gen_assert__(__assert_double_not_equals, a, b, epsilon)

/**
 * Assert that two (null-terminated) strings are equal (contain the same characters).
 *
 * @param str1 First string.
 * @param str2 String to compare with first.
 */
void __assert_string_equals(const char *str1, const char *str2);
#define assert_string_equals(str1, str2) __gen_assert__(__assert_string_equals, str1, str2)

/**
 * Assert that two (null-terminated) strings are not equal (have differences in their characters).
 *
 * @param str1 First string.
 * @param str2 String to compare with first.
 */
void __assert_string_not_equals(const char *str1, const char *str2);
#define assert_string_not_equals(str1, str2) __gen_assert__(__assert_string_not_equals, str1, str2)

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
void __assert_equals(const void *obj1, const void *obj2, const size_t size);
#define assert_equals(obj1, obj2, size) __gen_assert__(__assert_equals, obj1, obj2, size)

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
void __assert_not_equals(const void *obj1, const void *obj2, const size_t size);
#define assert_not_equals(obj1, obj2, size) __gen_assert__(__assert_not_equals, obj1, obj2, size)

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
void __assert_array_equals(const void *arr1, const void *arr2, const size_t n, const size_t size);
#define assert_array_equals(arr1, arr2, n, size) __gen_assert__(__assert_array_equals, arr1, arr2, n, size)

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
void __assert_array_not_equals(const void *arr1, const void *arr2, const size_t n, const size_t size);
#define assert_array_not_equals(arr1, arr2, n, size) __gen_assert__(__assert_array_not_equals, arr1, arr2, n, size)

/**
 * Assert that a pointer is not null.
 *
 * @param ptr Pointer to check.
 */
void __assert_not_null(const void *ptr);
#define assert_not_null(ptr) __gen_assert__(__assert_not_null, ptr)

/**
 * Assert that a pointer is null.
 *
 * @param ptr Pointer to check.
 */
void __assert_null(const void *ptr);
#define assert_null(ptr) __gen_assert__(__assert_null, ptr)

#endif  // __TESTSUITE_H__

