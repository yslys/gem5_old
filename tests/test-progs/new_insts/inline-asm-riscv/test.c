#include <stdint.h>
#include <stdio.h>

int main() {
    uint64_t a = 1;
    uint64_t *addr = &a;
    uint64_t val = (uint64_t)addr;
    return 0;
}
