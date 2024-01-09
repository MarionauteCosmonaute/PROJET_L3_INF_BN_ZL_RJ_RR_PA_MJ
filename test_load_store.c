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
    memory m;
    arm_core a;
	uint32_t tmp[15];
    srandom(getpid());
	m = memory_create(0x8000);//memoire allouee
	r = registers_create();
	registers_write_cpsr(r, 0x1d3);
	uint8_t mode = registers_get_mode(r);
	registers_write(r, 15, mode, 0);
	for(int i = 0; i < 15; i++) {
		tmp[i] = random(); 
		registers_write(r, i, mode, tmp[i]);
	}

	uint32_t ins;
	uint16_t offset;
	uint32_t adresse;
	int result;
	uint32_t value;

	uint32_t memoire;
	a = arm_create(r, m);
	for(int j = 0; j<6; j++){
		arm_write_word(a, j*4,(uint32_t)j<<25);
		arm_read_word(a, j*4, &memoire);
		printf("Memoire : %d\nA l'adresse : %d\n", memoire, j*4);

		switch(j){
			case 0: //LDR and LDRB with immediate offset
				offset = (random()%(1<<10)) * 4;
			//word
				printf("LDR with immediate offset, checking value with pre_indexing\n");
				ins = 0xE5956000 | offset;  // P=1 et W=0
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				result = arm_read_word(a, adresse + offset, &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)));
				
				printf("LDR with immediate offset, checking if the base register's address is updated\n");
				ins = 0xE5B56000 | offset;  // P=1 et W=1
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				result = arm_read_word(a, adresse + offset, &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)) && arm_read_register(a, get_bits(ins, 19, 16)) == adresse + offset);
				
				printf("LDR with immediate offset, checking value with post_indexing\n");
				ins = 0xE4956000 | offset;  // P=0 et W=0
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				result = arm_read_word(a, adresse, &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)));

				printf("LDR with immediate offset, checking for instructions LDRBT, LDRT, STRBT or STRT not used\n");
				ins = 0xE4B56000 | offset;  // P=0 et W=1
				print_test(arm_load_store(a, ins) == UNDEFINED_INSTRUCTION);

			//byte
				printf("LDRB with immediate offset, checking value with pre_indexing\n");
				ins = 0xE5D56000; //p=1 et w=0
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				result = arm_read_byte(a, adresse, &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)));
                break;

			
			case 1:  //LDR and LDRB with register offset without shift			
				offset = (random()%(1<<30)) * 4;
			//word
				printf("LDR with register offset with shift = 0, checking value with pre_indexing\n");
				ins = 0xE7956004; //pour faciliter les calculs, pas de décalage, car dépend d'une fonction à part
				arm_write_register(a, get_bits(ins, 3, 0), offset);
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				result = arm_read_word(a, adresse + arm_read_register(a,get_bits(ins, 3, 0)), &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)));
					
			//byte
				printf("LDRB with register offset with shift = 0, checking value with pre_indexing\n");
				ins = 0xE7D56004;
				arm_write_register(a, get_bits(ins, 3, 0), offset);
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				result = arm_read_byte(a, adresse + arm_read_register(a,get_bits(ins, 3, 0)), &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)));
                break;
			
			
			
			case 2:  // STR and STRB with immediate offset
				offset = (random()%(1<<10)) * 4;
			//word
				printf("STR with immediate offset, checking value with pre_indexing\n");
				ins = 0xE5856000 | offset; //P=1 et W=0
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				result = arm_read_word(a, adresse + offset, &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)));
				
			//byte
				printf("STRB with immediate offset, checking value with pre_indexing\n");
				ins = 0xE5C56000;
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				result = arm_read_byte(a, adresse + offset, &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)));

                break;
			
			
			
			case 3:  // STR and STRB with register offset without shift
				offset = (random()%(1<<30)) * 4;
				//word
				printf("STR with register offset with shift = 0, checking value with pre_indexing\n");
				ins = 0xE7856004; 
				arm_write_register(a, get_bits(ins, 3, 0), offset);
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				result = arm_read_word(a, adresse + arm_read_register(a,get_bits(ins, 3, 0)), &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)));
				
				//byte
				printf("STRB with register offset with shift = 0, checking value with pre_indexing\n");
				ins = 0xE7C56004;
				arm_write_register(a, get_bits(ins, 3, 0), offset);
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				result = arm_read_byte(a, adresse + arm_read_register(a,get_bits(ins, 3, 0)), &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)));

                break;
			
			
			
			case 4:
				// LDRH with immediate offset
				uint8_t immedH = ((random()%(1<<2)) * 4);
				uint8_t immedL = (random()%(1<<2)) * 4;
				printf("LDMH with immediate offset, checking value with pre_indexing\n");
				ins = 0xE1D560B0 | (immedH<<8) | immedL;
				offset = (immedH << 4) | immedL;
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				result = arm_read_half(a, adresse + offset, &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)));

				
				// LDRH with register offset
				offset = ((random()%(1<<30)) * 4);
				printf("LDMH with register offset, checking value with pre_indexing\n");
				ins = 0xE19560B4;
				arm_write_register(a, get_bits(ins, 3, 0), offset);
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				result = arm_read_half(a, adresse + arm_read_register(a,get_bits(ins, 3, 0)), &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)));
				
                break;


			case 5: 
				// STRH with immediate offset
				uint8_t immedH = ((random()%(1<<2)) * 4);
				uint8_t immedL = (random()%(1<<2)) * 4;
				printf("STMH with immediate offset, checking value with pre_indexing\n");
				ins = 0xE1C560B0 | (immedH<<8) | immedL;
				offset = (immedH << 4) | immedL;
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				result = arm_read_half(a, adresse + offset, &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)));

				//STRH with register offset
				printf("STMH with register offset, checking value with pre_indexing\n");
				ins = 0xE18560B4;
				arm_write_register(a, get_bits(ins, 3, 0), offset);
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				result = arm_read_half(a, adresse + arm_read_register(a,get_bits(ins, 3, 0)), &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)));
                break;
			/*
				
			case 6:
				uint16_t register_list = (random()%(1<<16)) & (~(1<<15));
                //cond 100P U0WL Rn xxxx(reg list, reg à 1 si actif, ne met pas tout à zero)
				// Case 10: LDM(1)
				printf("LDM(1) with base register not updated");
				ins = 0xE8950000 | register_list; //base reg pas update
				

				printf("LDM(1) with base register updated");
				ins = 0xE8BRnxxxx; //base reg update, r. The base register is incremented (U==1) by four times the number of registers in the register list
				print_test(arm_coprocessor_others_swi(a, ins)==arm_execute_instruction(a));
                break;
           
		   
		    case 7:
                // Case 11: STM(1) 
				printf("STM(1) with base register not updated");
				ins = 0xE88Rnxxxx; //pas d'update
				print_test(arm_coprocessor_others_swi(a, ins)==arm_execute_instruction(a));
                break;
			
			*/
			default:
				printf("default");
				break;
		}
    }
    return 0;
}



/*

// Case 8: STRH with immediate offset
//cond 0001 U1WL Rn Rd immedH 1SH1 ImmedL
ins = 0xE1CxximmedHBimmedL; 

// Case 9: STRH with register offset
//cond 0001 U0WL Rn Rd 0000 1SH1 Rm
ins = 0xE18xx0Bx; 


///// load store multiple /////

//cond 100P U0WL Rn xxxx(reg list, reg à 1 si actif, ne met pas tout à zero)
// Case 10: LDM(1)
ins = 0xE89Rnxxxx; //base reg pas update
ins = 0xE8BRnxxxx; //base reg update, r. The base register is incremented
(U==1) by four times the number of registers in the register list


// Case 11: STM(1)
ins = 0xE88Rnxxxx; //pas d'update
*/