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
#ifndef __MEMORY_H__
#define __MEMORY_H__
#include <stdint.h>
#include <sys/types.h>

struct memory_data {
    size_t size;
    uint8_t *data;
};

typedef struct memory_data *memory;

memory memory_create(size_t size);
size_t memory_get_size(memory mem);
void memory_destroy(memory mem);

/* All these functions perform a read/write access to a byte/half/word data at
 * address a in mem. The result is respectively taken from or stored to the
 * parameter value. The access is made using the given endianess (be == 1 for a
 * big endian access and be == 0 for a little endian access).
 * The return value indicates a succes (0) or a failure (-1).
 */
int memory_read_byte(memory mem, uint32_t address, uint8_t * value);
int memory_read_half(memory mem, uint32_t address, uint16_t * value, uint8_t be);
int memory_read_word(memory mem, uint32_t address, uint32_t * value, uint8_t be);
int memory_write_byte(memory mem, uint32_t address, uint8_t value);
int memory_write_half(memory mem, uint32_t address, uint16_t value, uint8_t be);
int memory_write_word(memory mem, uint32_t address, uint32_t value, uint8_t be);

#endif
