#include "arm_load_store.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "util.h"
#include "debug.h"
#include <stdlib.h>

int main(){
    memory m = memory_create(0x8000);
    registers reg = registers_create();
    arm_core a;





    registers_destroy(reg);
    memory_destroy(m);
}