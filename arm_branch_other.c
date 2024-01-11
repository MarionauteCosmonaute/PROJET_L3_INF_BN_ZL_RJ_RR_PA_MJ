/*
Armator - simulateur de jeu d'instruction ARMv5T à but pédagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique Générale GNU publiée par la Free Software
Foundation (version 2 ou bien toute autre version ultérieure choisie par vous).

Ce programme est distribué car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but spécifique. Reportez-vous à la
Licence Publique Générale GNU pour plus de détails.

Vous devez avoir reçu une copie de la Licence Publique Générale GNU en même
temps que ce programme ; si ce n'est pas le cas, écrivez à la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
États-Unis.

Contact: Guillaume.Huard@imag.fr
	 Bâtiment IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'Hères
*/
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"
#include "arm_core.h"
#include <debug.h>
#include <stdlib.h>


int arm_branch(arm_core p, uint32_t ins) {
    if (cond_not_respect(p,ins) == 0 || cond_not_respect(p,ins) == 2 ){
        uint8_t bit_l = get_bit(ins,24);
        uint32_t signed_immed_24 = get_bits(ins,23,0);
        if get_bit(ins,23){signed_immed_24=set_bits(signed_immed_24, 30, 24, 0xFF);}
        if((bit_l) | (get_bits(ins,31,28) == 15)){
            arm_write_register(p, 14,arm_read_register(p, 15));
        }
        if (get_bits(ins,31,28) == 15){
            arm_write_register(p, 16,set_bit(arm_read_register(p, 16),5));      //le bit T de CPSR prend la valeur 1
            arm_write_register(p, 15,arm_read_register(p, 15) + (signed_immed_24 << 2) + (bit_l << 1) - 4);
        }
        else{
            arm_write_register(p, 15,arm_read_register(p, 15) + (signed_immed_24 << 2) - 4);
        }
        return 0;
    }
    return -1;
}

int arm_coprocessor_others_swi(arm_core p, uint32_t ins) {
    if (get_bit(ins, 24)) {
        return SOFTWARE_INTERRUPT;
    }
    return UNDEFINED_INSTRUCTION;
}


int arm_miscellaneous(arm_core p, uint32_t ins) {
    if (cond_not_respect(p,ins) == 0){
        if(!(get_bit(ins,27)) & !(get_bit(ins,26)) & get_bit(ins,24) & !(get_bit(ins,23)) & get_bit(ins,21) & !(get_bit(ins,20))){ // a changer je ne connais pas les autre OPCODE
            return fonction_MSR(p,ins);
        }
        if (get_bits(ins,27,20) == 18){
            return fonction_BX(p,ins);
        }
    }
    return UNDEFINED_INSTRUCTION;
}

/*
if (get_bits(ins,27,20) == 18){ // fonction BX A4-20
        if(get_bit(ins,0)) {arm_write_register(p, 16,clr_bit(arm_read_register(p, 16),5));} //bit T de CPSR prend la valeur du bit de poids faible de l'ins
        else{arm_write_register(p, 16,set_bit(arm_read_register(p, 16),5));
        return 0;
        }
    }
    else if((ins,27,20) == 18){ // fonction SWP A4-212 
*/

int fonction_BX(arm_core p, uint32_t ins){
    if(get_bit(ins,0)) {arm_write_register(p, 16,clr_bit(arm_read_register(p, 16),5));} //bit T de CPSR prend la valeur du bit de poids faible de l'ins
    else{arm_write_register(p, 16,set_bit(arm_read_register(p, 16),5));}
    arm_write_register(p,15,(arm_read_register(p, get_bits(ins,3,0))) & 0xFFFFFFFE);
    return 0;
}

int fonction_MSR(arm_core p, uint32_t ins){
    uint32_t operand;
    uint32_t byte_mask = 0;
    uint32_t mask;
    if(get_bit(ins,25)){        // fonction MSR A4-76
        uint32_t r = get_bits(ins,7,0);
        uint32_t val = get_bits(ins,11,8) * 2;
        operand = (r>>val) | (r<<(32-val));
    }
    else{
        operand = arm_read_register(p,get_bits(ins,3,0));
    }
    if((operand & UnallocMask) != 0){return DATA_ABORT;} // pas sur de ce return (raison du return:  "Attempt to set reserved bits")
    if(get_bit(ins,16)){byte_mask |= 0x000000FF;}
    if(get_bit(ins,17)){byte_mask |= 0x0000FF00;}
    if(get_bit(ins,18)){byte_mask |= 0x00FF0000;}
    if(get_bit(ins,19)){byte_mask |= 0xFF000000;}
    if(!(get_bit(ins,22))){ /*verification du bit R == 0 (n22)*/
        if(arm_in_a_privileged_mode(p)){
            if((operand & StateMask) != 0){return DATA_ABORT;} /* Attempt to set non-ARM execution state */
            else{mask = byte_mask & (UserMask | PrivMask);}
        }
        else{mask = byte_mask & UserMask;}
        arm_write_cpsr(p,((arm_read_cpsr(p) & !(mask)) | (operand & mask)));
        return 0;
    }
    else{                   /*R == 1*/
        if (arm_current_mode_has_spsr(p)){
            mask = byte_mask & (UserMask | PrivMask | StateMask);
            arm_write_spsr(p,((arm_read_spsr(p) & !(mask)) | (operand & mask)));
            return 0;
        }
        else{return DATA_ABORT;}
    }
}