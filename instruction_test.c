#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "registers.h"
#include "arm_data_processing.h"
#include "arm_load_store.h"
#include "arm_branch_other.h"
#include "arm_instruction.h"
#include "arm_core.h"
#include "util.h"
#include "arm_constants.h"

void print_test(int result) {
    if (result)
        printf("Test succeded\n");
    else
        printf("TEST FAILED !!\n");
}

int main() {
	registers r;
	memory m;
	arm_core a;
	uint32_t tmp[15];
	srandom(getpid());
	m = memory_create(0x8000); //Memoire allouée
	r = registers_create();
	registers_write_cpsr(r, 0x1d3);
	uint8_t mode = registers_get_mode(r);
	registers_write(r, 15, mode, 0);
	for (int i = 0; i < 15; i++) {
		tmp[i] = random(); 
		registers_write(r, i, mode, tmp[i]);
	}
	uint32_t memoire;
	a = arm_create(r, m);
	for(int j = 0; j<8; j++){
		arm_write_word(a,j*4,(uint32_t)j<<25);
		arm_read_word(a,j*4,&memoire);	
		printf("Memoire : %d\nA l'adress : %d\n",memoire,j*4);
		switch(j){
		case 0:
			printf("Cas 0 :\n");
			print_test(arm_data_processing_shift(a, 0) == arm_execute_instruction(a));
			break;
		case 1:
			printf("Cas 1:\n");
			print_test(arm_data_processing_immediate_msr(a, 0) == arm_execute_instruction(a));
			break;
		case 2:
			printf("Cas 2:\n");
			print_test(arm_load_store(a, 0) == arm_execute_instruction(a));
			break;
		case 3:
			printf("Cas 3:\n");
			print_test(arm_load_store(a, 0) == arm_execute_instruction(a));
			break;
		case 4:
			printf("Cas 4:\n");
			print_test(arm_load_store_multiple(a, 0) == arm_execute_instruction(a));
			break;
		case 5:
			printf("Cas 5:\n");
			print_test(arm_branch(a, 0) == arm_execute_instruction(a));
			break;
		case 6:
			printf("Cas 6:\n");
			print_test(arm_coprocessor_load_store(a, 0) == arm_execute_instruction(a));
			break;
		case 7:
			printf("Cas 7:\n");
			print_test(arm_coprocessor_others_swi(a, 0) == arm_execute_instruction(a));
			break;
		default:
			printf("default");
			break;
		}
	}
	arm_destroy(a);
	registers_destroy(r);
	memory_destroy(m);
	return 0;
}

