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
    int word_byte = get_bits(ins, 27, 26) == 0b01;

    //si mot ou byte non signé
    if(word_byte) {
        int immediat = get_bit(ins, 25);
        int b = get_bit(ins, 22);

        // si ce n'est pas un immediat
        if(immediat) {
            uint8_t Rm = get_bits(ins, 3, 0);
            uint32_t Rm_value = arm_read_register(p, Rm);
            uint8_t shift_op = get_bits(ins, 6, 5); //opérateur de décalage à appliquer à la valeur d'offset
            uint8_t shift_imm = get_bits(ins, 11, 7); //valeur du décalage appliqué à la valeur d'offset            

            if(get_bit(ins, 4)) return UNDEFINED_INSTRUCTION;
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
                        int carry_flag = get_bit(arm_read_cpsr(p), C);
                        offset = ((carry_flag << 31) | (Rm_value >> 1));
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
            if(w) arm_write_register(p, Rn, adresse);
        }

        if(l) { //load
            if(b) { //LDRB (byte)
                uint8_t value;

                int result = arm_read_byte(p, adresse, &value);
                if(result == -1){
                    return DATA_ABORT;
                }
                arm_write_register(p, Rd, value);
            } 
            else { //LDR (word)
                uint32_t value;

                int result = arm_read_word(p, adresse, &value);
                if(result == -1){
                    return DATA_ABORT;
                }
                arm_write_register(p, Rd, value);
            }
                
        } else { //store
            uint32_t value = arm_read_register(p, Rd);
            int result;

            if(b) //STRB (byte) 
                result = arm_write_byte(p, adresse, value);
            else //STR (word)
                result = arm_write_word(p, adresse, value);

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
        int half_signed_byte = (get_bits(ins, 27, 25) == 0b000 && get_bit(ins, 7) == 1 &&            get_bit(ins, 4) == 1 && p_bit == 1 && !(get_bits(ins, 6, 5) == 0b00);

        if(half_signed_byte) {
            int immediat = get_bit(ins, 22);
            uint8_t sh = get_bits(ins, 6, 5);
            uint8_t acces_mem = (l << 2) | sh; // LSH (sur 3 bits type acces mem)

            if(immediat){ //si immediat
                uint8_t immedL = get_bits(ins, 3, 0); 
                uint8_t immedH = get_bits(ins, 11, 8);
                offset = (immedH << 4) | immedL; //voir p.475
            } 
            else{
                uint8_t Rm = get_bits(ins, 3, 0);
                offset = arm_read_register(p, Rm);
            }

            if(p_bit) {
                adresse += u ? offset : -offset;
                if(w){
                  arm_write_register(p, Rn, adresse);
                }
            }

            /*
            L=0, S=0, H=1 Store halfword. Case 1
            L=0, S=1, H=0 Load doubleword. On a pas
            L=0, S=1, H=1 Store doubleword. On a pas
            L=1, S=0, H=1 Load unsigned halfword. Case 5
            L=1, S=1, H=0 Load signed byte. On a pas
            L=1, S=1, H=1 Load signed halfword. On a pas
            */


            switch(acces_mem) {
                case 1: { //STRH
                    uint16_t value = arm_read_register(p, Rd);

                    int result = arm_write_half(p, adresse, value);
                    if(result == -1){
                        return DATA_ABORT;
                    }
                    break;
                }
                case 5: { //LDRH 
                    uint16_t value;
                    int result =  arm_read_half(p, adresse, &value);
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


/*
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
    // vaut 1 si l'accès mémoire concerne un word ou byte
    int word_byte = get_bits(ins, 27, 26) == 0b01;

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

*/

/*
//page 482
int arm_load_store_multiple(arm_core p, uint32_t ins) {
    if(cond_not_respect(p, ins)){
        return -1;
    }

    /*
    page 481
    IA (Increment After)  P 0 U 1
    IB (Increment Before) P 1 U 1 
    DA (Decrement After)  P 0 U 0
    DB (Decrement Before) P 1 U 0

    W 1 base register is updated after the transfer. The base register is incremented
    (U==1) or decremented (U==0) by four times the number of registers in the register list.
    */
    /*
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

    int direction = u ? 1 : -1; // direction du transfert

     /*
    if ((p_bit == 0 && u == 0) || (p_bit == 1 && u == 1)) {
        adresse -= direction; // à verif j'ai pas hyper compris
    }
    */
/*
    adresse = calcul_adr(p, w, adresse); //
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
    /*
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
*/

int arm_load_store_multiple(arm_core p, uint32_t ins) {
    int p_bit = get_bit(ins, 24);
    int u = get_bit(ins, 23);
    int s = get_bit(ins, 22);
    int w = get_bit(ins, 21);
    int l = get_bit(ins, 20);
    uint8_t Rn = get_bits(ins, 19, 16);
    uint16_t list_registers = get_bits(ins, 15, 0);
    uint32_t value;
    uint32_t adresse = arm_read_register(p, Rn);
    
    if(p_bit) // incrementation ou decrementation en fonction de u
        adresse += u ? 4 : -4;

    if (list_registers == 0){
        return UNDEFINED_INSTRUCTION;
    }
    
    if(!s) { //si s==1 on part sur LDMs
        if(l) { //LDM(1)
            for(int reg_num = 0; reg_num < 15; reg_num++){ //on veut pas 15 car sinon c'est la PC
                if(get_bit(list_registers, reg_num)) {
                    int result = arm_read_word(p, adresse, &value);
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
                    int result = arm_write_word(p, adresse, value);
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

    if(w){
        if(p_bit) {
            adresse -= u ? 4 : -4; //à verif j'ai essayé de faire les calculs mais le mal de crane
        }
        arm_write_register(p, Rn, address);
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