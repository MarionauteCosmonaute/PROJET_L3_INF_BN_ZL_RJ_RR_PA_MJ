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
#ifndef __REGISTERS_H__
#define __REGISTERS_H__
#include <stdint.h>
#include <stdio.h>

#define R0 0
#define R1 1
#define R2 2
#define R3 3 
#define R4 4
#define R5 5
#define R6 6
#define R7 7
#define R8 8
#define R9 9
#define R10 10
#define R11 11
#define R12 12
#define R13 13
#define R14 14
#define PC 15
#define CPSR 16
#define R13_svc 17
#define R14_svc 18
#define SPSR_svc 19
#define R13_abt 20
#define R14_abt 21
#define SPSR_abt 22
#define R13_und 23
#define R14_und 24
#define SPSR_und 25
#define R13_irq 26
#define R14_irq 27
#define SPSR_irq 28
#define R8_fiq 29
#define R9_fiq 30
#define R10_fiq 31
#define R11_fiq 32
#define R12_fiq 33
#define R13_fiq 34
#define R14_fiq 35
#define SPSR_fiq 36

typedef struct registers_data *registers;

registers registers_create();
void registers_destroy(registers r);

uint8_t registers_get_mode(registers r);
int registers_current_mode_has_spsr(registers r);
int registers_in_a_privileged_mode(registers r);

uint32_t registers_read(registers r, uint8_t reg, uint8_t mode);
uint32_t registers_read_cpsr(registers r);
uint32_t registers_read_spsr(registers r, uint8_t mode);
void registers_write(registers r, uint8_t reg, uint8_t mode, uint32_t value);
void registers_write_cpsr(registers r, uint32_t value);
void registers_write_spsr(registers r, uint8_t mode, uint32_t value);

#endif
