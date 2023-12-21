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
#include <stdlib.h>

void read_write(int l, int b, arm_core p, uint32_t adresse, uint8_t Rd, uint32_t *value){
    if (l){ //load
        if(b == 1){
            arm_read_byte(p, adresse, (uint8_t *) value);
        }
        else{
            arm_read_word(p, adresse, value);
        }
        arm_write_register(p, Rd, *value);
    }
    else { //store
        arm_read_register(p, Rd);
        if(b == 1){
            arm_write_byte(p, adresse, *(uint8_t *)value); 
        }
        else{
            arm_write_word(p, adresse, *value);
         }
    }
}

//page 459
int arm_load_store(arm_core p, uint32_t ins) {
    if(cond_not_respect(p, ins)){
        return -1;
    }

    int p_bit = get_bit(ins, 24);
    int u = get_bit(ins, 23);
    int b = get_bit(ins, 22); //bit B : byte ou word
    int w = get_bit(ins, 21);
    int l = get_bit(ins, 20); // 0 store, 1 load
    int immediat = get_bit(ins, 25);
    uint8_t Rn = get_bits(ins, 19, 16);
    uint8_t Rd = get_bits(ins, 15, 12);
    uint32_t offset;
    uint32_t adresse = (p->reg)->registre[Rn];
    uint32_t *value = (uint32_t *)malloc(sizeof(uint32_t));
    
    if(immediat == 0){
        offset = get_bits(ins, 11, 0);
    }
    else{
        uint8_t Rm = get_bits(ins, 3, 0);
        offset = (p->reg)->registre[Rm];
    }

    if (u == 0){
        offset = -offset;
    }

    if (p_bit == 0 && w == 0){
        read_write(l, b, p, adresse, Rd, *value);
        adresse += offset;
        (p->reg)->registre[Rn] = adresse;
        return 0;
    }
    else if(p_bit == 0 && w == 1){
        if (l){ //load
            if(b == 1){
                arm_read_byte(p, adresse, (uint8_t *) value);
            }
            else{
                arm_read_word(p, adresse, value);
            }
            arm_write_usr_register(p, Rd, *value);
        }
        else { //store
            arm_read_usr_register(p, Rd);
            if(b == 1){
               arm_write_byte(p, adresse, *value); 
            }
            else{
               arm_write_word(p, adresse, *value);
            }
        }
        adresse += offset;
        (p->reg)->registre[Rn] = adresse;
        return 0;
    }
    else if(p_bit == 1 && w == 0){
        read_write(l, b, p, adresse, Rd, *value);
        return 0;
    }
    else{ // p == 1 et w == 1)
        adresse += offset; 
        (p->reg)->registre[Rn] = adresse;
        read_write(l, b, p, adresse, Rd, *value);
        return 0;
    }
    return UNDEFINED_INSTRUCTION;
}

//page 482
int arm_load_store_multiple(arm_core p, uint32_t ins) {
    if(cond_not_respect(p, ins)){
        return -1;
    }
    
    int p_bit = get_bit(ins, 24);
    int u = get_bit(ins, 23);
    int s = get_bit(ins, 22);
    int w = get_bit(ins, 21);
    int l = get_bit(ins, 20);
    uint8_t Rn = get_bits(ins, 19, 16);
    uint32_t adresse = (p->reg)->registre[Rn];
    uint32_t *value = (uint32_t *)malloc(sizeof(uint32_t)); 
    uint32_t val_pc;   
    
    if (l && s && ( registers_get_mode(p->reg) == SYS || registers_get_mode(p->reg) == USR)) {
        return UNDEFINED_INSTRUCTION;
    }

    int direction; // direction du transfert
    if (u) {
        direction = 1;  
    }
    else {
        direction = -1;
    }

    if ((p_bit == 0 && u == 0) || (p_bit == 1 && u == 1)) {
        adresse -= direction; // à verif j'ai pas hyper compris
    }

    int nbReg = 0;
    for(int i=0; i<1; i++){
        if (get_bit(ins,i) == 1){
            nbReg+=1;
        }
    }
    if (nbReg == 0){
        return UNDEFINED_INSTRUCTION;
    }

    int increment = 0; //calcule l'incrementation
    if (w) {
        increment = 4 * nbReg * direction;
    }

    /*
    Rajouter ce cas particulier :
    For LDMs that load the PC, the S bit indicates that the CPSR is loaded from the SPSR. For
    LDMs that do not load the PC and all STMs, the S bit indicates that when the processor is in a
    privileged mode, the User mode banked registers are transferred instead of the registers of
    the current mode.
    */
    for (int reg_num = 0; reg_num < 16; reg_num++) {
        if (get_bit(ins, reg_num)) {
            if (l) { // Load
                if(reg_num == 15 && s == 1){
                    arm_write_cpsr(p, arm_read_spsr(p));
                    val_pc = arm_read_cpsr(p);
                    arm_write_register(p, reg_num, val_pc);
                }
                else{
                    if(s == 1 && arm_in_a_privileged_mode(p) == 1){
                        arm_read_word(p, adresse, value);
                        arm_write_usr_register(p, reg_num, *value);
                        //the User mode banked registers are transferred instead of the registers of the current mode. 
                    }
                    else{
                        arm_read_word(p, adresse, value);
                        arm_write_register(p, reg_num, *value);
                    }
                }
            } 
            else { // Store
                if(s == 1 && arm_in_a_privileged_mode(p) == 1){
                    //the User mode banked registers are transferred instead of the registers of the current mode. 
                    *value = arm_read_usr_register(p, reg_num);
                    arm_write_word(p, adresse, *value);                   
                }
                else{
                    *value = (p->reg)->registre[reg_num];
                    arm_write_word(p, adresse, *value);  
                } 
            }
        }
    }

    if (w) { //maj du reg apres le transfert
        (p->reg)->registre[Rn] = adresse + increment;
    }

    return 0;
}


void read_write_coprocessor(int l, arm_core p, uint32_t adresse, uint8_t CRd, uint32_t *value){
    if (l){ //load
        arm_read_word(p, adresse, value);
        arm_write_register(p, CRd, *value);
    }
    else { //store
        arm_read_register(p, CRd);
        arm_write_word(p, adresse, *value);
    }
}

//page 490
int arm_coprocessor_load_store(arm_core p, uint32_t ins) {
    if(cond_not_respect(p, ins)){
        return -1;
    }
    
    int p_bit = get_bit(ins, 24);
    int u = get_bit(ins, 23);
    int w = get_bit(ins, 21);
    int l = get_bit(ins, 20);
    uint8_t Rn = get_bits(ins, 19, 16);
    uint8_t CRd = get_bits(ins, 15, 12);
    //uint8_t coprocessor_num = get_bits(ins, 11, 8);
    uint32_t offset = get_bits(ins, 7, 0);
    uint32_t adresse = (p->reg)->registre[Rn];
    uint32_t *value = (uint32_t *)malloc(sizeof(uint32_t));

    if(p_bit == 0 && w == 0 && u == 0){ //cas spécial
        return UNDEFINED_INSTRUCTION;
    }

    if (u == 0){
        offset = -offset;
    }
    
    if (p_bit == 0 && w == 0){
        read_write_coprocessor(l, p, adresse, CRd, *value);
        return 0;
    }
    else if(p_bit == 0 && w == 1){ // on peut réunir avec le else car la seule différence est entre arm_write_usr_register et arm_write_register
        read_write_coprocessor(l, p, adresse, CRd, *value);
        (p->reg)->registre[Rn] += offset;
        return 0;
    }
    else if(p_bit == 1 && w == 0){
        adresse +=offset;
        read_write_coprocessor(l, p, adresse, CRd, *value);
        return 0;
    }
    else { // p == 1 et w == 1
        adresse += offset;
        (p->reg)->registre[Rn] = adresse;
        read_write_coprocessor(l, p, adresse, CRd, *value);
        return 0;
    }
}