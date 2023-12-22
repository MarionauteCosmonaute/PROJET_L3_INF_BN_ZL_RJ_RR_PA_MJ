#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "registers.h"
#include "arm_constants.h"
#include "util.h"
#include "arm_core.c"

void print_test(int result) {
    if (result)
        printf("Test succeded\n");
    else
        printf("TEST FAILED !!\n");
}

int main() {
	registers r;
	uint32_t test[15];

    srandom(getpid());
	for(int j = 0; i<8; i++){
		arm_core a;
		m = memory_create(10); //Memoire allouée
		r = registers_create();
		registers_write_cpsr(r, 0x1d3);
		uint8_t mode = registers_get_mode(r);
		for (int i = 0; i < 16; i++) {
			test[i] = random(); 
			if(i == 15){
				test[15] = (test[15]&~(7<<25))|(j<<25);
			}
			registers_write(r, i, mode, test[i]);
		}
		a = arm_create(r, m);
		uint32_t ins = (a->reg)->registre[15];
		switch(j){
			case 0:
				return print_test(arm_data_processing_shift(a, ins)==arm_execute_instruction(a));
			case 1:
				return print_test(arm_data_processing_immediate_msr(a, ins)==arm_execute_instruction(a));
			case 2:
				return print_test(arm_load_store(a, ins)==arm_execute_instruction(a));
			case 3:
				return print_test(arm_load_store(a, ins)==arm_execute_instruction(a));
			case 4:
				return print_test(arm_load_store_multiple(a, ins)==arm_execute_instruction(a));
			case 5:
				return print_test(arm_branch(a, ins)==arm_execute_instruction(a));
			case 6:
				return print_test(arm_coprocessor_load_store(a, ins)==arm_execute_instruction(a))
			case 7:
				return print_test(arm_coprocessor_others_swi(a, ins)==arm_execute_instruction(a));
			default:
				return -1;
		}
		}
    }
}

