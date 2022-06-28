#include <stdint.h>
#include <stdio.h>

/**
 *
 */

int add(uint64_t a, uint64_t b) {
        uint64_t sum;
        printf("Before inline asm: a=%ld, b=%ld\n", a, b);
        __asm__("add %0, %1, %2"
                : "=r" (sum)
                : "r" (a), "r" (b));
        printf("After inline asm: a=%ld, b=%ld\n", a, b);
        return sum;
}


int main() {
        uint64_t x = 23;
        uint64_t y = 19;
        uint64_t res = add(x, y);
        printf("result should be 23+19 = %ld\n", res);
        return 0;
}
