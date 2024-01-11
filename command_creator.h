#ifndef __C_CREATOR_H__
#define __C_CREATOR_H__

#include <stdint.h>


#define EQ 0x0
#define NE 0x1 
#define CS 0x2
#define CC 0x3
#define MI 0x4
#define PL 0x5
#define VS 0x6
#define VC 0x7
#define HI 0x8
#define LS 0x9
#define GE 0xA
#define LT 0xB
#define GT 0xC
#define LE 0xD
#define AL 0xE

#define AND 0x0 
#define EOR 0x1
#define SUB 0x2
#define RSB 0x3
#define ADD 0x4
#define ADC 0x5
#define SBC 0x6
#define RSC 0x7
#define TST 0x8
#define TEQ 0x9
#define CMP 0xA
#define CMN 0xB
#define ORR 0xC
#define MOV 0xD
#define BIC 0xE
#define MVN 0xF

#define LSL 0
#define LSR 1
#define ASR 2
#define ROR 3

uint32_t construire_instruction(int cond,int I,int opcode,int S,int Rn,int Rd, int shifter_operand);
//I=0
uint16_t construire_shifter_im(int rotate, int immediate);
//I=1
uint16_t construire_shifter_val(int valeur, int type_decal,int Rm);
//I=1
uint16_t construire_shifter_Rs(int Rs, int type_decal,int Rm);

#endif