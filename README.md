# CAtom

This is a simple test suite for running unit tests on C code.

## Quick Start

<ol start="0">
    <li>Clone this test suite into your project. If your project is also a git repository, add <code>CUnit/*</code> To your <code>.gitignore</code> first.</li>
    <li>Build this test suite. A simple <code>make</code> is enough to do so.</li>
    <li>Include <code>testsuite.h</code> in the test source code that is calling this test suite.</li>
    <li>Create some test functions that match the signature <code>void (*TestFunction)(void)</code>.</li>
    <li>Create a <code>static constant</code> array of <code>Test</code> structs at the global level in your test source.</li>
    <li>Call <code>run_tests</code> using that array of <code>Test</code> structs in <code>main</code>.</li>
    <li>When building the test sources, link <code>libtestsuite.a</code> from this folder.</li>
    <li>Simply run your test code in order to run the tests.</li>
    <li>Repeat steps 4-6 using the <code>void (*BenchmarkFunction)(void)</code> signature, the <code>Benchmark</code> struct and the <code>run_benchmarks</code> function to run a set of benchmarks.</li>
</ol>

### Example

Example shown in the `example` directory:

`example.h`:
```c
#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

float exm_fma(float a, float b, float c);

#endif
```

`example.c`:

```c
#include "example.h"

float exm_fma(float a, float b, float c) {
    return a * b + c;
}
```

`testexample.c`:

```c
#include "../testsuite.h"
#include "example.h"

void test_fma_correct_result(void) {
    assert_float_equals(exm_fma(1.0f, 1.0f, 0.0f), 1.0f, 0.001f);
    assert_float_equals(exm_fma(2.0f, 3.0f, 4.0f), 10.0f, 0.001f);
    assert_float_equals(exm_fma(8.0f, 1.5f, 2.5f), 14.5f, 0.001f);
}

void test_fma_negatives(void) {
    assert_float_equals(exm_fma(-1.0f, 1.0f, 0.0f), -1.0f, 0.001f);
    assert_float_equals(exm_fma(1.0f, -1.0f, 0.0f), -1.0f, 0.001f);
    assert_float_equals(exm_fma(-1.0f, -1.0f, 0.0f), 1.0f, 0.001f);
    assert_float_equals(exm_fma(-1.0f, -1.0f, -1.0f), 0.0f, 0.001f);
    assert_float_equals(exm_fma(-5.0f, 5.0f, 10.0f), -15.0f, 0.001f);
}

void test_failing(void) {
    // This is an example of a test that fails on purpose.
    assert_true(false);
}

void benchmark_fma(void) {
    for (int i = 0; i < 1000; ++i) {
        float a = 16.5f;
        float b = 18.5f;
        float c = 2.0f;

        float t = exm_fma(a, b, c);
    }
}

static const Test TESTS[] = {
    { .test = test_fma_correct_result, .name = "Test if fma returns correct results" },
    { .test = test_fma_negatives, .name = "Test if fma correctly handles negatives" },
    { .test = test_failing, .name = "This test will always fail" }
};

static const Benchmark BENCHMARKS[] = {
    { .benchmark = benchmark_fma, .name = "Performance check for fma" }
};

int main(void) {
    run_tests(TESTS, sizeof(TESTS) / sizeof(Test));
    run_benchmarks(BENCHMARKS, sizeof(BENCHMARKS) / sizeof(Benchmark), 5, 5);
    return 0;
}
```

`Makefile`:

```makefile
CC      = gcc
CFLAGS  = -g3 -Og -D_POSIX_SOURCE -D_DEFAULT_SOURCE -std=c99 -Wextra -Werror -pedantic
LDFLAGS = -L.. -ltestsuite
TARGET  = testexample
OBJS    = example.o testexample.o
BUILD   = $(TARGET)

.SUFFIXES: .c .o

.PHONY: all clean mod rebuild test testsuite

all: mod testsuite $(BUILD) test

rebuild: clean all

clean:
	rm -f $(BUILD) *.o

$(TARGET): $(OBJS)
	gcc $(OBJS) -o $@ $(LDFLAGS)

mod:
	touch $(TARGET)

test:
	./$(TARGET)

testsuite:
	+$(MAKE) -C ..

example.o: example.h
```

Then to run, do:
* `make` the test suite in the test code's directory (in this case, `example`).

For this example above, expect the following output or similar:

```
--- TESTS: testexample.c ---

Running 3 tests.

--------------------------------------------------------------------------------
[1 / 3] Running test "Test if fma returns correct results":

Test passed. "Test if fma returns correct results" terminated in 0.000059 seconds.
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
[2 / 3] Running test "Test if fma correctly handles negatives":

Test passed. "Test if fma correctly handles negatives" terminated in 0.000039 seconds.
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
[3 / 3] Running test "This test will always fail":

[testexample.c] Assertion Failed. assert_true failed in test_failing at line 20:
BOOL is TRUE: 0?

Test failed. "This test will always fail" terminated in 0.000018 seconds.
--------------------------------------------------------------------------------

Tests completed in 0.000214 seconds with 2 / 3 passed (1 failed).

--- BENCHMARKS: testexample.c ---

Running 1 benchmark.

--------------------------------------------------------------------------------
[1 / 1] Running benchmark "Performance check for fma":

Running warmup iteration 1 / 5. Finished warmup iteration 1 / 5 in 0.000007 seconds.
Running warmup iteration 2 / 5. Finished warmup iteration 2 / 5 in 0.000006 seconds.
Running warmup iteration 3 / 5. Finished warmup iteration 3 / 5 in 0.000006 seconds.
Running warmup iteration 4 / 5. Finished warmup iteration 4 / 5 in 0.000005 seconds.
Running warmup iteration 5 / 5. Finished warmup iteration 5 / 5 in 0.000006 seconds.
Running benchmark iteration 1 / 5. Finished benchmark iteration 1 / 5 in 0.000006 seconds.
Running benchmark iteration 2 / 5. Finished benchmark iteration 2 / 5 in 0.000007 seconds.
Running benchmark iteration 3 / 5. Finished benchmark iteration 3 / 5 in 0.000006 seconds.
Running benchmark iteration 4 / 5. Finished benchmark iteration 4 / 5 in 0.000006 seconds.
Running benchmark iteration 5 / 5. Finished benchmark iteration 5 / 5 in 0.000006 seconds.

Benchmark complete.
"Performance check for fma" finished 5 iterations (and 5 warmup iterations) in 0.000031 seconds (0.000061 seconds with warmup).
It took 0.000006 seconds on average to run (0.000006 seconds average with warmup).
--------------------------------------------------------------------------------

Benchmarks completed in 0.000061 seconds.
```

## Documentation

Documentation is available in the `doc` folder. You can manually generate it with doxygen, or use the makefile to call doxygen for you. The makefile also contains cleaning commands to clear out the created documentation.

## Notes

* Do not memory allocate inside a test unless you are freeing it before an assertion. The test suite *will* leak memory if the assertion fails.
* All assertions are macros. Do not use the \_\_-prefixed public functions.
* For a more detailed version of the example including a sample makefile, see the `example` subdirectory.
* You must compile this test suite and any test sources with at least the `-g` flag and no optimisation flags (alternatively `-Og`) to get useful results.
* On Windows, use `Makefile.win` instead of the normal `Makefile`.
* Use the `-V` makefiles to use the verbose test suite. Useful for debugging when asserts occur.
* You may get a warning from your linter that adding an int to a string does not concatenate them when you're using the assert macros. Feel free to ignore that warning.
