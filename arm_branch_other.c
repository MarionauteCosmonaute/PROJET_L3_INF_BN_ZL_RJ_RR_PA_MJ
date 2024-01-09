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
    if (cond_not_respect(p,ins)){return -1;}
    uint8_t bit_l = get_bit(ins,24);
    uint8_t mode = registers_get_mode(p->reg);
    uint32_t signed_immed_24 = get_bits(ins,23,0);
    if((bit_l) | (get_bits(ins,31,28) == 15)){
        arm_write_register(p, 14,arm_read_register(p, 15) + 4);
    }
    if (get_bits(ins,31,28) == 15){
        arm_write_register(p, 16,set_bit(arm_read_register(p, 16),5));      //le bit T de CPSR prend la valeur 1
        arm_write_register(p, 15,arm_read_register(p, 15) + (signed_immed_24 << 2) + (bit_l << 1));
    }
    else{
        arm_write_register(p, 15,arm_read_register(p, 15) + (signed_immed_24 << 2));
    }
    return 0;
}

int arm_coprocessor_others_swi(arm_core p, uint32_t ins) {
    if (get_bit(ins, 24)) {
        return SOFTWARE_INTERRUPT;
    }
    return UNDEFINED_INSTRUCTION;
}


int arm_miscellaneous(arm_core p, uint32_t ins) {
    if (cond_not_respect(p,ins)){
        uint32_t valeur;
        if(get_bit(ins,25)){        // fonction MSR A4-76
            
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