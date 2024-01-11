//#include "arm_data_processing.h"
#include <stdint.h>
#include <stdio.h>
#include "command_creator.h"
#include "arm_data_processing.h"

uint32_t update_cond(uint32_t ins,uint32_t mask){
    return (ins & EQ) | mask;
}
// COMMANDE= construire_instruction(cond,I,opcode,S,Rn,Rd,construire_shifter_xxx(args));

int main()
{
    //uint32_t commande= construire_instruction(AL,0,AND,1,5,5,construire_shifter_im(0x10,0x5F));
    uint32_t commande= construire_instruction(AL,0,AND,1,5,5,construire_shifter_Rs(8,LSR,2));
    printf("commande=%x\n",commande);
    printf("LSR ? %d\n",LSR == Determiner_Decalage((commande&0x20),(commande&0x10)));
}