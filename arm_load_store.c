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


int arm_load_store(arm_core p, uint32_t ins) {
    if(cond_not_respect(p, ins)){
        return -1;
    }
    
    uint8_t Rn = get_bits(ins, 19, 16);
    uint8_t Rd = get_bits(ins, 15, 12);
    int u = get_bit(ins, 23);
    int p_bit = get_bit(ins, 24);
    int w = get_bit(ins, 21);
    int l = get_bit(ins, 20);
    uint32_t offset;
    uint32_t adresse = (p->reg)->registre[Rn];
    uint32_t value;
    if(Rn ==15){
        value = adresse + 8;
    }
    int word_byte = get_bits(ins, 27, 26) == 0b01;
    int result; //sera utilisé pour vérifier si on a pas de data abort
    //voir pour definir value en uint32_t ici mais il faut cast dans certains cas

    //si mot ou byte non signé
    if(word_byte) {
        int immediat = get_bit(ins, 25);
        int b = get_bit(ins, 22);

        // si ce n'est pas un immediat
        if(immediat) {
            uint8_t Rm = get_bits(ins, 3, 0);
            uint32_t Rm_value = arm_read_register(p, Rm);
            uint8_t shift_op = get_bits(ins, 6, 5);
            uint8_t shift_imm = get_bits(ins, 11, 7); //val qu'on va utiliser pour décaler la valeur d'offset            

            if(get_bit(ins, 4)) return UNDEFINED_INSTRUCTION; //est indiqué à 0 dans la doc
            //page 445
            //à voir si les opérations sont faites autres part
            switch(shift_op) {
                case LSL: //Logical shift left 
                    offset = Rm_value << shift_imm;
                    break;
                case LSR: //Logical shift right
                    offset = (shift_imm == 0) ? 0 : (Rm_value >> shift_imm);
                    break;
                case ASR: //Arithmetic shift right
                    offset = asr(Rm_value, shift_imm);
                    break;
                case ROR: { // Rotate right, shift_imm==0, RRX Rotate right with extend
                    if(shift_imm == 0){
                        // bit de retenue sortante
                        int carry = get_bit(arm_read_cpsr(p), C);
                        offset = ((carry << 31) | (Rm_value >> 1));
                    } 
                    else{
                        offset = ror(Rm_value, shift_imm); 
                    }
                    break;
                }
                default:
                    return UNDEFINED_INSTRUCTION;
            }
        } 
        else{//si valeur immédiate
            offset = get_bits(ins, 11, 0);
        }

        if(p_bit) {
            adresse += u ? offset : -offset; //si u==0, on decremente l'offset sinon on incremente
            if(w){
                arm_write_register(p, Rn, adresse);
            }
        }

        if(l) { //load
            if(b) { //LDRB (byte)
                if(Rn!=15){
                    result = arm_read_byte(p, adresse, (uint8_t) &value);
                    if(result == -1){
                        return DATA_ABORT;
                    }
                }
                arm_write_register(p, Rd, value);
            } 
            else { //LDR (word)
                if(Rn!=15){
                    result = arm_read_word(p, adresse, &value);
                    if(result == -1){
                        return DATA_ABORT;
                    }
                }
                arm_write_register(p, Rd, value);
            }
                
        } 
        else { //store
            if(Rn!=15) value = arm_read_register(p, Rd);
            if(b){ //STRB (byte) 
                result = arm_write_byte(p, adresse, (uint8_t) value);
            }
            else{ //STR (word)
                result = arm_write_word(p, adresse, value);
            }

            if(result == -1){
                    return DATA_ABORT;
            }
        }  

        //the offset is then applied to the base register value and written back to the base register
        if(!p_bit) {
            if(w){ //LDRBT / LDRT / STRBT / STRT non demandées
                return UNDEFINED_INSTRUCTION;
            }
            adresse += u ? offset : -offset;
            arm_write_register(p, Rn, adresse);
        }
    } 
    
        
    //si half
    else {
        if(Rn == 15){
            return UNDEFINED_INSTRUCTION;
        }
        int half = (get_bits(ins, 27, 25) == 0b000 && get_bit(ins, 7) == 1 && get_bit(ins, 4) == 1);
        uint16_t value;
        if(half) {
            int immediat = get_bit(ins, 22);
            uint8_t sh = get_bits(ins, 6, 5);
            uint8_t acces_mem = (l << 2) | sh; // LSH (sur 3 bits type acces mem)
             /*
            L=0, S=0, H=1 Store halfword. Case 1
            L=1, S=0, H=1 Load unsigned halfword. Case 5
            */

            if(immediat){ //si immediat
                uint8_t immedL = get_bits(ins, 3, 0); 
                uint8_t immedH = get_bits(ins, 11, 8);
                offset = (immedH << 4) | immedL; //voir p.475 immedH top 4 bits, immedL bottom 4 bits
            } 
            else{
                uint8_t Rm = get_bits(ins, 3, 0);
                offset = arm_read_register(p, Rm); // Specifies the register containing the offset to add to or subtract from Rn
            }

            if(p_bit) {
                adresse += u ? offset : -offset;
                if(w){
                  arm_write_register(p, Rn, adresse);
                }
            }

            switch(acces_mem) {
                case 1: { //STRH
                    value = arm_read_register(p, Rd);
                    result = arm_write_half(p, adresse, value);
                    if(result == -1){
                        return DATA_ABORT;
                    }
                    break;
                }
                case 5: { //LDRH 
                    result =  arm_read_half(p, adresse, &value);
                    if(result == -1){
                        return DATA_ABORT;
                    }
                    arm_write_register(p, Rd, value);
                    break;
                }     
           
                default:
                    return UNDEFINED_INSTRUCTION;
            }


            if(!p_bit) {
                if(w){
                    return UNDEFINED_INSTRUCTION;
                }
                adresse += u ? offset : -offset;
                arm_write_register(p, Rn, adresse);
            }
        } 
        else { 
            return UNDEFINED_INSTRUCTION;
        }
    }

    return 0;
}

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
    uint16_t list_registers = get_bits(ins, 15, 0);
    uint32_t value;
    int result;
    uint32_t adresse = arm_read_register(p, Rn);
    
    if(p_bit){ // incrementation ou decrementation en fonction de u
        adresse += u ? 4 : -4;
    }
    if (list_registers == 0){
        return UNDEFINED_INSTRUCTION;
    }
    
    if(!s) { //si s==1 on part sur LDMs
        if(l) { //LDM(1)
            for(int reg_num = 0; reg_num < 15; reg_num++){ //on veut pas 15 car sinon c'est la PC
                if(get_bit(list_registers, reg_num)) {
                    result = arm_read_word(p, adresse, &value);
                    if(result == -1){
                        return DATA_ABORT;
                    }
                    adresse += u ? 4 : -4;
                    arm_write_register(p, reg_num, value);
                }
            }

            if(get_bit(list_registers, 15)) {
                //pas trouvé si cas spé pour la PC, car on a pas S==1 donc pas le truc avec CPSR etc
            }

        } 
        else { //STM(1)
            for(int reg_num = 0; reg_num <= 15; reg_num++) {
                if(get_bit(register_list, reg_num)) {
                    value = arm_read_register(p, reg_num);
                    result = arm_write_word(p, adresse, value);
                    if(result == -1) {
                        return DATA_ABORT;
                    }
                    adresse += u ? 4 : -4;
                }
            }

        }
    } 
    
    else { 
        return UNDEFINED_INSTRUCTION;
    }

    int nbReg = 0;
    for(int i=0; i<1; i++){
        if (get_bit(ins,i) == 1){
            nbReg+=1;
        }
    }
    
    if(w){
        adresse += u ? 4*nbReg : -4*nbReg;
        arm_write_register(p, Rn, adresse);
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