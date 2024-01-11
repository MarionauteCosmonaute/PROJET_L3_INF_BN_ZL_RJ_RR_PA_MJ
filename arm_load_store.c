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
#include "arm_data_processing.h"
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
    uint8_t value8;
    if(Rn ==15){
        value = adresse + 8;
    }
    int word_byte = get_bits(ins, 27, 26) == 0b01;
    int result; //sera utilisé pour vérifier si on a pas de data abort

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

            if(get_bit(ins, 4)){
                return UNDEFINED_INSTRUCTION; //est indiqué à 0 dans la doc
            }
            offset = Effectuer_Decalage(shift_op, shift_imm, Rm_value); //page 445         
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
                    result = arm_read_byte(p, adresse, &value8);
                }
                arm_write_register(p, Rd, value8);
            } 
            else { //LDR (word)
                if(Rn!=15){
                    result = arm_read_word(p, adresse, &value);
                }
                arm_write_register(p, Rd, value);
            }
            if(result == -1){
                return DATA_ABORT;
            }
                
        } 
        else { //store
            if(Rn!=15) {
                value = arm_read_register(p, Rd);
                value8 = arm_read_register(p, Rd);
            }
            if(b){ //STRB (byte) 
                result = arm_write_byte(p, adresse, value8);
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
                    break;
                }
                case 5: { //LDRH 
                    result =  arm_read_half(p, adresse, &value);
                    arm_write_register(p, Rd, value);
                    break;
                }     
                default:
                    return UNDEFINED_INSTRUCTION;
            }
            
            if(result == -1){
                return DATA_ABORT;
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
    uint16_t list_reg= get_bits(ins, 15, 0);
    uint32_t value;
    int result;
    uint32_t adresse = arm_read_register(p, Rn);
    
    
    if(p_bit){ // incrementation ou decrementation en fonction de u
        adresse += u ? 4 : -4;
    }
    if (list_reg == 0 || (get_bit(ins, Rn) && w)){
        return UNDEFINED_INSTRUCTION;
    }
    
    if(!s) { 
        for(int reg_num = 0; reg_num < 15; reg_num++){ 
            if(get_bit(list_reg, reg_num)) {
                if(l){
                    result = arm_read_word(p, adresse, &value);
                    arm_write_register(p, reg_num, value);
                }
                else{
                    value = arm_read_register(p, reg_num);
                    result = arm_write_word(p, adresse, value);
                }
                if(result == -1){
                    return DATA_ABORT;
                }
                adresse += u ? 4 : -4;
            }
        }
        
        if(get_bit(list_reg, 15)) {
            return UNDEFINED_INSTRUCTION; //cas spé pour la pc, branchement pour load et implementation defined en store
        }
    } 
    
    else { 
        return UNDEFINED_INSTRUCTION;
    }
    
    if(w){
        arm_write_register(p, Rn, adresse);
    }

    return 0;
}


int arm_coprocessor_load_store(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}