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
#include <stdlib.h>
#include <stdio.h>
#include "memory.h"
#include "util.h"

/*Initialize memory access*/
memory memory_create(size_t size) {
    memory mem = malloc(sizeof(struct memory_data));
    if (mem==NULL){
      return NULL;
    }
    mem->size = size;
    mem->data = (uint8_t *)malloc(sizeof(uint8_t) *size);

    if (mem->data == NULL) {
        free(mem);
    }
    return mem;
}
/*gives memory size*/
size_t memory_get_size(memory mem) {
    return mem->size;
}
/*destroy memory access*/
void memory_destroy(memory mem) {
    free(mem->data);
    free(mem);
}
/*Reads a byte at the given address in the given memory, stores it at value, returns 0 if everything went well, -1 otherwise*/
int memory_read_byte(memory mem, uint32_t address, uint8_t *value) {
    if ((address < 0) || (address >= (mem->size))) {
        printf("Adresse fournie incorrecte, hors de la plage de valeur\n");
        return -1;
    }
    *value = (uint8_t)mem->data[address];
    return 0;
}

/*Reads a half word at the given address in the given memory, stores it at value, returns 0 if everything went well, -1 otherwise*/
int memory_read_half(memory mem, uint32_t address, uint16_t *value, uint8_t be) {
    if ((address < 0) || (address >= (mem->size))) {
        printf("Adresse fournie incorrecte, hors de la plage de valeur\n");
        return -1;
    }
    uint16_t tmp = mem->data[address+1] +(mem->data[address]<<8);
    if (be) { //si en big endian
        *value = tmp;  
    } 
    else {
        *value = reverse_2(tmp);
    }
    return 0;
}

    
/*Reads a word at the given address in the given memory, stores it at value, returns 0 if everything went well, -1 otherwise*/
int memory_read_word(memory mem, uint32_t address, uint32_t *value, uint8_t be) {
    if ((address < 0) || (address >= (mem->size))) {
        printf("Adresse fournie incorrecte, hors de la plage de valeur\n");
        return -1;
    }
    uint32_t tmp = 0;
    for (int i=0;i<4;i++){
        tmp+=(mem->data[address+i]<<(3-i)*8);
    }
    if(be) {
        *value = tmp;
    } 
    else {
        *value = reverse_4(tmp);
    }
    return 0; 
}

/*Writes a byte at the given address in the given memory, returns 0 if everything went well, -1 otherwise*/
int memory_write_byte(memory mem, uint32_t address, uint8_t value) {
    if ((address < 0) || (address >= (mem->size))) {
        printf("Adresse fournie incorrecte, hors de la plage de valeur\n");
        return -1;
    }
    mem->data[address] = (value);
    //set_bits(mem->data[address], 24, 8, value);
    return 0;
}
/*Writes a half word at the given address in the given memory, returns 0 if everything went well, -1 otherwise*/
int memory_write_half(memory mem, uint32_t address, uint16_t value, uint8_t be) {
    if ((address < 0) || (address >= (mem->size))) {
        printf("Adresse fournie incorrecte, hors de la plage de valeur");
        return -1;
    }
    for(int i=0;i<2;i++){
        if(be) {
            mem->data[address+i] = get_bits(value,8*(2-i),8*(1-i));
            //set_bits(mem->data[address], 16, 16, value);
        } 
        else {
            mem->data[address+i] = get_bits(value,8*(i+1),8*i);
           //set_bits(reverse_2(mem->data[address]), 16, 16, value);
        }
    }
    return 0;
    
}
/*Writes a word at the given address in the given memory, returns 0 if everything went well, -1 otherwise*/
int memory_write_word(memory mem, uint32_t address, uint32_t value, uint8_t be) {
    if ((address < 0) || (address >= (mem->size))) {
        printf("Adresse fournie incorrecte, hors de la plage de valeur");
        return -1;
    }
    for(int i=0;i<4;i++){
        if(be){
            mem->data[address+i] = get_bits(value,8*(4-i),8*(3-i));
            //set_bits(mem->data[address], 0, 32, value);    
        }
        else{
            mem->data[address+i] = get_bits(value,8*(i+1),8*i);
            //set_bits(reverse_4(mem->data[address]), 0, 32, value);    
        }
    }
    return 0;
}
