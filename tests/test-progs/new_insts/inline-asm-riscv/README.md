# Inline Assembly in RISC-V

## Resources:
+ https://0xax.gitbooks.io/linux-insides/content/Theory/linux-theory-3.html
+ https://github.com/riscv-non-isa/riscv-asm-manual/blob/master/riscv-asm.md


## Start

Before inline-assembly'ing any registers, need to check this [webite](https://github.com/riscv-non-isa/riscv-asm-manual/blob/master/riscv-asm.md). All the registers that we could use are in the ABI column.

I used to try to use ```a8``` register, but it could not be recognized. That is because ```a8``` is not defined in ABIs.

By checking the generated code of some simple programs, the most frequently used registers are ```a``` family registers, which serve as function arguments. ```t``` family registers represent temporary registers. ```s``` family registers are saved registers. (Remember to check the website! The **ABI**!)


Some notations:
```
__asm__("li a6, 256");
__asm__("sw a6, %0"
        :"=m" (j));
```
+ ```"=m"```: The whole string inside the quotes is called ```constraint(s)```.
+ ```=```: This symbol is ```modifier``` which denotes output value.
+ ```m```: This is ```memory specifier``` that tells the compiler that the given inline assembly statement executes read/write operations on memory not specified by operands in the output list. This prevents the compiler from keeping memory values loaded and cached in registers.
+ ```r```: This is ```qualifier``` that denotes the placement of an operand. The r symbol means a value will be stored in one of the general purpose register.
+ ```0``` or any other digit from ```0``` to ```9```: This is called ```matching constraint```. With this a single operand can be used for multiple roles. The value of the constraint is the source operand index.

## 1\_ld\_st.c
```
__asm__("li a6, 256");
__asm__("sw a6, %0"
        :"=m" (j));
```

This code does 1 load and 1 store. Load 256 into a register, then store it to a memory address (of ```j```). This is equivalent to ```int j = 256;```.

You can notice that the output operand has constraint ```"=m"```. 
+ ```"="```: This symbol is modifier which denotes output value.
+ ```"m"```: This is ```memory specifier``` that tells the compiler that the given inline assembly statement executes read/write operations on memory not specified by operands in the output list.


## 2\_add.c
```
int add(uint64_t a, uint64_t b) {
        uint64_t sum;
	printf("Before inline asm: a=%ld, b=%ld\n", a, b);
        __asm__("add %0, %1, %2"
                : "=r" (sum)
                : "r" (a), "0" (b));
        printf("After inline asm: a=%ld, b=%ld\n", a, b);
        return sum;
}
```
Here we use ```"=r"``` for variable ```sum```. **I tried to use ```"=m"``` since I thought, "we are writing the result to sum, so it is a store, isn't it?". But I was wrong!**

**Whenever we want to implement the inline assembly. We first need to check what the operands are.**
+ Are they registers? Or...
+ Are they memory addresses that stored in some registers?

If the operands are registers, like ```add```, we should not use ```"=m"``` to specify the output operand. Instead, we should use ```"=r"``` since we are writing the result to a register. Then the next instruction, as you have "predicted", is a store instruction which stores the result in register to the memory address of the variable.

