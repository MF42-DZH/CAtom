#include "../testsuite.h"
#include "example.h"

void test_fma_correct_result(void) {
    assert_float_equals(my_fma(1.0f, 1.0f, 0.0f), 1.0f, 0.001f);
    assert_float_equals(my_fma(2.0f, 3.0f, 4.0f), 10.0f, 0.001f);
    assert_float_equals(my_fma(8.0f, 1.5f, 2.5f), 14.5f, 0.001f);
}

void test_fma_negatives(void) {
    assert_float_equals(my_fma(-1.0f, 1.0f, 0.0f), -1.0f, 0.001f);
    assert_float_equals(my_fma(1.0f, -1.0f, 0.0f), -1.0f, 0.001f);
    assert_float_equals(my_fma(-1.0f, -1.0f, 0.0f), 1.0f, 0.001f);
    assert_float_equals(my_fma(-1.0f, -1.0f, -1.0f), 0.0f, 0.001f);
    assert_float_equals(my_fma(-5.0f, 5.0f, 10.0f), -15.0f, 0.001f);
}

void benchmark_fma(void) {
    for (int i = 0; i < 1000; ++i) {
        float a = 16.5f;
        float b = 18.5f;
        float c = 2.0f;

        float t = my_fma(a, b, c);
    }
}

static const Test TESTS[] = {
    { .test = test_fma_correct_result, .name = "Test if fma returns correct results" },
    { .test = test_fma_negatives, .name = "Test if fma correctly handles negatives" }
};

static const Benchmark BENCHMARKS[] = {
    { .benchmark = benchmark_fma, .name = "Performance check for fma" }
};

int main(void) {
    run_tests(TESTS, sizeof(TESTS) / sizeof(Test));
    run_benchmarks(BENCHMARKS, sizeof(BENCHMARKS) / sizeof(Benchmark), 5, 5);
    return 0;
}
