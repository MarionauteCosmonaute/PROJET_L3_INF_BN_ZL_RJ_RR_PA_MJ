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

struct memory_data {
    size_t size;
    uint32_t *data;
};

memory memory_create(size_t size) {
    memory mem = malloc(sizeof(struct memory_data));
    if (mem==NULL){
      return NULL;
    }
    mem->size = size;
    mem->data = (uint32_t *)malloc(sizeof(uint32_t) *size);

    if (mem->data == NULL) {
        free(mem);
    }
    return mem;
}

size_t memory_get_size(memory mem) {
    return mem->size;
}

void memory_destroy(memory mem) {
    free(mem->data);
    free(mem);
}

int memory_read_byte(memory mem, uint32_t address, uint8_t *value) {
    if ((address < 0) || (address >= (mem->size))) {
        printf("Adresse fournie incorrecte, hors de la plage de valeur\n");
        return -1;
    }
    *value = get_bits(mem->data[address], 8, 0);
    return 0;
}

int memory_read_half(memory mem, uint32_t address, uint16_t *value, uint8_t be) {
    if ((address < 0) || (address >= (mem->size))) {
        printf("Adresse fournie incorrecte, hors de la plage de valeur\n");
        return -1;
    }

    if (be) { //si en big endian
        *value = get_bits(mem->data[address], 16, 0);  
    } 
    else {
        *value = get_bits(reverse_2(mem->data[address]), 16, 0);
    }
    return 0;
}

int memory_read_word(memory mem, uint32_t address, uint32_t *value, uint8_t be) {
    if ((address < 0) || (address >= (mem->size))) {
        printf("Adresse fournie incorrecte, hors de la plage de valeur\n");
        return -1;
    }
  
    if(be) {
        *value = mem->data[address];
    } 
    else {
        *value = reverse_4(mem->data[address]);
    }
    return 0; 
}

int memory_write_byte(memory mem, uint32_t address, uint8_t value) {
    if ((address < 0) || (address >= (mem->size))) {
        printf("Adresse fournie incorrecte, hors de la plage de valeur\n");
        return -1;
    }
    mem->data[address] = (value);
    //set_bits(mem->data[address], 24, 8, value);
    return 0;
}

int memory_write_half(memory mem, uint32_t address, uint16_t value, uint8_t be) {
    if ((address < 0) || (address >= (mem->size))) {
        printf("Adresse fournie incorrecte, hors de la plage de valeur");
        return -1;
    }
    
    if(be) {
        mem->data[address] = (uint32_t)(value);
        //set_bits(mem->data[address], 16, 16, value);
    } 
    else {
        mem->data[address] = (uint32_t)(value<<16);
       //set_bits(reverse_2(mem->data[address]), 16, 16, value);
    }
    return 0;
    
}

int memory_write_word(memory mem, uint32_t address, uint32_t value, uint8_t be) {
    if ((address < 0) || (address >= (mem->size))) {
        printf("Adresse fournie incorrecte, hors de la plage de valeur");
        return -1;
    }

    if(be){
        mem->data[address] = value;
        //set_bits(mem->data[address], 0, 32, value);    
    }
    else{
        mem->data[address] = reverse_4(value);
        //set_bits(reverse_4(mem->data[address]), 0, 32, value);    
    }
    return 0;
}
