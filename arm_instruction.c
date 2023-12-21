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

int execute_instruction(arm_core p ){
	arm_execute_instruction(p);
}

static int arm_execute_instruction(arm_core p) {
	uint32_t adr=0;
	arm_fetch(p, &adr);
	uint32_t ins ;
	arm_read_word(p,adr,&ins);
	ins=(ins>>25)&7;// 
	switch(ins){
	case 0: // 000
		return arm_data_processing_shift(p, adr);
	case 1: // 001
		return arm_data_processing_immediate_msr(p, adr);
	case 2: //01I avec I=0
		return arm_load_store(p, adr);
	case 3: //01I avec I=1
		return arm_load_store(p, adr);
	case 4: // 100
		return arm_load_store_multiple(p, adr);
	case 5: // 101
		return arm_branch(p, adr);
	case 6: // 110
		return arm_coprocessor_load_store(p, adr);
	case 7: // 111
		return arm_coprocessor_others_swi(p, adr);
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
