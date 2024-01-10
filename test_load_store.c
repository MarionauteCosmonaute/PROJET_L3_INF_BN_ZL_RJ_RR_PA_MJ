#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "registers.h"
#include "arm_constants.h"
#include "util.h"
#include "arm_data_processing.h"
#include "arm_load_store.h"

void print_test(int result) {
    if (result)
        printf("TEST SUCCEDED !\n");
    else
        printf("TEST FAILED !\n");
}

int main() {
	registers r;
    memory m;
    arm_core a;
	uint32_t tmp[15];
    srandom(getpid());
	m = memory_create(0xFFFFFFFF);//memoire allouee
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
	uint8_t value8;
	uint16_t value16;
	uint8_t immedH;
	uint8_t immedL;
	
	uint32_t memoire;
	a = arm_create(r, m);

	for(int j = 0; j<9; j++){
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
				arm_write_word(a, adresse + offset, 20); //test en ecrivant une valeur a l'adresse
				result = arm_load_store(a, ins);
				arm_read_word(a, adresse + offset, &value);
				printf("offset = %x\nresult = %d\nadresse = %x\nvalue = %x\n", offset, result, adresse, value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)) && (result == 0));
				
				printf("\nLDR with immediate offset, checking if the base register's address is updated\n");
				ins = 0xE5B56000 | offset;  // P=1 et W=1
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				arm_read_word(a, adresse + offset, &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)) && (arm_read_register(a, get_bits(ins, 19, 16)) == adresse + offset) && (result == 0));
				
				printf("\nLDR with immediate offset, checking value with post_indexing\n");
				ins = 0xE4956000 | offset;  // P=0 et W=0
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				arm_read_word(a, adresse, &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)) && (result == 0));

				printf("\nLDR with immediate offset, checking for instructions LDRBT, LDRT, STRBT or STRT not used\n");
				ins = 0xE4B56000 | offset;  // P=0 et W=1
				print_test(arm_load_store(a, ins) == UNDEFINED_INSTRUCTION);

			//byte
				printf("\nLDRB with immediate offset, checking value with pre_indexing\n");
				ins = 0xE5D56000; //p=1 et w=0
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				arm_read_byte(a, adresse, &value8);
				print_test(value8 == arm_read_register(a, get_bits(ins, 15, 12)) && (result == 0));
                break;

			
			case 1:  //LDR and LDRB with register offset without shift			
				offset = (random()%(1<<30)) * 4;
			//word
				printf("\nLDR with register offset with shift = 0, checking value with pre_indexing\n");
				ins = 0xE7956004; //pour faciliter les calculs, pas de décalage, car dépend d'une fonction à part
				arm_write_register(a, get_bits(ins, 3, 0), offset);
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				arm_read_word(a, adresse + arm_read_register(a,get_bits(ins, 3, 0)), &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)) && (result == 0));
					
			//byte
				printf("\nLDRB with register offset with shift = 0, checking value with pre_indexing\n");
				ins = 0xE7D56004;
				arm_write_register(a, get_bits(ins, 3, 0), offset);
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				arm_read_byte(a, adresse + arm_read_register(a,get_bits(ins, 3, 0)), &value8);
				print_test(value8 == arm_read_register(a, get_bits(ins, 15, 12)) && (result == 0));
                break;
			
			
			
			case 2:  // STR and STRB with immediate offset
				offset = (random()%(1<<10)) * 4;
			//word
				printf("\nSTR with immediate offset, checking value with pre_indexing\n");
				ins = 0xE5856000 | offset; //P=1 et W=0
				arm_write_register(a, 6, 7);
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				arm_read_word(a, adresse + offset, &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)) && (result == 0));
				
			//byte
				printf("\nSTRB with immediate offset, checking value with pre_indexing\n");
				ins = 0xE5C56000 | offset;
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				arm_read_byte(a, adresse + offset, &value8);
				print_test(value8 == arm_read_register(a, get_bits(ins, 15, 12)) && (result == 0));

                break;
			
			
			
			case 3:  // STR and STRB with register offset without shift
				offset = (random()%(1<<30)) * 4;
				//word
				printf("\nSTR with register offset with shift = 0, checking value with pre_indexing\n");
				ins = 0xE7856004; 
				arm_write_register(a, get_bits(ins, 3, 0), offset);
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				arm_read_word(a, adresse + arm_read_register(a,get_bits(ins, 3, 0)), &value);
				print_test(value == arm_read_register(a, get_bits(ins, 15, 12)) && (result == 0));
				
				//byte
				printf("\nSTRB with register offset with shift = 0, checking value with pre_indexing\n");
				ins = 0xE7C56004;
				//printf("Rd = %u\n", arm_read_register(a, 5));
				arm_write_register(a, get_bits(ins, 3, 0), offset);
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				arm_read_byte(a, adresse + arm_read_register(a,get_bits(ins, 3, 0)), &value8);
				//printf("value8 = %x \nreg = %x\n", value8, arm_read_register(a, get_bits(ins, 15, 12)));
				print_test(value8 == arm_read_register(a, get_bits(ins, 15, 12)) && (result == 0));
                break;
			
			
			
			case 4:
				// LDRH with immediate offset
				immedH = ((random()%(1<<2)) * 4);
				immedL = (random()%(1<<2)) * 4;
				printf("\nLDMH with immediate offset, checking value with pre_indexing\n");
				ins = 0xE1D560B0 | (immedH<<8) | immedL;
				offset = (immedH << 4) | immedL;
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				arm_read_half(a, adresse + offset, &value16);
				print_test(value16 == arm_read_register(a, get_bits(ins, 15, 12)) && (result == 0));

				
				// LDRH with register offset
				offset = ((random()%(1<<30)) * 4);
				printf("\nLDMH with register offset, checking value with pre_indexing\n");
				ins = 0xE19560B4;
				arm_write_register(a, get_bits(ins, 3, 0), offset);
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				arm_read_half(a, adresse + arm_read_register(a,get_bits(ins, 3, 0)), &value16);
				print_test(value16 == arm_read_register(a, get_bits(ins, 15, 12)) && (result == 0));
				
                break;


			case 5: 
				// STRH with immediate offset
				immedH = ((random()%(1<<2)) * 4);
				immedL = (random()%(1<<2)) * 4;
				printf("\nSTMH with immediate offset, checking value with pre_indexing\n");
				ins = 0xE1C560B0 | (immedH<<8) | immedL;
				offset = (immedH << 4) | immedL;
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				arm_read_half(a, adresse + offset, &value16);
				print_test(value16 == arm_read_register(a, get_bits(ins, 15, 12)) && (result == 0));

				//STRH with register offset
				printf("\nSTMH with register offset, checking value with pre_indexing\n");
				ins = 0xE18560B4;
				arm_write_register(a, get_bits(ins, 3, 0), offset);
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				result = arm_load_store(a, ins);
				arm_read_half(a, adresse + arm_read_register(a,get_bits(ins, 3, 0)), &value16);
				print_test(value16 == arm_read_register(a, get_bits(ins, 15, 12)) && (result == 0));
                break;
		
				
			case 6:
				// LDM(1)
				printf("\nLDM(1) with base register not updated\n");
				ins = 0xE89600A8; //base reg pas update, registres 3, 5 et 7 dans la liste
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				arm_write_word(a, adresse, 10);
				arm_write_word(a, adresse+4, 15);
				arm_write_word(a, adresse+8, 20);
				result = arm_load_store_multiple(a, ins);
				for(int reg_num = 0; reg_num < 15; reg_num++){
					if(get_bit(get_bits(ins, 15, 0), reg_num)){
						arm_read_word(a, adresse, &value);
						printf("value = %x\n", value);
						printf("adresse = %x\n", adresse);
						print_test(value == arm_read_register(a, reg_num));
						adresse += 4; //on incremente localement pour vérifier la mémoire
					}
				}
				printf("adresse finale = %x\n", arm_read_register(a, get_bits(ins, 19, 16)));
				
				printf("\nLDM(1) with base register updated\n");
				ins = 0xE8B600A8; //base reg update, registres 3, 5 et 7 dans la liste
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				arm_write_word(a, adresse, 5);
				arm_write_word(a, adresse+4, 10);
				arm_write_word(a, adresse+8, 12);
				result = arm_load_store_multiple(a, ins);
				for(int reg_num = 0; reg_num < 15; reg_num++){
					if(get_bit(get_bits(ins, 15, 0), reg_num)){
						//printf("reg = %x\n",arm_read_register(a, reg_num));
						arm_read_word(a, adresse, &value);
						//printf("num reg : %d\n", reg_num);
						printf("value = %x\n", value);
						printf("adresse = %x\n", adresse);
						print_test(value == arm_read_register(a, reg_num));
						adresse += 4; //on incremente localement pour vérifier la mémoire
					}
				}
				printf("adresse finale = %x\n", arm_read_register(a, get_bits(ins, 19, 16)));
				print_test(adresse == arm_read_register(a, get_bits(ins, 19, 16)));
				break;
           
		   
		    case 7:
                // STM(1) 
				printf("\nSTM(1) with base register not updated\n");
				ins = 0xE88600A8; //pas d'update
				adresse = arm_read_register(a, get_bits(ins, 19, 16));
				arm_write_register(a, 3, 10);
				arm_write_register(a, 5, 15);
				arm_write_register(a, 7, 20);
				result = arm_load_store_multiple(a, ins);
				for(int reg_num = 0; reg_num < 15; reg_num++){
					if(get_bit(get_bits(ins, 15, 0), reg_num)){
						value = arm_read_register(a, reg_num);
						printf("value = %x\n", value);
						printf("adresse = %x\n", adresse);
						print_test(value == arm_read_register(a, reg_num));
						adresse += 4;
					}
				}
                break;
			
			case 8 :

				printf("\nCas instructions non définies ou autorisées\n");
				printf("Dans load_store word/byte\n");
				ins = 0xE7956014; //bit 4 mis à 1
				print_test(arm_load_store(a, ins) == UNDEFINED_INSTRUCTION);
				
				ins = 0xE6B56004; //bits p=0 et w=1
				print_test(arm_load_store(a, ins) == UNDEFINED_INSTRUCTION);
				
				printf("Dans load_store half\n");
				immedH = ((random()%(1<<2)) * 4);
				immedL = (random()%(1<<2)) * 4;
				ins = 0xE1DF60B0 | (immedH<<8) | immedL; //Rn=15
				print_test(arm_load_store(a, ins) == UNDEFINED_INSTRUCTION);

				ins = 0xE1D560F0 | (immedH<<8) | immedL; //bit s=1
				print_test(arm_load_store(a, ins) == UNDEFINED_INSTRUCTION);

				ins = 0xE0F560B0 | (immedH<<8) | immedL; //p=0 et w=1
				print_test(arm_load_store(a, ins) == UNDEFINED_INSTRUCTION);
				
				printf("Dans load_store_multiple\n");
				ins = 0xE8860000; //list_reg = 0
				print_test(arm_load_store_multiple(a, ins) == UNDEFINED_INSTRUCTION);

				ins = 0xE8B60040; //si Rn (ici 6) dans list_reg et w=1
				print_test(arm_load_store_multiple(a, ins) == UNDEFINED_INSTRUCTION);

				ins = 0xE8C600A8; //bit s=1
				print_test(arm_load_store_multiple(a, ins) == UNDEFINED_INSTRUCTION);

				ins = 0xE8868000; //R15 dans list_reg
				print_test(arm_load_store_multiple(a, ins) == UNDEFINED_INSTRUCTION);
			
			
			default:
				printf("\ndefault\n");
				break;
		}
    }
	memory_destroy(m);
	registers_destroy(r);
	arm_destroy(a);
    return 0;
}