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
#ifndef __ARM_DATA_PROCESSING_H__
#define __ARM_DATA_PROCESSING_H__
#include <stdint.h>
#include "arm_core.h"

// Spécification: Détermine de quel type de décalage il s'agit
// Prend en argument: int bit6-> le bit de poids fort du décalage
//					  int bit5-> le bit de poids faible du décalage
// Retourne: le code associé au décalage (défini dans le fichier arm_constants.h)
// Pas d'effets de bord
int Determiner_Decalage(int bit6, int bit5);

// Spécification: Effectue un décalage de "val" sur une donnée r
// Prend en argument: int décalage -> le type de décalage à faire,
//					  int val -> la valeur du décalage
//					  uint32_t r -> donnée qui va subir le décalage
// Retourne: La valeur r modifiée
// Effets de Bord 
uint32_t Effectuer_Decalage(int decalage, int val, uint32_t r);

// Spécification: Met à jour les flags dans cpsr
// Prend en argument: uint64_t temp -> resultat du calcul (sur 64 bit car cela permet de récupérer l'overflow (V) et la Carry (C). Pour la Carry, on regarde le bit 32 et pour l'overflow on compare le resultat sur 64 bits avec le résultat sur 32 bits)
//					  arm_core p -> récupère l'ensemble des registres
//					  int bit_S -> recupère le bit 20 de l'instruction afin de savoir si oui ou non il faut mettre les flags à jour
//					  uint32_t Rd_num -> Si Rd-num=15, nous devons tester dans quel mode nous sommes
// Retourne: 0 si tout c'est passé sans problème, 
//			 UNDEFINED_INSTRUCTION si nous avons un problème de mode
//			 1 s'il y a eu un problème
// Effets de bord : peut modifier le contenu du registre cpsr
int Update_Flags(uint64_t temp, arm_core p, int bit_S, uint32_t Rd_num);

// Specification: Effectue une opération entre deux opérandes
// Prend en argiment: int opCode -> type d'opération à effectuer (ADD = addition, LSL = décalage à gauche...)
//					  uint32_t Rn -> Premier opérande
//					  uint32_t operande2 -> 2nd opérande
//					  arm_core p -> récupère l'ensemble des registres
//					  uint32_t ins -> donnée qui contient l'instruction
// Retourne: 0 si tout c'est passé sans problème, 
//			 UNDEFINED_INSTRUCTION si nous avons un problème de mode
//			 1 s'il y a eu un problème
// Effets de bord : si besoin : met à jour cpsr (flags) ou met à jour le resultat dans le registre Rd de l'instruction
int Effectuer_Operation(int opeCode, uint32_t Rn, uint32_t operande2, arm_core p, uint32_t ins);

// Specification: S'occupe des instructions de data_processing avec des valeurs contenues dans un registre
// Prend en argument: arm_core p -> ensemble des registres
//					  uint32_t ins -> donnée qui contient l'instruction
// Retourne: 0 si tout c'est passé sans problème, 
//			 UNDEFINED_INSTRUCTION si nous avons un problème de mode
//			 1 s'il y a eu un problème
// Effet de bord: peut modifier les registres cpsr et le registre destination de l'instruction
int arm_data_processing_shift(arm_core p, uint32_t ins);

// Specification: S'occupe des instructions de data_processing avec des valeurs immédiates
// Prend en argument: arm_core p -> ensemble des registres
//					  uint32_t ins -> donnée qui contient l'instruction
// Retourne: 0 si tout c'est passé sans problème, 
//			 UNDEFINED_INSTRUCTION si nous avons un problème de mode
//			 1 s'il y a eu un problème
// Effet de bord: peut modifier les registres cpsr et le registre destination de l'instruction
int arm_data_processing_immediate_msr(arm_core p, uint32_t ins);

#endif