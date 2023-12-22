/*
Armator - simulateur de jeu d'instruction ARMv5T � but p�dagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique G�n�rale GNU publi�e par la Free Software
Foundation (version 2 ou bien toute autre version ult�rieure choisie par vous).

Ce programme est distribu� car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but sp�cifique. Reportez-vous � la
  
Vous devez avoir re�u une copie de la Licence Publique G�n�rale GNU en m�me
temps que ce programme ; si ce n'est pas le cas, �crivez � la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
�tats-Unis.

Contact: Guillaume.Huard@imag.fr
	 B�timent IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'H�res
*/
#include "arm_data_processing.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "arm_branch_other.h"
#include "util.h"
#include "debug.h"


int Determiner_Decalage(int bit6, int bit5)
{
	if ( bit6 == 0 && bit5 == 0 )
	{
		return LSL;
	}

	if ( bit6 == 0 && bit5 == 1 )
	{
		return LSR;
	}
	if ( bit6 == 1 && bit5 == 0 )
	{
		return ASR;
	}
	// if ( bit6 == 1 && bit5 == 1 )
	return ROR;
}

uint32_t Effectuer_Decalage(int decalage, int val, uint32_t r)
{
	if ( decalage == LSL ) // On ajoute "val" aux bits de poids faibles 10110010 <--0
	{
		return r<<val;
	}
	if ( decalage == LSR ) // On ajoute "val" aux bits de poids forts 0 --> 01010011
	{
		return r>>val;
	}
	if ( decalage == ASR ) // On veut insérer "val" fois le bit 31 à la position 31 (en poussant les bits de poids faibles)
	{
		uint8_t bit_31 = get_bit(r, 31);
		for(int i=0; i<val; i++){
			if ( bit_31 == 1 )
			{
				 r = r>>1 | (1<<31);
			}
			else
			{
				 r = r>>1 & ~(1<<31);
			}
		}
		return r;
	}
	// 	if ( decalage == ROR ) //On veut effectuer une rotation par la droite: que les "val" de poids faibles re-rentrent et poussent les "val" bits de poids forts 
	return (r>>val) | (r<<(32-val));
}

int Update_Flags(uint64_t temp, arm_core p, int bit_S, uint32_t Rd_num)
{
	if (bit_S == 1 && Rd_num == 15)	
	{
		if(arm_current_mode_has_spsr(p))
		{
			arm_write_cpsr(p, arm_read_spsr(p)); // CPSR = SPSR
		}
		else
		{
			return UNDEFINED_INSTRUCTION;
		}
	}
	else if (bit_S == 1) // On actualise les flags
	{
		uint32_t res = (uint32_t) get_bits(temp,31,0);
		uint32_t cpsr = arm_read_cpsr(p); // Contenu du registre
		uint32_t masque_flagN = ~(1<<31);
		uint32_t masque_flagZ= ~(1<<30);
		uint32_t masque_flagC= ~(1<<29);
		uint32_t masque_flagV= ~(1<<28);

		int bit_poids_fort; // On veut récupèrer le bit de poids fort du résultat

		//Z
		if ( temp == 0 ) // Z = 1
		{
			arm_write_cpsr(p, (cpsr | (~masque_flagZ))); // Le masque était 1011 1111 1111 1111 1111 1111 1111 1111 on prend l'inverse: 0100 0000 0000 0000 0000 0000 0000 0000 pour forcer un 1 en position 30.
		}
		else // Z = 0
		{
			arm_write_cpsr(p, (cpsr & masque_flagZ)); // Le masque est 1011 1111 1111 1111 1111 1111 1111 1111: on force la mise à 0 du bitZ
		}

		//N
		bit_poids_fort = get_bit(temp, 31);	
		if ( bit_poids_fort == 1 ) // N = 1;
		{
			arm_write_cpsr(p, (cpsr | (~masque_flagN))); // Le masque était 0111 1111 1111 1111 1111 1111 1111 1111 on prend l'inverse: 1000 0000 0000 0000 0000 0000 0000 0000 pour forcer un 1 en position 30.
		}
		else // N = 0
		{
			arm_write_cpsr(p, (cpsr & masque_flagN)); // Le masque était 0111 1111 1111 1111 1111 1111 1111 1111: on force la mise à 0 du bitN
		}	

		//C
		if ( get_bit(temp,32) == 1 ) //on verifie si une carry est presente
		{ 							// C = 1
			arm_write_cpsr(p, (cpsr  | (~masque_flagC)));
		}
		else{
			arm_write_cpsr(p, (cpsr  & masque_flagC));
		}

		//V
		if (temp == (uint64_t)res) // V=0
		{
			arm_write_cpsr(p, (cpsr | masque_flagV)); // Le masque était 1110 1111 1111 1111 1111 1111 1111 1111: on force la mise à 0 du bitV
		}
		else{	// V=1
			arm_write_cpsr(p, (cpsr  & masque_flagV));
		}
	}
	return 0;
}

int Effectuer_Operation(int opeCode, uint32_t Rn, uint32_t operande2, arm_core p, uint32_t ins)
{
	uint32_t res;
	uint64_t temp;
	uint32_t Rd_num = get_bits(ins, 15, 12); // Numero du registre destination Rd

	uint32_t cpsr = arm_read_cpsr(p); // Contenu du registre
	int flagC = get_bit(cpsr, C);

	uint32_t masque_flagN = ~(1<<31);
	uint32_t masque_flagZ= ~(1<<30);
	uint32_t masque_flagC= ~(1<<29);
	uint32_t masque_flagV= ~(1<<28);

	int bit_poids_fort; // On veut récupèrer le bit de poids fort du résultat
	int bit_S = (int) get_bit(ins,20);

	int pb;

	switch (opeCode)
	{
		case 0:	// AND 0000 Rd <= Rn AND operande2
			res = (Rn & operande2);
			arm_write_register(p, Rd_num, res);
			// p159 doc arm
			pb = Update_Flags((uint64_t) res,p,bit_S,Rd_num);
			return pb;

		case 1:	// EOR 0001 Rd <= Rn XOR operande2
			res = (Rn ^ operande2);
			arm_write_register(p, Rd_num, res);
			// p183 doc arm
			pb = Update_Flags((uint64_t) res,p,bit_S,Rd_num);
			return pb;

		case 2:	// SUB 0010 Rd <= Rn - operande2
			res = (Rn - operande2);
			arm_write_register(p, Rd_num, res);
			// p359 doc arm
			pb = Update_Flags((uint64_t) res,p,bit_S,Rd_num);
			return pb;

		case 3:	// RSB 0010 Rd <= operande2 - Rn
			res = (operande2 - Rn);
			arm_write_register(p, Rd_num, res);
			// p266 doc arm
			pb = Update_Flags((uint64_t) res,p,bit_S,Rd_num);
			return pb;

		case 4:	// ADD 0100 Rd <= Rn + operande2
			res = (Rn + operande2);
			arm_write_register(p, Rd_num, res);
			// p157 doc arm
			pb = Update_Flags((uint64_t) res,p,bit_S,Rd_num);
			return pb;

		case 5:	// ADC 0101 Rd <= Rn + operande2 + C
			res = (Rn + operande2 + flagC);
			arm_write_register(p, Rd_num, res);
			// p155 doc arm
			pb = Update_Flags((uint64_t) res,p,bit_S,Rd_num);
			return pb;

		case 6:	// SBC 0110 Rd <= Rn - operande2 + C - 1
			res = (Rn - operande2 + flagC - 1);
			arm_write_register(p, Rd_num, res);
			// p276 doc arm
			pb = Update_Flags((uint64_t) res,p,bit_S,Rd_num);
			return pb;  

		case 7:	// RSC 0111 Rd <= operande2- Rn + C - 1
			res = (operande2 - Rn + flagC - 1);
			arm_write_register(p, Rd_num, res);
			// p268 doc arm
			pb = Update_Flags((uint64_t) res,p,bit_S,Rd_num);
			return pb;  

		case 8:	// TST 1000 Positionne les flags pour Rn AND Operande2
			res = Rn & operande2;
			// p380 doc arm
			
			//Z
			if ( res == 0 ) // Z = 1
			{
				arm_write_cpsr(p, (cpsr | (~masque_flagZ))); // Le masque était 1011 1111 1111 1111 1111 1111 1111 1111 on prend l'inverse: 0100 0000 0000 0000 0000 0000 0000 0000 pour forcer un 1 en position 30.
			}
			else // Z = 0
			{
				arm_write_cpsr(p, (cpsr & masque_flagZ)); // Le masque est 1011 1111 1111 1111 1111 1111 1111 1111: on force la mise à 0 du bitZ
			}

			//N
			bit_poids_fort = get_bit(res, 31);	
			if ( bit_poids_fort == 1 ) // N = 1;
			{
				arm_write_cpsr(p, (cpsr | (~masque_flagN))); // Le masque était 0111 1111 1111 1111 1111 1111 1111 1111 on prend l'inverse: 1000 0000 0000 0000 0000 0000 0000 0000 pour forcer un 1 en position 30.
			}
			else // N = 0
			{
				arm_write_cpsr(p, (cpsr & masque_flagN)); // Le masque était 0111 1111 1111 1111 1111 1111 1111 1111: on force la mise à 0 du bitN
			}
			return 0;  

		case 9:	// TEQ 1001 Positionne les flags pour Rn XOR Operande2
			res = Rn ^ operande2;
			// p378 doc arm

			//Z
			if ( res == 0 ) // Z = 1
			{
				arm_write_cpsr(p, (cpsr | (~masque_flagZ))); // Le masque était 1011 1111 1111 1111 1111 1111 1111 1111 on prend l'inverse: 0100 0000 0000 0000 0000 0000 0000 0000 pour forcer un 1 en position 30.
			}
			else // Z = 0
			{
				arm_write_cpsr(p, (cpsr & masque_flagZ)); // Le masque est 1011 1111 1111 1111 1111 1111 1111 1111: on force la mise à 0 du bitZ
			}

			//N
			bit_poids_fort = get_bit(res, 31);	
			if ( bit_poids_fort == 1 ) // N = 1;
			{
				arm_write_cpsr(p, (cpsr | (~masque_flagN))); // Le masque était 0111 1111 1111 1111 1111 1111 1111 1111 on prend l'inverse: 1000 0000 0000 0000 0000 0000 0000 0000 pour forcer un 1 en position 30.
			}
			else // N = 0
			{
				arm_write_cpsr(p, (cpsr & masque_flagN)); // Le masque était 0111 1111 1111 1111 1111 1111 1111 1111: on force la mise à 0 du bitN
			}			
			return 0;  

		case 10: // CMP 1010 Positionne les flags pour Rn - Operande2
			temp = (uint64_t) Rn - (uint64_t)operande2;
			res = (uint32_t) get_bits(temp,31,0);
			// p178 doc arm

			//Z
			if ( temp == 0 ) // Z = 1
			{
				arm_write_cpsr(p, (cpsr | (~masque_flagZ))); // Le masque était 1011 1111 1111 1111 1111 1111 1111 1111 on prend l'inverse: 0100 0000 0000 0000 0000 0000 0000 0000 pour forcer un 1 en position 30.
			}
			else // Z = 0
			{
				arm_write_cpsr(p, (cpsr & masque_flagZ)); // Le masque est 1011 1111 1111 1111 1111 1111 1111 1111: on force la mise à 0 du bitZ
			}

			//N
			bit_poids_fort = get_bit(temp, 31);	
			if ( bit_poids_fort == 1 ) // N = 1;
			{
				arm_write_cpsr(p, (cpsr | (~masque_flagN))); // Le masque était 0111 1111 1111 1111 1111 1111 1111 1111 on prend l'inverse: 1000 0000 0000 0000 0000 0000 0000 0000 pour forcer un 1 en position 30.
			}
			else // N = 0
			{
				arm_write_cpsr(p, (cpsr & masque_flagN)); // Le masque était 0111 1111 1111 1111 1111 1111 1111 1111: on force la mise à 0 du bitN
			}	

			//C
			if ( get_bit(temp,32) == 1 ) //on verifie si une carry est presente
			{ 							// C = 1
				arm_write_cpsr(p, (cpsr  | (~masque_flagC)));
			}
			else{
				arm_write_cpsr(p, (cpsr  & masque_flagC));
			}

			//V
			if (temp == (uint64_t)res) // V=0
			{
				arm_write_cpsr(p, (cpsr | masque_flagV)); // Le masque était 1110 1111 1111 1111 1111 1111 1111 1111: on force la mise à 0 du bitV
			}
			else{
				arm_write_cpsr(p, (cpsr  & masque_flagV));
			}
			return 0;  

		case 11: // CMN 1011 Positionne les flags pour Rn + Operande2
			temp = (uint64_t) Rn + (uint64_t)operande2;
			res = (uint32_t) get_bits(temp,31,0);
			// p176 doc arm
			
			//Z
			if ( temp == 0 ) // Z = 1
			{
				arm_write_cpsr(p, (cpsr | (~masque_flagZ))); // Le masque était 1011 1111 1111 1111 1111 1111 1111 1111 on prend l'inverse: 0100 0000 0000 0000 0000 0000 0000 0000 pour forcer un 1 en position 30.
			}
			else // Z = 0
			{
				arm_write_cpsr(p, (cpsr & masque_flagZ)); // Le masque est 1011 1111 1111 1111 1111 1111 1111 1111: on force la mise à 0 du bitZ
			}

			//N
			bit_poids_fort = get_bit(temp, 31);	
			if ( bit_poids_fort == 1 ) // N = 1;
			{
				arm_write_cpsr(p, (cpsr | (~masque_flagN))); // Le masque était 0111 1111 1111 1111 1111 1111 1111 1111 on prend l'inverse: 1000 0000 0000 0000 0000 0000 0000 0000 pour forcer un 1 en position 30.
			}
			else // N = 0
			{
				arm_write_cpsr(p, (cpsr & masque_flagN)); // Le masque était 0111 1111 1111 1111 1111 1111 1111 1111: on force la mise à 0 du bitN
			}	

			//C
			if ( get_bit(temp,32) == 1 ) //on verifie si une carry est presente
			{ 							// C = 1
				arm_write_cpsr(p, (cpsr  | (~masque_flagC)));
			}
			else{
				arm_write_cpsr(p, (cpsr  & masque_flagC));
			}

			//V
			if (temp == (uint64_t)res) // V=0
			{
				arm_write_cpsr(p, (cpsr | masque_flagV)); // Le masque était 1110 1111 1111 1111 1111 1111 1111 1111: on force la mise à 0 du bitV
			}
			else{
				arm_write_cpsr(p, (cpsr  & masque_flagV));
			}
			return 0;

		case 12: // ORR 1100 Rd <= Rn OR opérande2
			res = (Rn | operande2);
			arm_write_register(p, Rd_num, res);
			// p235 doc arm
			pb = Update_Flags((uint64_t) res,p,bit_S,Rd_num);
			return pb;  

		case 13: // MOV Rd <= opérande2
			res = operande2;
			arm_write_register(p, Rd_num, res);
			// p219 doc arm
			pb = Update_Flags((uint64_t) res,p,bit_S,Rd_num);
			return pb;  

		case 14: // BIC Rd <= Rn AND NOT opérande2
			res = (Rn & (~operande2));
			arm_write_register(p, Rd_num, res);
			// p163 doc arm
			pb = Update_Flags((uint64_t) res,p,bit_S,Rd_num);
			return pb;  

		case 15: // MVN Rd <= NOT opérande2
			res = (~operande2);
			arm_write_register(p, Rd_num, res);
			// p233 doc arm
			pb = Update_Flags((uint64_t) res,p,bit_S,Rd_num);
			return pb;    
		
		default:	//On ne fait rien
			return 1;	// Nous ne sommes pas sencé arriver dans ce cas
	}
}

int arm_data_processing_shift(arm_core p, uint32_t ins)	// Le 25ième bit i égal 0 
{
	if (cond_not_respect(p, ins))
	{
		return 1;
	}

	// La condition est respectée on continue
	// On s'occupe de notre 2nd opérande --> 	Comme bit_I = 0  Nous avons à faire à une valeur présente dans un registre: Le registre est situé dans les bits 0 à 3 et est utilisé comme 2nd opérande. Les bits 4 à 11 déterminent le décalage

	uint8_t Rm_num = (uint8_t) get_bits(ins,3,0); // Je récupère le numéro du registre Rm qui correspond au registre utilisé comme second opérande.
	uint32_t Rm_valeur = arm_read_register(p, Rm_num); // Je socke la valeur contenue dans Rm
	int decalage = Determiner_Decalage(get_bit(ins,6),get_bit(ins,5));	//Type de décalage
	int valeur; // Valeur du décalage

	uint8_t bit_4 = (uint8_t) get_bit(ins,4);
	// La valeur du bit 4 permet de savoir si la valeur du décalage est donné dans un registre (bit4 = 1) ou par valeur immédiate (bit4 = 0) 
	if ( bit_4 == 0 ) // La valeur immédiate du décalge se trouve dans les bits 7 à 11.
	{
		valeur = (int) get_bits(ins,11,7); //
	}
	else // bit_4 == 1		La valeur du décalage correspond au 5 bits de poids faible du registre qui a pour numero les bits 8 à 11.
	{
		uint8_t Rs_num = (uint8_t) get_bits(ins,11,8); // Je récupère le numéro du registre Rs.
		uint32_t Rs_valeur = arm_read_register(p,Rs_num); // Je socke la valeur contenue dans Rs
		valeur = (int) get_bits(Rs_valeur,4,0);	//Je récupère les 5 bits de poids faible de Rs
	}
	Rm_valeur = Effectuer_Decalage(decalage, valeur ,Rm_valeur);

	//Notre 2nd opérande est à jour
	//Actuellement stocké dans la VARIABLE Rm_valeur (uint32_t)

	int opCode = (int) get_bits(ins,24,21);
	uint8_t Rn_num = (uint8_t) get_bits(ins,19,16);
	uint32_t Rn_valeur = arm_read_register(p,Rn_num);
	int pb = Effectuer_Operation(opCode, Rn_valeur, Rm_valeur, p, ins);

    return pb;
}

int arm_data_processing_immediate_msr(arm_core p, uint32_t ins) // Le 25ième bit i égal 1
{
	if (cond_not_respect(p, ins))
	{
		return 1;
	}

	// La condition est respectée on continue
	// bit_I == 1 	Les bits 0 à 7 correspondent à la valeur utilisée comme second opérande et les bits 8 à 11 correspondent au décalage (icic Rotation) appliquée à cette valeur.
	
	uint8_t valeur_I = (uint8_t) get_bits(ins,7,0); // Je récupère la valeur de l'immédiat I utilisée comme second opérande
	int valeur = (int) get_bits(ins,11,8);	// On récupère la valeur du décalage
	valeur_I = (valeur_I>>valeur) | (valeur_I<<8-valeur); // On effectue une rotation sur 8 bits donc on n'utilise pas la fonction Effectuer_Decalage()
	// Notre 2nd opérande est à jour
	// Actuellement stocké dans la Variable valeur_i (uint8_t)

	int opCode = (int) get_bits(ins,24,21);
	uint8_t Rn_num = (uint8_t) get_bits(ins,19,16);
	uint32_t Rn_valeur = arm_read_register(p,Rn_num);
	int pb = Effectuer_Operation(opCode, Rn_valeur, (uint32_t) valeur_I, p, ins);
	
    return pb;
}
