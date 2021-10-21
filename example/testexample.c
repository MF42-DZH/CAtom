#include "../catom.h"
#include "example.h"

UNTIMED_TEST(test_fma_correct_result, "Test if fma returns correct results") {
    assert_float_equals(exm_fma(1.0f, 1.0f, 0.0f), 1.0f, 0.001f);
    assert_float_equals(exm_fma(2.0f, 3.0f, 4.0f), 10.0f, 0.001f);
    assert_float_equals(exm_fma(8.0f, 1.5f, 2.5f), 14.5f, 0.001f);
}

UNTIMED_TEST(test_fma_negatives, "Test if fma correctly handles negatives") {
    assert_float_equals(exm_fma(-1.0f, 1.0f, 0.0f), -1.0f, 0.001f);
    assert_float_equals(exm_fma(1.0f, -1.0f, 0.0f), -1.0f, 0.001f);
    assert_float_equals(exm_fma(-1.0f, -1.0f, 0.0f), 1.0f, 0.001f);
    assert_float_equals(exm_fma(-1.0f, -1.0f, -1.0f), 0.0f, 0.001f);
    assert_float_equals(exm_fma(-5.0f, 5.0f, 10.0f), -15.0f, 0.001f);
}

UNTIMED_TEST(test_failing, "This test will always fail") {
    // This is an example of a test that fails on purpose.
    assert_true(false);
}

BENCHMARK(benchmark_fma, "Performance check for fma") {
    for (int i = 0; i < 1000; ++i) {
        float a = 16.5f;
        float b = 18.5f;
        float c = 2.0f;

        float t = exm_fma(a, b, c);
    }
}

int main(void) {
    Test TESTS[] = { test_fma_correct_result, test_fma_negatives, test_failing };
    const Benchmark BENCHMARKS[] = { benchmark_fma };

    run_tests(TESTS, sizeof(TESTS) / sizeof(Test));
    run_benchmarks(BENCHMARKS, sizeof(BENCHMARKS) / sizeof(Benchmark), 5, 5);

    // The -1 in this example is to compensate for the fact that 1 test always fails.
    // You will not need the -1 in your test files.
    return count_failures(TESTS, sizeof(TESTS) / sizeof(Test)) - 1;
}
