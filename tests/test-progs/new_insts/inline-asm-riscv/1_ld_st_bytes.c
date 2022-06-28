#include <stdio.h>

/**
 * Load 371 to register a6;
 * Store value in register a6 to address of j;
 * Print j, will get 371.
 */

int main() {
        int j;
        // __asm__("li a6, 371");
        //     imm[31:12]        |rd[11:7] | opcode
        // 00000000 00000000 0000|  00000  | 0000000
        // 00000000 00000000 0000|  10000  | 0110111: LUI instruction
        // Since we want to load 371 to register r6 (index 16), imm is 371




        // 00000000 00010111 0011|  10000  | 0110111
        //     imm:  371         | reg r6  | opcode
        // Now we have the final format of the 32-bit instruction,
        // Need to convert it into little endian order
        // 00000000 00010111 00111000 00110111
        //    (3)      (2)      (1)      (0)
        // Little endian means the lowest bits are at the beginning, each byte
        // remains its own value.
        // Hence, the order should be (0), (1), (2), (3)
        // 00110111 00111000 00010111 00000000
        //   0x37     0x38     0x17     0x00

        __asm__(".byte 0b00110111, 0b00011000, 0b0, 0b0");
        __asm__("sw a6, %0"
                :"=m" (j));

        printf("value of j: %lu\n", j);
        return 0;

}


/*
00000000 00000000 0000|  10000  | 0110111

10000000 00000000 00001000 00110111


00000000 00000000 0001|1000 00110111: 4096
00000000 00000000 0010|1000 00110111: 8192
00000001 00000000 0000|1000 00110111: 16777216
00000100 00000000 0000|1000 00110111: 67108864
00000000 10000000 0000|1000 00110111: 8388608


*/
