#include <stdio.h>

float subtract(float in1, float in2) {
        float ret = 0.0;
        asm("fsubr %2, %0" : "=&t" (ret) : "%0" (in1), "u" (in2));
        return ret;
}

int main(){
        float a = 1;
        float b = 10;
        float c = subtract(a, b);
        printf("hi!!!: %f\n", c);
        return 0;
}
