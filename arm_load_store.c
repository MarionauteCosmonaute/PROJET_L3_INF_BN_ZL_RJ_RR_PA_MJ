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
#include "arm_load_store.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "util.h"
#include "debug.h"

/*
load : lecture d'une adresse dans la mémoire et on le stocke dans un reg
-> read byte/word puis write reg

store : on lit dans un reg et on le stocke dans une adresse
-> read reg et write byte/word


L load 1 / store 0
B byte 1 / word 0
25 immediat 0 / dans reg 1
U add 1 / sub 0
P 0 : W 0 R0 = Mem[R1] puis R1 = R1 + offset
      W 1 unprivileged acces ???
  1 : W 0 R0 = Mem[R1]
      W 1 R1 = R1 + offset puis R0 = Mem[R1]


LDR R0, [R1, #4]

R0 = reg source dest
R1 = reg base
*/


int arm_load_store(arm_core p, uint32_t ins) {
    int p = get_bit(ins, 24);
    int u = get_bit(ins, 23);
    int b = get_bit(ins, 22); //bit B : byte ou word
    int w = get_bit(ins, 21);
    int l = get_bit(ins, 20); // 0 store, 1 load
    int immediat = get_bit(ins, 25);
    uint8_t reg_base = get_bits(ins, 19, 16);
    uint8_t reg_source_dest = get_bits(ins, 15, 12);
    uint32_t offset;
    uint32_t adresse;
    uint32_t *value;
    if(immediat == 0){
        offset = get_bits(ins, 11, 0);
    }
    else{
        uint8_t rm = get_bits(ins, 3, 0);
        offset = (p->reg)->registre[rm];
    }

    // instruction p->reg pas bonne, vérif la structure
    if (p == 0 && w == 0){
        adresse = (p->reg)->registre[reg_base];
        if (l){ //load
            if(b == 1)
            {
                arm_read_byte(p, adresse, (uint8_t *) value);
            }
            else{
                arm_read_word(p, adresse, value);
            }
            arm_write_register(p, reg_source_dest, *value);
        }
        else { //store
            arm_read_register(p, reg_source_dest);
            if(b == 1)
            {
               arm_write_byte(p, adresse, value); 
            }
            else{
               arm_write_word(p, adresse, value);
            }
        }
        if (u == 1){
            adresse = adresse + offset;
        }
        else{
            adresse = adresse - offset;
        } 


    }
    else if(p == 0 && w == 1){
        //?????
    }
    else if(p == 1 && w == 0){
        adresse = (p->reg)->registre[reg_base];
        if (l){ //load
            if(b == 1)
            {
                arm_read_byte(p, adresse, (uint8_t *) value);
            }
            else{
                arm_read_word(p, adresse, value);
            }
            arm_write_register(p, reg_source_dest, *value);
        }
        else { //store
            arm_read_register(p, reg_source_dest);
            if(b == 1)
            {
               arm_write_byte(p, adresse, value); 
            }
            else{
               arm_write_word(p, adresse, value);
            }
        }
    }
    else { // p == 1 et w == 1)
        adresse = (p->reg)->registre[reg_base];
        if (u == 1){
            adresse = adresse + offset;
        }
        else{
            adresse = adresse - offset;
        } 
        if (l){ //load
            if(b == 1)
            {
                arm_read_byte(p, adresse, (uint8_t *) value);
            }
            else{
                arm_read_word(p, adresse, value);
            }
            arm_write_register(p, reg_source_dest, *value);
        }
        else { //store
            arm_read_register(p, reg_source_dest);
            if(b == 1)
            {
               arm_write_byte(p, adresse, value); 
            }
            else{
               arm_write_word(p, adresse, value);
            }
        }
    }


    return UNDEFINED_INSTRUCTION;
}

int arm_load_store_multiple(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}

int arm_coprocessor_load_store(arm_core p, uint32_t ins) {
    /* Not implemented */
    return UNDEFINED_INSTRUCTION;
}
