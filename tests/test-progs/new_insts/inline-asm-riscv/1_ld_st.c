#include <stdio.h>

/**
 * Load 256 to register a6;
 * Store value in register a6 to address of j;
 * Print j, will get 256.
 */

int main() {
        int j;
        __asm__("li a6, 256");
        __asm__("sw a6, %0"
                :"=m" (j));

        printf("value of j: %d\n", j);
        return 0;

}
