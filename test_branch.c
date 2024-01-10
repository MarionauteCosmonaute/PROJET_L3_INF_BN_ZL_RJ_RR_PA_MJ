#include <stdlib.h>
#include <stdio.h>
#include "arm_branch_other.h"

int main(){
    memory mem = memory_create(0x200);
    registers reg = registers_create();
    arm_core p = arm_create(reg,mem);
    arm_write_register(p,15,20);
    uint32_t ins = 0xEBFFFFFF; //0xEA00000000 pour L = 0
    arm_branch(p,ins);
    printf("%d\n",arm_read_register(p,15) - 4);
    memory_destroy(mem);
    registers_destroy(reg);
    arm_destroy(p);
    return 0;
}