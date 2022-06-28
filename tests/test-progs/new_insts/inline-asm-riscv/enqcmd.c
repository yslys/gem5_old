/**
 * ENQCMD = lr_q + sc_q
 * lr_q: load value at address Rs1 into a temporary register (s12Reg).
 *       We only need to specify Rs1 and Rd.
 *          rs1=a6/x16, rd=s6/x22
 *       We can check the result of lr_q by checking value of rd register
 *
 * sc_q: store the value in temporary register (s12Reg) to address Rs1
 *       We only need to specify Rs1.
 */
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

static inline uint64_t lr_q(void *src) {
    /* obtain the address of the 64-byte value */
    uint64_t addr = (uint64_t)src;

    /* load value of @addr into register a6 */
    __asm__ volatile("ld a6, %0"::"m" (addr));

    /* invoke lr_q instruction, with rs1 = a6(x16), rd = s6(x22) */
    __asm__ volatile(".byte 0x2f, 0x4b, 0x08, 0x16");

    /* check the result of lr_q by checking rd (s6/x22) */
    uint64_t ret = 2;
    __asm__ volatile("sd s6, %0": "=m" (ret));

    printf("[lr_q]: retval of lr_q: %lu\n", ret);

    /* store the value in a6 into return val */
    /*
    uint64_t ret = 0;
    __asm__ volatile("sd a6, %0"
                     : "=m" (ret));
    printf("%lu\n", ret);
    */
    // print decimal value of the address
    printf("[lr_q]: decimal value of addr: %lu\n", addr);
    printf("[lr_q]: hex value of src: %p\n", src);
    return ret;
}

static inline void sc_q(void *dst) {
    /* obtain the destination address to be written to */
    uint64_t addr = (uint64_t)dst;

    /* load value of destination address @addr into register a6 */
    __asm__ volatile("ld a6, %0"::"m" (addr));

    /* invoke sc_q instruction, */
    __asm__ volatile(".byte 0x2f, 0x4b, 0x08, 0x1e");

    /* check the result of sc_q by checking rd (s6/x22) */
    uint64_t ret = 4;
    __asm__ volatile("sd s6, %0": "=m" (ret));
    printf("[sc_q]: retval of sc_q: %lu\n", ret);
    printf("[sc_q]: decimal value of addr: %lu\n", addr);
    printf("[sc_q]: hex value of dst: %p\n", dst);


}

static inline void fence() {
        __asm__ volatile("":::"memory");
}

static inline void enqcmd(void *src, void *dst) {

}


struct work_desc
{
    uint64_t pasid;
    uint8_t tmp[56]; // placeholder to make this struct 64-byte
};

/**
 * allocate a new work descriptor of 64 bytes
 */
struct work_desc *alloc_desc(uint64_t id)
{
    // allocate 64-byte of memory
    struct work_desc *src = memalign(4096, 64);
    if (!src) return NULL;

    src->pasid = id;
    // TODO: this only fills in the area with 3.
    memset(src->tmp, 3, sizeof(uint8_t)*56);
    return src;
}


int main() {
    uint64_t ret_lr = 4;
    uint64_t ret_sc = 4;

    struct work_desc *src = alloc_desc(5);
    struct work_desc *dst = alloc_desc(10);
    printf("[main]: source address: %p\n", src);
    printf("[main]: destination address: %p\n", dst);

    fence();
    lr_q((void *)src);
    sc_q((void *)dst);
    fence();

    printf("after sc_q: id should be five: %ld\n", dst->pasid);
    return 0;

}
