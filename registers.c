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
#include "registers.h"
#include "arm_constants.h"
#include <stdlib.h>



struct registers_data
{
    uint32_t *registre;
};

registers registers_create()
{
	registers table_registre = malloc(sizeof(registers));
	table_registre->registre  = malloc(sizeof(uint32_t)*37);   
	if ( table_registre == NULL ){
        return NULL;
    }
    return table_registre;
}

void registers_destroy(registers table_registre)
{
    free(table_registre->registre);
	free(table_registre);
}

uint8_t registers_get_mode(registers table_registre) 
{
    return (uint8_t)table_registre->registre[CPSR] & 0x1f;  //On masque le registre CPSR pour récupérer les 5 bits de poids faible
}

static int registers_mode_has_spsr(uint8_t mode) {
	if(mode == USR || mode == SYS){
		return 0;
	} 
    return 1;
}

int registers_current_mode_has_spsr(registers r) {
    return registers_mode_has_spsr(registers_get_mode(r));
}

int registers_in_a_privileged_mode(registers r) {
	if (registers_get_mode(r) == USR){
		return 0;
	}
	return 1;
}

uint32_t registers_read(registers r, uint8_t reg, uint8_t mode) {
    uint32_t value = 0;
	switch(reg){
        case R8:
        if(mode == FIQ){
            value = r->registre[R8_fiq];
        }else{
            value = r->registre[R8];
        }
        break;
        case R9:
        if(mode == FIQ){
            value = r->registre[R9_fiq];
        }else{
            value = r->registre[R9];
        }
        break;
        case R10:
        if(mode == FIQ){
            value = r->registre[R10_fiq];
        }else{
            value = r->registre[R10];
        }
        
        break;
        case R11:
        if(mode == FIQ){
            value = r->registre[R11_fiq];
        }else{
            value = r->registre[R11];
        }

        break;
        case R12:
        if(mode == FIQ){
            value = r->registre[R12_fiq];
        }else{
            value = r->registre[R12];
        }

        break;
        case R13:
        switch(mode){ 
            case SVC:
                value = r->registre[R13_svc];
            break;
            case ABT:
                value = r->registre[R13_abt];
            break;
            case UND:
                value = r->registre[R13_und];
            break;
            case IRQ:
                value = r->registre[R13_irq];
            break;
            case FIQ:
                value = r->registre[R13_fiq];
            break;
            case USR:
            case SYS:
                value = r->registre[R13];
            break;
        }
        break;
        case R14:
            switch(mode){
            case SVC:
                value = r->registre[R14_svc];
            break;
            case ABT:
                value = r->registre[R14_abt];
            break;
            case UND:
                value = r->registre[R14_und];
            break;
            case IRQ:
                value = r->registre[R14_irq];
            break;
            case FIQ:
                value = r->registre[R14_fiq];
            break;
            case USR:
            case SYS:
                value = r->registre[R14];
            break;
        }
        break;
        case 17://acces a spsr 
            value = registers_read_spsr(r,mode);
        break;
        default:
            value = r->registre[reg];
        break;
    }
    return value;
}

uint32_t registers_read_cpsr(registers r) {
    uint32_t value = 0;
	value = r->registre[CPSR]; 
    return value;
}

uint32_t registers_read_spsr(registers r, uint8_t mode) {
	uint32_t value = 0;
	if(registers_mode_has_spsr(mode)){
		switch(mode){
            case SVC:
                value = r->registre[SPSR_svc];
            break;
            case ABT:
                value = r->registre[SPSR_abt];
            break;
            case UND:
                value = r->registre[SPSR_und];
            break;
            case IRQ:
                value = r->registre[SPSR_irq];
            break;
            case FIQ:
                value = r->registre[SPSR_fiq];
            break;
        }
	}else{
        printf("ERROR : This mode has no SPSR :/ undefined behaviour\n");
        return 0;
    }
	return value;
}

void registers_write(registers r, uint8_t reg, uint8_t mode, uint32_t value) {
		switch(reg){
        case R8:
        if (mode == FIQ){
            r->registre[R8_fiq] = value;
        }else{
            r->registre[R8] = value;
        }
        break;
        case R9:
        if (mode == FIQ){
            r->registre[R9_fiq] = value;
        }else{
            r->registre[R9] = value;
        }
        break;
        case R10:
        if (mode == FIQ){
            r->registre[R10_fiq] = value;
        }else{
            r->registre[R10] = value;
        }
        
        break;
        case R11:
        if (mode == FIQ){
            r->registre[R11_fiq]= value;
        }else{
            r->registre[R11] = value;
        }

        break;
        case R12:
        if (mode == FIQ){
            r->registre[R12_fiq] = value;
        }else{
            r->registre[R12] = value;
        }

        break;
        case R13:
        switch(mode){ 
            case SVC:
                r->registre[R13_svc] = value;
            break;
            case ABT:
                r->registre[R13_abt] = value;
            break;
            case UND:
                r->registre[R13_und] = value;
            break;
            case IRQ:
                r->registre[R13_irq] = value;
            break;
            case FIQ:
                r->registre[R13_fiq] = value;
            break;
            case USR:
            case SYS:
                r->registre[R13] = value;
            break;
        }
        break;
        case R14:
            switch(mode){
            case SVC:
                r->registre[R14_svc] = value;
            break;
            case ABT:
                r->registre[R14_abt] = value;
            break;
            case UND:
                r->registre[R14_und] = value;
            break;
            case IRQ:
                r->registre[R14_irq] = value;
            break;
            case FIQ:
                r->registre[R14_fiq] = value;
            break;
            case USR:
            case SYS:
                r->registre[R14] = value;
            break;
        }
        break;
        case 17://acces a spsr 
            registers_write_spsr(r,mode,value);
        break;

        default:
            r->registre[reg] = value;
        break;
    }
}


void registers_write_cpsr(registers r, uint32_t value) {
	r->registre[CPSR] = value;
}

void registers_write_spsr(registers r, uint8_t mode, uint32_t value) {
	if(registers_mode_has_spsr(mode)){
		switch(mode){
            case SVC:
                r->registre[SPSR_svc]=value;
            break;
            case ABT:
                r->registre[SPSR_abt] = value;
            break;
            case UND:
                r->registre[SPSR_und] = value;
            break;
            case IRQ:
                r->registre[SPSR_irq] = value;
            break;
            case FIQ:
                r->registre[SPSR_fiq] = value;
            break;
        }
	}else{
        printf("ERROR : This mode has no acces to a SPSR register :/ undefined behaviour\n");
    }
}

