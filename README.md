# CUnit

This is a simple test suite for running unit tests on C code.

## Quick Start

* Clone this test suite into your project.
* Build this test suite. A simple `make` is enough to do so.
* Include `testsuite.h` in the test source code that is calling this test suite.
* Create some test functions that match the signature `void (*TestFunction)(void)`.
* Create an array of `static constant` `Test` structs at the global level in your test source.
* Call `run_tests` using that array of `Test` structs in `main`.
* When building the test sources, link `libtestsuite.a` from this folder.
* Simply run your test code in order to run the tests.

### Example

Example shown in the `example` directory:

`example.h`:
```c
#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

float my_fma(float a, float b, float c);

#endif
```

`example.c`:

```c
#include "example.h"

float my_fma(float a, float b, float c) {
    return a * b + c;
}
```

`testexample.c`:

```c
#include "example.h"
#include "../testsuite.h"

void test_fma_correct_result() {
    assert_float_equals(my_fma(1.0f, 1.0f, 0.0f), 1.0f, 0.001f);
    assert_float_equals(my_fma(2.0f, 3.0f, 4.0f), 10.0f, 0.001f);
    assert_float_equals(my_fma(8.0f, 1.5f, 2.5f), 14.5f, 0.001f);
}

void test_fma_negatives() {
    assert_float_equals(my_fma(-1.0f, 1.0f, 0.0f), -1.0f, 0.001f);
    assert_float_equals(my_fma(1.0f, -1.0f, 0.0f), -1.0f, 0.001f);
    assert_float_equals(my_fma(-1.0f, -1.0f, 0.0f), 1.0f, 0.001f);
    assert_float_equals(my_fma(-1.0f, -1.0f, -1.0f), 0.0f, 0.001f);
    assert_float_equals(my_fma(-5.0f, 5.0f, 10.0f), -15.0f, 0.001f);
}

static const Test TESTS[] = {
    { .test = test_fma_correct_result, .name = "Test if fma returns correct results" },
    { .test = test_fma_negatives, .name = "Test if fma correctly handles negatives" }
};

int main(void) {
    run_tests(TESTS, sizeof(TESTS) / sizeof(Test));
    return 0;
}
```

`Makefile`:

```makefile
CC      = gcc
CFLAGS  = -g3 -D_POSIX_SOURCE -D_DEFAULT_SOURCE -std=c99 -Wextra -Werror -pedantic
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
--- testexample.c ---

Running 2 tests.

--------------------------------------------------------------------------------
[1 / 2] Running test "Test if fma returns correct results":

Test passed. "Test if fma returns correct results" terminated in 0.001000 seconds.
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
[2 / 2] Running test "Test if fma correctly handles negatives":

Test passed. "Test if fma correctly handles negatives" terminated in 0.002000 seconds.
--------------------------------------------------------------------------------

Tests completed in 0.043000 seconds with 2 / 2 passed (0 failed).
```

For more information on available asserts and compile flags, please look at `testsuite.h`.

## Notes

* Do not memory allocate inside a test unless you are freeing it before an assertion. The test suite *will* leak memory if the assertion fails.
* All assertions are macros. Do not use the \_\_-prefixed public functions.
* For a more detailed version of the example including a sample makefile, see the `example` subdirectory.
* You must compile this test suite with at least the `-g` flag and no optimisation flags to get useful results.

