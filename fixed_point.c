#include "fixed_point.h"
#include <stdio.h>

void print_fixed(int16_t raw, int16_t q) {
    // Scale by 10^6 for 6 decimal places
    int64_t scaled = (int64_t)raw * 1000000;
    int64_t divisor = (int64_t)1 << q;
    int64_t result = scaled / divisor;  // truncation via integer division

    int64_t abs_result = result < 0 ? -result : result;
    int64_t integer_part = abs_result / 1000000;
    int64_t frac_part = abs_result % 1000000;

    if (result < 0) {
        printf("-%lld.%06lld", (long long)integer_part, (long long)frac_part);
    } else {
        printf("%lld.%06lld", (long long)integer_part, (long long)frac_part);
    }
}

int16_t add_fixed(int16_t a, int16_t b) {
    return a + b;
}

int16_t subtract_fixed(int16_t a, int16_t b) {
    return a - b;
}

int16_t multiply_fixed(int16_t a, int16_t b, int16_t q) {
    // Use wider type to avoid overflow
    int32_t product = (int32_t)a * (int32_t)b;
    return (int16_t)(product / (1 << q));
}

void eval_poly_ax2_minus_bx_plus_c_fixed(int16_t x, int16_t a, int16_t b, int16_t c, int16_t q) {
    // Evaluate: y = a*x^2 - b*x + c
    int16_t x_sq = multiply_fixed(x, x, q);
    int16_t ax2 = multiply_fixed(a, x_sq, q);
    int16_t bx = multiply_fixed(b, x, q);
    int16_t ax2_minus_bx = subtract_fixed(ax2, bx);
    int16_t y = add_fixed(ax2_minus_bx, c);

    printf("the polynomial output for a=");
    print_fixed(a, q);
    printf(", b=");
    print_fixed(b, q);
    printf(", c=");
    print_fixed(c, q);
    printf(" is ");
    print_fixed(y, q);
    printf("\n");
}
