/*
Armator - simulateur de jeu d'instruction ARMv5T � but p�dagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique G�n�rale GNU publi�e par la Free Software
Foundation (version 2 ou bien toute autre version ult�rieure choisie par vous).

Ce programme est distribu� car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but sp�cifique. Reportez-vous � la
Licence Publique G�n�rale GNU pour plus de d�tails.

Vous devez avoir re�u une copie de la Licence Publique G�n�rale GNU en m�me
temps que ce programme ; si ce n'est pas le cas, �crivez � la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
�tats-Unis.

Contact: Guillaume.Huard@imag.fr
	 B�timent IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'H�res
*/
#include "arm_instruction.h"
#include "arm_exception.h"
#include "arm_data_processing.h"
#include "arm_load_store.h"
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"


static int arm_execute_instruction(arm_core p) {
	uint32_t ins;
	arm_fetch(p, &ins);
	int function_to_use;
	function_to_use = (ins>>25)&7;
	switch(function_to_use){
	case 0:
		if(get_bit(ins, 24)==1 && get_bit(ins, 23)==0 && get_bit(ins, 20)==0){
			return arm_miscellaneous(p, ins);
		}
		if(get_bit(ins, 4)==1 && get_bit(ins, 7)==1){
			return arm_load_store(p, ins);
		}
		return arm_data_processing_shift(p, ins);
	case 1: // 001
		if(get_bit(ins, 24)==1 && get_bit(ins, 23)==0 && get_bit(ins, 21)==0 && get_bit(ins, 20)==0){
			return UNDEFINED_INSTRUCTION;
		}
		return arm_data_processing_immediate_msr(p, ins);
	case 2: //01I avec I=0
		return arm_load_store(p, ins);
	case 3: //01I avec I=1
		if(get_bits(ins, 24, 20)==0x1F && get_bits(ins, 7, 4)==0xF){
			return UNDEFINED_INSTRUCTION;
		}
		return arm_load_store(p, ins);
	case 4: // 100
		return arm_load_store_multiple(p, ins);
	case 5: // 101
		return arm_branch(p, ins);
	case 6: // 110
		return arm_coprocessor_load_store(p, ins);
	case 7: // 111
		return arm_coprocessor_others_swi(p, ins);//On ajoutera ici le gestionnaire d'interruptions
	default:
		return -1;
	}
}

int arm_step(arm_core p) {
    int result;

    result = arm_execute_instruction(p);
    if (result) {
        return arm_exception(p, result);
    }
    return result;
}
