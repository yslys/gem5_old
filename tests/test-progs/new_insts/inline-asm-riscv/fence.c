#include <stdio.h>

int main() {
        int a = 1;
        int b = 0;
        b = a;
        __asm__ volatile("":::"memory");
        a = b;
        return 0;
}
