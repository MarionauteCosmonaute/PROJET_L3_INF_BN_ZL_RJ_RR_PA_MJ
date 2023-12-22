#include <stdlib.h>
#include <stdio.h>
#include "arm_data_processing.h"
#include "arm_core.h"

int main(){

    memory mem = memory_create(0x200);
    registers reg = registers_create();
    arm_core p = arm_create(reg,mem);
    


    return 0;
}