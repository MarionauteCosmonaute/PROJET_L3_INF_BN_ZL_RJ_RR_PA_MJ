#include "command_creator.h"



uint32_t construire_instruction(int cond,int I,int opcode,int S,int Rn,int Rd, int shifter_operand){
    uint32_t out= 0 + (cond<<28) + ((I&0b1)<<25) + (opcode<<21) + ((S&0b1)<<20) + ((Rn&0xF)<<16) + ((Rd&0xF)<<12) + (shifter_operand&0xFFF);
    return out;
}


//Operande 2

uint16_t construire_shifter_im(int rotate, int immediate){
    uint16_t out= 0 + ((rotate&0xF)<<8) + (immediate & 0xFF) ;
    return out;
}

uint16_t construire_shifter_val(int valeur, int type_decal,int Rm){
    uint16_t out= 0 + ((valeur&0x1F)<<7) + (type_decal<<5)+ (Rm&0xF);
    return out;
}

uint16_t construire_shifter_Rs(int Rs, int type_decal,int Rm){
    uint16_t out= 0 + ((Rs&0xF)<<8) + (type_decal<<5) + (1<<4) + (Rm&0xF);
    return out;
}

