
## Abbreviations
+ AMO: Atomic Memory Operations
+ LR/SC: Load-Reserved/Store-Conditional
+ RMW: Read-Modify-Write
+ rel: release
+ CIL: Common Intermediate Language
+ CSR: Control and Status Register
+ aq: acquire
+ rl: release





## .isa files organization

### formats.isa
src/arch/riscv/isa/formats/formats.isa

This file is a file that includes all the formats for various types of instructions, which contains:

+ The basic instruction format
    + [src/arch/riscv/isa/formats/basic.isa](#basicisa-basic-instructions)
+ The type formats
    + [src/arch/riscv/isa/formats/standard.isa](#standardisa)
    + [src/arch/riscv/isa/formats/fp.isa](#fpisa-floating-point)
    + [src/arch/riscv/isa/formats/mem.isa](#memisa-memory-instructions)
    + [src/arch/riscv/isa/formats/amo.isa](#amoisa-atomic-memory-operations)
+ Formats for non-standard extensions
    + [src/arch/riscv/isa/formats/compressed.isa](#compressedisa-compressed-instructions-16-bit-in-length)
+ Pseudo operations format
    + [src/arch/riscv/isa/formats/m5ops.isa](#m5opsisa-pseudo-instructions)
+ The unknown instructions format
    + [src/arch/riscv/isa/formats/unknown.isa](#unknownisa-the-unknown-instructions)


// Pseudo operations
##include "m5ops.isa"

// Include the unknown
##include "unknown.isa"

### basic.isa (basic instructions)
src/arch/riscv/isa/formats/basic.isa

Basic instructions do not consist of three stages like AMO does; hence it only contains execute() template.
+ Declaration templates
    + BasicDeclare
+ Constructor templates:
    + BasicConstructor
+ execute() templates
    + BasicExecute
+ basic decode template:
    + BasicDecode

Decode format is as follows:
+ BasicOp

### standard.isa
This file contains some integer instructions.

+ Immediate
    + ImmDeclare
    + ImmConstructor
    + ImmExecute
    + CILuiExecute
+ Fence
    + FenceExecute
+ Branch
    + BranchDeclare
    + BranchExecute
+ Jump
    + JumpDeclare
    + JumpExecute
+ All CSR instructions atomically read-modify-write a single CSR
    + CSRExecute

Formats:
+ ROp (R-type)
+ IOp (I-type)
+ FenceOp
+ BOp (Branch)
+ Jump
+ UOp (U-type)
+ JOp (Jump)
+ SystemOp (SYSTEM instructions) 
    + System instructions are used to access system functionality that might require privileged access and are encoded using the I-type instruction format.
+ CSROp (Control and Status Register)

### fp.isa (Floating Point)
standard.isa contains integer instructions, while in this file (fp.isa), it defines floating point instructions.

Template:
+ FloatExecute

Format:
+ FPROp (Floating Point Register?)


### mem.isa (Memory instructions)
This file contains memory instructions (load and store operations)

Memory operation instruction templates:
+ Both load and store share the same declaration and constructor
    + LoadStoreDeclare
    + LoadStoreConstructor
+ Load:
    + LoadExecute
    + LoadInitiateAcc
    + LoadCompleteAcc
+ Store:
    + StoreExecute
    + StoreInitiateAcc
    + StoreCompleteAcc


Instruction formats:
+ Load
+ Store
+ Note that both two formats internally contains a function called LoadStoreBase.



### amo.isa (Atomic Memory Operations)
src/arch/riscv/isa/formats/amo.isa

In this file, it contains a set of templates listed below:
+ Declaration templates:
    + LRSCDeclare
    + LRSCMicroDeclare
    + AtomicMemOpDeclare
    + AtomicMemOpRMWDeclare
+ Constructor templates:
    + LRSCMacroConstructor
    + LRSCMicroConstructor
    + AtomicMemOpMacroConstructor
    + AtomicMemOpRMWConstructor
+ execute() templates
    + LoadReservedExecute
    + StoreCondExecute
    + AtomicMemOpRMWExecute
+ initiateAcc() templates
    + LoadReservedInitiateAcc
    + StoreCondInitiateAcc
    + AtomicMemOpRMWInitiateAcc
+ completeAcc() templates
    + LoadReservedCompleteAcc
    + StoreCondCompleteAcc
    + AtomicMemOpRMWCompleteAcc

Each "operation" (use this term for now) consists of three stages: initiateAcc, execute, completeAcc. We can declare the operation, and use the constructor to create an instance, then implement all three stages of each operation.

It also contains decode formats:
+ LR/SC/AMO decode formats
    + LoadReserved
    + StoreCond
    + AtomicMemOp


### compressed.isa (compressed instructions: 16-bit in length)
+ Compressed basic instructions class declaration template:
    + CBasicDeclare
+ Compressed basic instruction class execute method template:
    + CBasicExecute

decode formats:
+ CROp
+ CIAddi4spnOp
+ CIOp
+ CJOp
+ CBOp
+ CompressedLoad
+ CompressedStore
+ CompressedROp


### m5ops.isa (pseudo instructions)
It only defines the format:
+ M5Op

### unknown.isa (the unknown instructions)
It only defines the format:
+ Unknown


## Atomic Memory Operation Instructions - in detail
Since we want to simulate ENQCMD, we need to find an instruction which has the similar behavior of ENQCMD. ENQCMD does an atomic memory write, then set ZF flag (in X86) to indicate whether enqueue success or failure. Hence, the best suited instruction falls into the category of atomic memory operation instructions (aio.isa), in which, the ```StoreCond``` family of instructions is what we are looking for.

If you are wondering why, check [“A” Standard Extension for Atomic Instructions, Version 2.1](https://five-embeddev.com/riscv-isa-manual/latest/a.html#ref-Gharachorloo90memoryconsistency). Some useful notes below:
+ Base RISC-V ISA has a relaxed memory model, with the FENCE instruction used to impose additional ordering constraints.
+ To support release consistency:
    + Two bits (in the atomic instruction) are provided: ```aq``` (acquire) and ```rl``` (release)
    + If both bits are clear
        + no additional ordering constraints are imposed on the atomic memory operation.
    + If only ```aq``` bit is set
        + no following memory operations on this RISC-V hart can be observed to take place before the acquire memory operation.
    + If only the ```rl``` bit is set
        + the release memory operation cannot be observed to take place before any earlier memory operations on this RISC-V hart.
    + If both the ```aq``` and ```rl``` bits are set
        +  the atomic memory operation is sequentially consistent and cannot be observed to happen before any earlier memory operations or after any later memory operations in the same RISC-V hart and to the same address domain.
+ load-reserved (LR) and store-conditional (SC) instructions
    + Complex atomic memory operations on a single memory word or doubleword are performed with the load-reserved (LR) and store-conditional (SC) instructions. 
    + ```LR.W```: loads a word from the address in rs1, places the sign-extended value in rd, and registers a reservation set — a set of bytes that subsumes the bytes in the addressed word. 
    + ```SC.W``` conditionally writes a word in rs2 to the address in rs1: the SC.W succeeds only if the reservation is still valid and the reservation set contains the bytes being written. 
        + If the ```SC.W``` succeeds, the instruction writes the word in rs2 to memory, and it writes zero to rd. 
        + If the ```SC.W``` fails, the instruction does not write to memory, and it writes a nonzero value to rd. 
        + Regardless of success or failure, executing an SC.W instruction invalidates any reservation held by this hart. 
    + ```LR.D``` and ```SC.D``` act analogously on doublewords and are only available on RV64. For RV64, LR.W and SC.W sign-extend the value placed in rd.

**The best way to figure out how it works is by investigating one example. Since ENQCMD does a 64-byte MMIO write to memory (which is mapped to device memory - portal), we need to use ```sc_d``` and ```sc_w``` instructions as examples (check build/RISCV/arch/riscv/generated/exec-ns.cc.inc).**

The generated code is C++, which is easier to understand. Hence, what we are going to do next is to start from the generated C++ code, to trace back to the original .isa files.

In generated code: there is an one line comment before each instruction:
```
// StoreCond::sc_d((['\n                    Mem = Rs2;\n                ', '\n                    Rd = result;\n                '], {'mem_flags': 'LLSC', 'inst_flags': 'IsStoreConditional'}))
```
To make it more readable:
```
StoreCond::sc_d((
    [
        'Mem = Rs2;', 
        'Rd = result;'
    ], 
    {
        'mem_flags': 'LLSC', 
        'inst_flags': 'IsStoreConditional'
    }
))
```

+ How to interpret this one-line comment?
    + This one line comment consists of two parts: a list of C++ code, and a dictionary of the // TODO: 
    + List: two lines of C++ code that simulates the behavior of the ```sc_d``` instruction.
    + Dictionary: two key-value pairs to indicate the properties of this instruction
        + ```LLSC```: ([src](https://en.wikipedia.org/wiki/Load-link/store-conditional))
            + load-linked/store-conditional (LL/SC), a.k.a. load-reserved/store-conditional (LR/SC), are a pair of instructions used in multithreading to achieve synchronization. 
            + Load-link (LL, or LR) returns the current value of a memory location, while a subsequent store-conditional (SC) to the same memory location will store a new value only if no updates have occurred to that location since the load-link. Together, this implements a lock-free atomic read-modify-write operation.
        + ```'inst_flags': 'IsStoreConditional'```: instruction type

+ Where does this one-line comment come from?
    + In ```arch/riscv/isa/decoder.isa```, search for ```StoreCond::sc_d```, we can see the following:
```
0x3: StoreCond::sc_d(
    {{ Mem = Rs2; }}, 
    {{ Rd = result; }}, 
    mem_flags=LLSC, 
    inst_flags=IsStoreConditional
);
```

+ How does ISA parser in gem5 interprets this decode format and translate it into C++ code?
    + ISA parser: ```src/arch/isa_parser/isa_parser.py```
    + The class name is ```StoreCond```, which corresponds to the ```format StoreCond``` in amo.isa file.
    + ISA parser decodes one generated instruction (0-1 bit string), then figure out that it lies into ```StoreCond``` category. Then, the ```format StoreCond``` in amo.isa file serves as a guidance of how to chop down those bits into pieces and interpret them piece-by-piece.
    + By checking ```format StoreCond```, it takes in 5 arguments:
        + ```(memacc_code, postacc_code={{ }}, ea_code={{EA = Rs1;}}, mem_flags=[], inst_flags=[])```
        + ```memacc_code```: 1st element in the list.
        + ```postacc_code={{ }}```: 2nd element in the list
        + ```ea_code={{EA = Rs1;}}```: this is not done by the decoder. 
        + ```mem_flags=[]```: the dictionary key in the dict.
        + ```inst_flags=[]```: the dictionary key in the dict.



+ Before continue, check ```format AtomicMemOp``` in amo.isa. 
    + It takes in 6 arguments: ```(memacc_code, amoop_code, postacc_code={{ }}, ea_code={{EA = Rs1;}}, mem_flags=[], inst_flags=[])```
    + The corresponding code in decoder.isa is shown below:

```
0x0: AtomicMemOp::amoadd_d(
    {{ Rd_sd = Mem_sd;}}, 
    {{
        TypedAtomicOpFunctor<int64_t> *amo_op =
                new AtomicGenericOp<int64_t>(Rs2_sd,
                        [](int64_t* b, int64_t a){ *b += a; });
    }}, 
    mem_flags=ATOMIC_RETURN_OP
);
```

+ It contains 

Next, we could check the actual templates in ```src/arch/riscv/isa/formats/amo.isa```. Recall that the templates consist of three parts: InitiateAcc, Execute and CompleteAcc. What I will do next is to compare the template with the generated C++ code.

InitiateAcc:
```
// template
def template StoreCondInitiateAcc {{
    Fault
    %(class_name)s::%(class_name)sMicro::initiateAcc(ExecContext *xc,
                                            Trace::InstRecord *traceData) const
    {
        Addr EA;

        %(op_decl)s;
        %(op_rd)s;
        %(ea_code)s;
        %(memacc_code)s;

        {
            Fault fault = writeMemTimingLE(xc, traceData, Mem, EA,
                                           memAccessFlags, nullptr);
            if (fault != NoFault)
                return fault;
        }

        %(op_wb)s;

        return NoFault;
    }
}};

// generated cpp
// %(class_name)s = Sc_d
Fault
Sc_d::Sc_dMicro::initiateAcc(ExecContext *xc,
                             Trace::InstRecord *traceData) const
{
    Addr EA; // same as template

    // %(op_decl)s;
    uint64_t Rs1 = 0;
    uint64_t Rs2 = 0;
    uint64_t Mem = {};
    ;
    // %(op_rd)s;
    Rs1 = xc->readIntRegOperand(this, 0);
    Rs2 = xc->readIntRegOperand(this, 1);
    ;
    // %(ea_code)s;
    EA = Rs1;
    ;

    // %(memacc_code)s;
    Mem = Rs2;
    ;

    {
        Fault fault = writeMemTimingLE(xc, traceData, Mem, EA,
                                       memAccessFlags, nullptr);
        if (fault != NoFault)
            return fault;
    }
    ;

    // %(op_wb)s; = NULL

    return NoFault;
}
```


Execute:
```
// template
def template StoreCondExecute {{
    Fault %(class_name)s::%(class_name)sMicro::execute(ExecContext *xc,
                                            Trace::InstRecord *traceData) const
    {
        Addr EA;
        uint64_t result;

        %(op_decl)s;
        %(op_rd)s;
        %(ea_code)s;

        %(memacc_code)s;

        {
            Fault fault = writeMemAtomicLE(xc, traceData, Mem, EA, 
                                        memAccessFlags, &result);
            if (fault != NoFault)
                return fault;
        }
        // RISC-V has the opposite convention gem5 has for success flags,
        // so we invert the result here.
        result = !result;

        %(postacc_code)s;
        %(op_wb)s;

        return NoFault;
    }
}};

// generated cpp code
Fault Sc_d::Sc_dMicro::execute(ExecContext *xc,
                               Trace::InstRecord *traceData) const
{
    Addr EA; // same as template
    uint64_t result; // same as template

    //  %(op_decl)s;
    uint64_t Rd = 0;
    uint64_t Rs1 = 0;
    uint64_t Rs2 = 0;
    uint64_t Mem = {};
    ;
    // %(op_rd)s;
    Rs1 = xc->readIntRegOperand(this, 0);
    Rs2 = xc->readIntRegOperand(this, 1);
    ;
    // %(ea_code)s;
    EA = Rs1;
    ;
    // %(memacc_code)s;
    Mem = Rs2;
    ;

    {
        Fault fault = writeMemAtomicLE(xc, traceData, Mem, EA, memAccessFlags,
                                        &result);
        if (fault != NoFault)
            return fault;
    }
    // comments in template are also shown in generated cpp file
    // RISC-V has the opposite convention gem5 has for success flags,
    // so we invert the result here.
    result = !result; // same as template

    // %(postacc_code)s;
    Rd = result;
    ;
        
    // %(op_wb)s;
    {
        uint64_t final_val = Rd;
        xc->setIntRegOperand(this, 0, final_val);

        if (traceData) { 
            traceData->setData(final_val); 
        }
    };

    return NoFault;
}

```

CompleteAcc:
```
// template
def template StoreCondCompleteAcc {{
    Fault %(class_name)s::%(class_name)sMicro::completeAcc(Packet *pkt,
                        ExecContext *xc, Trace::InstRecord *traceData) const
    {
        %(op_dest_decl)s;

        // RISC-V has the opposite convention gem5 has for success flags,
        // so we invert the result here.
        uint64_t result = !pkt->req->getExtraData();

        %(postacc_code)s;
        %(op_wb)s;

        return NoFault;
    }
}};


// generated cpp code
Fault Sc_d::Sc_dMicro::completeAcc(Packet *pkt,
                        ExecContext *xc, Trace::InstRecord *traceData) const
{
    // %(op_dest_decl)s;
    uint64_t Rd = 0;
    ;

    // RISC-V has the opposite convention gem5 has for success flags,
    // so we invert the result here.
    uint64_t result = !pkt->req->getExtraData(); // same as template

    /** New comments added
     * @pkt: Packet *
     * @pkt->req: RequestPtr, a pointer to the original request (mem write)
     * @getExtraData(): Accessor function for store conditional return value.
     *                  It will return a uint64_t value.
     */


    // %(postacc_code)s;
    Rd = result; /* store the result in Rd register */
    ;
    
    // %(op_wb)s;
    {
        uint64_t final_val = Rd;
        xc->setIntRegOperand(this, 0, final_val);

        if (traceData) { traceData->setData(final_val); }
    };

    return NoFault;
}
```



### How decoder decodes SC.D instruction?
```
SC.D (64-bit double word):
[0:6]: Opcode: 0101111
[7:11]: rd
[12:14]: func3: 011
[15:19]: rs1
[20:24]: rs2
([25:31]: func7)
    [25]: rl
    [26]: aq
    [27:31]: 00011 
```

+ The decode value always starts from high bits to low bits.
+ Decode ```QUADRANT<1:0> (2 bits)```, in case 0x03, i.e. 0b11.
    + As we can see the Opcode part (0100011) of SC.D instruction, the ```<1:0>``` part is ```11```.

+ Decode ```OPCODE<6:2> (5 bits)```, in case 0x0b, i.e. 0b1011.
    + As we can see the ```<6:2>``` bits of Opcode (01011), it is 0x0b.

+ Decode ```FUNCT3<14:12> (3 bits)```, in case 0x3, i.e. 0b011.
    + As we can see the ```<14:12>``` bits of SC.D instruction is 011.

+ Decode ```AMOFUNCT<31:27> (5 bits)```, in case 0x3, i.e. 0b00011.
    + As we can see the ```<31:27>``` bits of SC.D instruction is 00011.

Now, we have fully decoded the instruction SC.D

### How decoder decodes LR.D instruction?
```
LR.D (64-bit double word):
[0:6]: Opcode: 0101111
[7:11]: rd
[12:14]: func3: 011
[15:19]: rs1
[20:24]: rs2: 00000
([25:31]: func7)
    [25]: rl
    [26]: aq
    [27:31]: 00010 
```

+ The decode value always starts from high bits to low bits.
+ Decode ```QUADRANT<1:0> (2 bits)```, in case 0x03, i.e. 0b11.
    + As we can see the Opcode part (0100011) of SC.D instruction, the ```<1:0>``` part is ```11```.

+ Decode ```OPCODE<6:2> (5 bits)```, in case 0x0b, i.e. 0b1011.
    + As we can see the ```<6:2>``` bits of Opcode (01011), it is 0x0b.

+ Decode ```FUNCT3<14:12> (3 bits)```, in case 0x3, i.e. 0b011.
    + As we can see the ```<14:12>``` bits of SC.D instruction is 011.

+ Decode ```AMOFUNCT<31:27> (5 bits)```, in case 0x2, i.e. 0b00010.
    + As we can see the ```<31:27>``` bits of SC.D instruction is 00010.

Now, we have fully decoded the instruction LR.D.

### Difference between LR.D and SC.D
Two differences:
+ ```AMOFUNCT<31:27> (5 bits)```
    + LR.D: 00010
    + SC.D: 00011
+ ```Rs2<24:20> (5 bits)```
    + LR.D: 00000
    + SC.D: other values

### LR.W and SC.W - similar instructions but operate on 32-bit words
+ They have the same ```QUADRANT<1:0> (2 bits)```.
+ They have the same ```OPCODE<6:2> (5 bits)```.
+ ```FUNCT3<14:12> (3 bits)``` are different:
    + ```010```: for 32-bit word operations.
    + ```011```: for 64-bit double operations.
+ ```AMOFUNCT<31:27> (5 bits)``` are the same **respectively**.
    + ```00010```: for LR.W and LR.D.
    + ```00011```: for SC.W and SC.D.

## Design of ENQCMD:
LR_Q and SC_Q - two instructions.

+ Same ```QUADRANT<1:0> (2 bits)```.
+ Same ```OPCODE<6:2> (5 bits)```.
+ ```FUNCT3<14:12> (3 bits)```: I pick ```100```.
+ ```AMOFUNCT<31:27> (5 bits)``` are the same **respectively**.
    + ```00010```: for LR.Q.
    + ```00011```: for SC.Q.

+ SC_Q is an uncacheable instruction
    + src/mem/request.hh: This file defines all the flags, including ```UNCACHEABLE```.

LR_Q:
```
// Note: rs1 is a6 register, i.e. x16 register, 0b10000
format:
    00010|aq|rl|00000|rs1<5>|100|rd<5>|0101111
aq = 1, rl = 1:
    00010|1|1|00000|rs1<5>|100|rd<5>|0101111
rs1 = 10000 (a6/x16 reg)
    000101100000|10000|100|rd<5>|0101111
rd register: s6/x22 reg, i.e. 0b10110
    000101100000|10000|100|10110|0101111

Final result:
    00010110|00001000|01001011|00101111
      0x16     0x08     0x4b     0x2f

inline asm (little endian, needs to reverse the bytes)
    __asm__ volatile(".byte 0x2f, 0x4b, 0x08, 0x16");
```

User testing code: **in order to make LR_Q work, must make sure the 64-byte data to be transferred is on a cache line block size boundary.** Hence, we use ```memalign()```.

SC_Q:
```
// Note, rs1 is a6 register. Hence, when sc_q is executed, we need to make sure
// rs6 stores the destination address already. a6, x16, i.e. 0b10000

// Note, sc_q instruction does not use rs2 register. Hence we can igore those 
// bits. For safety, I use t6/x31 (temporary register t6), i.e. 0b11111
format:
    00011|aq|rl|rs2<5>|rs1<5>|100|rd<5>|0101111
aq = 1, rl = 1:
    0001111|rs2<5>|rs1<5>|100|rd<5>|0101111
rs1 = 10000 (a6/x16 reg)
    0001111|rs2<5>|10000|100|rd<5>|0101111
rs2 = 00000 (zero/x0 reg)
    0001111|00000|10000|100|rd<5>|0101111
rd register: s6/x22 register, i.e. 0b10111
    0001111|00000|10000|100|10110|0101111

Final result:
    00011110|00001000|01001011|00101111
      0x1e     0x08     0x4b    0x2f

inline asm (little endian, needs to reverse the bytes)
    __asm__ volatile(".byte 0x2f, 0x4b, 0x08, 0x1e");
```

If both bits are clear, no additional ordering constraints are imposed on the atomic memory operation. If only the aq bit is set, the atomic memory operation is treated as an acquire access,
i.e., no following memory operations on this RISC-V hart can be observed to take place before
the acquire memory operation. If only the rl bit is set, the atomic memory operation is treated
as a release access, i.e., the release memory operation can not be observed to take place before
any earlier memory operations on this RISC-V hart. If both the aq and rl bits are set, the atomic
memory operation is sequentially consistent and cannot be observed to happen before any earlier
memory operations or after any later memory operations in the same RISC-V hart, and can only be
observed by any other hart in the same global order of all sequentially consistent atomic memory
operations to the same address domain.



Need a temporary 512-bit register. ENQCMD = LR_64 + SC_64


Load-reserved instruction: LR loads a word from the address in rs1, places the
sign-extended value in rd, and registers a reservation on the memory address.

**When initiateAcc() is called, the registers already store the desired values.**

In this case, Rs1 already stores the address to be read from.
```
Fault
Lr_d::Lr_dMicro::initiateAcc(ExecContext *xc,
    Trace::InstRecord *traceData) const
{
    // declare vars
    Addr EA;
    uint64_t Rs1 = 0;
    int64_t Mem = {};
    ;

    // this function is defined in src/cpu/simple/exec_context.hh
    // It reads the value stored in Rs1 register, which is the address to be read
    Rs1 = xc->readIntRegOperand(this, 0);
    ;
    EA = Rs1;;

    return initiateMemRead(xc, traceData, EA, Mem, memAccessFlags);
}


Fault
Lr_d::Lr_dMicro::completeAcc(PacketPtr pkt,
    ExecContext *xc, Trace::InstRecord *traceData) const
{
    int64_t Rd = 0;
    int64_t Mem = {};
    ;
    ;

    // defined in src/arch/generic/memhelpers.hh
    // getMemLE() -> getMem<ByteOrder::little>(pkt, Mem, traceData)
    // getMem(PacketPtr pkt, MemT &mem, Trace::InstRecord *traceData)
    //     -> mem = pkt->get<MemT>(Order);
    //     -> if (traceData) traceData->setData(mem);
    getMemLE(pkt, Mem, traceData);

    Rd = Mem;
    ;

    {
        int64_t final_val = Rd;
        xc->setIntRegOperand(this, 0, final_val);

        if (traceData) { traceData->setData(final_val); }
    };

    return NoFault;
}
```


+ Need to implement a function in src/mem/packet.hh and src/mem/packet.cc that returns a 512-bit value. Current API (getMemLe()) only supports 1, 2, 4 or 8 bytes (at most uint64_t).

+ In file src/mem/request.hh: ```enum:FlagsType``` - the ```mem_flags``` field within each instruction of decoder.isa. Our ```ld_64``` is a cached load, but ```sc_64``` is a uncached store. There are three flags that might be useful: 
    + ```UNCACHEABLE```: The request is to an uncacheable address.
    + ```STRICT_ORDER```: The request is required to be strictly ordered by CPU models and is non-speculative.
    + ```LLSC```: The request is a Load locked/store conditional.


Call graph:
```
TimingSimpleCPU::advanceInst()->
TimingSimpleCPU::fetch() ->
    thread->mmu->translateTiming() - do address translation


TimingSimpleCPU::completeIfetch() (src/cpu/simple/timing.cc)->
    preExecute()
    **initiateAcc()** -> initiateMemRead() -- does address translation
TimingSimpleCPU::finishTranslation()
    sendData() or sendSplitData()
        handleReadPacket() or handleWritePacket()

TimingSimpleCPU::DcachePort::recvTimingResp()


TimingSimpleCPU::completeDataAccess()
    **completeAcc()**
    postExecute(): update stats, TODO: needs add ENQCMD stats in base.cc
    advanceInst()
```


Corresponding generated cpp code:
```
Fault
Lr_64::Lr_64Micro::initiateAcc(ExecContext *xc,
    Trace::InstRecord *traceData) const
{
    // declare vars
    Addr EA;
    uint64_t Rs1 = 0;
    uint64_t Mem[8] = {};
    ;

    // It reads the value stored in Rs1 register, which is the address to be read
    Rs1 = xc->readIntRegOperand(this, 0);
    EA = Rs1;

    return initiateMemRead(xc, traceData, EA, mem, memAccessFlags);
}


Fault
Lr_64::Lr_64Micro::completeAcc(PacketPtr pkt, 
    ExecContext *xc, Trace::InstRecord *traceData) cost
{
    // declare vars

}
```


To add a new register class, need to modify the following files:

+ **All operate on ```std::array<uint8_t, 64>``` type**.

+ Add the real register
    + src/cpu/simple_thread.hh: in class SimpleThread.
+ Add the Operand type (possibly only for o3 cpu) (not necessary)
    + src/arch/isa_parser/operand_types.py
+ Implementation of function:
    + src/cpu/checker/cpu.hh: read/setS12RegOperand()
    + src/cpu/exec_context.hh: function definition
    + src/cpu/simple/exec_context.hh: read/setS12RegOperand()
        + src/cpu/simple_thread.hh: read/setS12Reg()
    + src/cpu/minor/exec_context.hh: read/setS12RegOperand()
    + src/cpu/o3/dyn_inst.hh: only the function prototypes due to virtual function definition in src/cpu/exec_context.hh

To see the result of instruction:
```
./build/RISCV/gem5.opt --debug-flags=S12Regs ./configs/learning_gem5/part1/two_level.py ./tests/test-progs/hello/bin/riscv/linux/hello
```

To add a new instruction, need to modify the following files:
+ Decoder file
    + src/arch/riscv/isa/decoder.isa
+ Instruction formats
    + If it is a memory reference instruction, needs to define 
        + templates: initiateAcc(), completeAcc() and execute().
        + format of the instruction
        + e.g. src/arch/riscv/isa/formats/amo.isa
+ Instruction operands
    + Given an instruction, we need to know the instruction operands, e.g. register(s) bits, including rs1, rs2 and rd. 
    + src/arch/riscv/insts/amo.hh
        + Define the instruction class, as well as ```generateDisassembly()```.
    + src/arch/riscv/insts/amo.cc
        + Implementation of ```generateDisassembly()```.

Error:
```
/home/yusen/gem5/build/RISCV/base/cprintf_formats.hh:100: 
multiple definition of `gem5::cp::operator<<(std::ostream&, std::array<unsigned char, 64ul>)'; 

build/RISCV/systemc/utils/sc_report.o:
/home/yusen/gem5/build/RISCV/base/cprintf_formats.hh:100: first defined here
```
Solution: ```static inline``` when defining the operator ```<<``` overloading



Store-conditional instruction call graph:

```StoreCondInitiateAcc()```:
+ ```writeMemTimingLE()```: src/arch/generic/memhelpers.hh
    + ```writeMemTiming<ByteOrder::little>()``` -> ```writeMemTiming()```
        + ```(ExecContext *)xc->writeMem()```: src/cpu/simple/timing.cc
            + ```WholeTranslationState()```
            + ```DataTranslation<TimingSimpleCPU *>()```
            + ```thread->mmu->translateTiming()```

+ ```finishTranslation()```: src/cpu/simple/timing.cc
    + ```sendData()```: src/cpu/simple/timing.cc
        + if read access, then ```handleReadPacket(pkt)```
        + else (write access), then
            + ```do_access = handleLockedWrite()```: src/arch/riscv/isa.cc
                + If request is uncacheable, then ```req->setExtraData(2)```, return true
            + if ```do_access == true```, then ```handleWritePacket()``` 



src/arch/riscv/regs/int.hh
mem_flags: not LLSC