#include <stdlib.h>
#include <stdio.h>
#include "arm_data_processing.h"
#include "arm_core.h"
#include "arm_constants.h"
#include <time.h>
#include "util.h"


void print_test(int result){
    if(result){
        printf("Test Succeed !\n");
    }
    else{
        printf("Test Failed\n");
    }
}

int Flags_Test(int MonV, int MonC, arm_core p, uint64_t temp, int Rd_num)
{
    int espoir = 1;
    int var = 1;
    uint32_t res = (uint32_t)temp;

    int bit_poids_fort;
    int bit_carry;

    uint32_t cpsr = arm_read_cpsr(p); // Contenu du registre
    uint8_t flagZ = (uint8_t) get_bit(cpsr,Z);
	uint8_t flagN = (uint8_t) get_bit(cpsr,N);
	uint8_t flagC = (uint8_t) get_bit(cpsr,C);
	int flagV = (int) get_bit(cpsr,V);

    bit_carry = get_bit(temp,32);
    bit_poids_fort = get_bit(temp, 31);
    
    printf("\n");
    printf("Le flagZ vaut: %d et le resulat est: %u\n", flagZ, res);
    printf("Le flagN vaut: %d et il devrait valoir: %d\n", flagN, bit_poids_fort);
    if((res == 0) && (flagZ!= 1))
    {
        printf("Failed flags Z 1\n");
        espoir = 0;

    }
    if ((res != 0) && (flagZ != 0))
    {
        printf("Failed flags Z 1\n");
        espoir = 0;
    }
    if (bit_poids_fort != flagN)
    {
        printf("Failed flags N\n");
        espoir = 0;
    }
    if (MonC == 1)
    {
        printf("Le flagC vaut: %d et il devrait valoir: %d\n", flagC, bit_carry);
        if (flagC != bit_carry)
        {
            printf("Failed flags C\n");
            espoir = 0;
        }
    }
    if (MonV == 1)
    {
        if (temp == (uint64_t)res)
        {
            var = 0;
        }
        printf("Le flagV vaut: %d et il devrait valoir: %d\n", flagV, var);
        if(flagV != var)   // S'occuper de V
        {
            printf("Failed flags V\n");
            espoir = 0; 
        }
    }
    return espoir;
}

int main ()
{
    uint32_t operation = 0;
    uint32_t squeleton = 0xe0101002;
    printf("###############################################################################################\n");
    printf("Début du test: \n\n");
    // On veut remplir Op1_value registre de 32 bits de manière aléatoire

    // Préparation de la machine virtuelle
    memory mem = memory_create(0x200);
    registers reg = registers_create();
    arm_core p = arm_create(reg,mem);

    uint32_t Rn_valeur;
    uint8_t Rn_num;
    int valeur; // Valeur du décalage
    uint32_t tmp;
    int retour;

    // Préparation de l'aléatoire
    srand(time(NULL));
    uint32_t Op1_value = rand() % RAND_MAX;
    uint32_t Op2_value = rand() % RAND_MAX;
    int shift_type;

    int shift_register= rand()%9+3;
    uint32_t s_reg_content=rand() << 5;    
    int shift_value = rand()%32;
    s_reg_content= s_reg_content | shift_value; //editer reg content
    int bit_I;

    for(int opeCode=0; opeCode<16;opeCode++)
    {
        shift_type= rand()%4;
        operation= squeleton | (opeCode <<21) | (shift_type<<5) | (1 << 4) | (shift_register <<8);
        printf("OpCode = %d\n", opeCode );
        printf("shift type = %x\n",shift_type);
        printf("shift_register = R%d\n",shift_register);
        printf("shift_value (stored in the register mentoned above): %d\n",shift_value);


        arm_write_register(p,shift_register,s_reg_content);


        bit_I = get_bit(operation,25);

        Op1_value = rand() % RAND_MAX;
        Op2_value = rand() % RAND_MAX;

        arm_write_register(p,0,Op1_value); // Rn
        arm_write_register(p,1,1); // Rd
        arm_write_register(p,2,Op2_value); // Rm

        printf("opérande1 Rn: %u\n", arm_read_register(p,0));
        printf("opérande2 Rm = %u\n\n",  arm_read_register(p,2));
        int old_flagC = get_bit(arm_read_cpsr(p),C);

        if (bit_I == 1)
        {
            retour = arm_data_processing_immediate_msr(p,operation);
            printf("Retour de la fonction arm_data_processing_immediate_msr: ");
            print_test(retour);
            printf("\n");

            uint32_t Rm_valeur = get_bits(operation,7,0); // Je récupère la valeur de l'immédiat I utilisée comme second opérande
            valeur = (int) get_bits(operation,11,8) * 2;	// On récupère la valeur du décalage
            Rm_valeur = Effectuer_Decalage(ROR, valeur, Rm_valeur);
            // Notre 2nd opérande est à jour --> Actuellement stocké dans la Variable valeur_i (uint8_t)
            Rn_num = (uint8_t) get_bits(operation,19,16);
            Rn_valeur = arm_read_register(p,Rn_num);
            tmp = (uint32_t) Rm_valeur;
        }
        else
        {
            retour = arm_data_processing_shift(p,operation);
            printf("Retour de la fonction arm_data_processing_shift: ");
            print_test(!retour);
            printf("\n");

            uint8_t Rm_num = (uint8_t) get_bits(operation,3,0); // Je récupère le numéro du registre Rm qui correspond au registre utilisé comme second opérande.
            uint32_t Rm_valeur = arm_read_register(p, Rm_num); // Je stocke la valeur contenue dans Rm
            int decalage = Determiner_Decalage(get_bit(operation,6),get_bit(operation,5));	//Type de décalage
            printf("===========================================================================\n");
            printf("Decalage: %d\n", decalage);

            uint8_t bit_4 = (uint8_t) get_bit(operation,4);
            // La valeur du bit 4 permet de savoir si la valeur du décalage est donnée dans Op1_value registre (bit4 = 1) ou par valeur immédiate (bit4 = 0) 
            if ( bit_4 == 0 ) // La valeur immédiate du décalge se trouve dans les bits 7 à 11.
            {
                valeur = (int) get_bits(operation,11,7); //
                printf("Valeur du décalage: %d\n", valeur);
            }
            else // bit_4 == 1		La valeur du décalage correspond aux 5 bits de poids faible du registre qui a pour numero les bits 8 à 11.
            {
                uint8_t Rs_num = (uint8_t) get_bits(operation,11,8); // Je récupère le numéro du registre Rs.
                uint32_t Rs_valeur = arm_read_register(p,Rs_num); // Je stocke la valeur contenue dans Rs
                valeur = (int) get_bits(Rs_valeur,4,0);	//Je récupère les 5 bits de poids faible de Rs
            }
            Rm_valeur = Effectuer_Decalage(decalage, valeur ,Rm_valeur);
            printf("opérande2 après le décalage: %u\n\n", Rm_valeur);

            //Notre 2nd opérande est à jour ---> Actuellement stocké dans la VARIABLE Rm_valeur (uint32_t)

            Rn_num = (uint8_t) get_bits(operation,19,16);
            Rn_valeur = arm_read_register(p,Rn_num);
            tmp = (uint32_t) Rm_valeur;
        }


        uint32_t Rd_num = get_bits(operation, 15, 12); // Numéro du registre destination Rd
        uint64_t temp;
        uint32_t res2 = arm_read_register(p, Rd_num);
        int bit_S = (int) get_bit(operation,20);

        int espoir;

        switch (opeCode)
            {
                case 0:	// AND 0000 Rd <= Rn AND opérande2
                    temp = (uint64_t)((uint64_t)Rn_valeur & (uint64_t)tmp);
                    printf("Test AND %u | %u \n", Rn_valeur, tmp);
                    printf("Resultat test: %u\n", (uint32_t)temp);
                    printf("Resultat prgrm: %u\n", res2);
                    print_test((uint32_t)temp == res2);
                    if (bit_S == 1)
                    {
                        printf("Test flags cas 0\n");
                        espoir = Flags_Test(0, 0, p, temp, Rd_num);
                        print_test(espoir);
                    }
                    break;

                case 1:	// EOR 0001 Rd <= Rn XOR opérande2

                    temp = (uint64_t)((uint64_t)Rn_valeur ^ (uint64_t)tmp);
                    printf("Test XOR %u ^ %u \n", Rn_valeur, tmp);
                    printf("Resultat test: %u\n", (uint32_t)temp);
                    printf("Resultat prgrm: %u\n", res2);
                    print_test((uint32_t)temp == res2);
                    if (bit_S == 1)
                    {
                        printf("Test flags cas 1\n");
                        espoir = Flags_Test(0, 0, p, temp, Rd_num);
                        print_test(espoir);
                    }
                    break;

                case 2:	// SUB 0010 Rd <= Rn - opérande2
                    temp = (uint64_t)((uint64_t)Rn_valeur - (uint64_t)tmp);
                    printf("Test SUB %u - %u \n", Rn_valeur, tmp);
                    printf("Resultat test: %u\n", (uint32_t)temp);
                    printf("Resultat prgrm: %u\n", res2);
                    print_test((uint32_t)temp == res2);
                    if (bit_S == 1)
                    {
                        printf("Test flags cas 2\n");
                        espoir = Flags_Test(1, 1, p, temp, Rd_num);
                        print_test(espoir);
                    }
                    break;

                case 3:	// RSB 0010 Rd <= opérande2 - Rn
                    temp =(uint64_t)((uint64_t)tmp - (uint64_t)Rn_valeur);
                    printf("Test RSB %u - %u \n", tmp, Rn_valeur);
                    printf("Resultat test: %u\n", (uint32_t)temp);
                    printf("Resultat prgrm: %u\n", res2);
                    print_test((uint32_t)temp == res2);
                    if (bit_S == 1)
                    {
                        printf("Test flags cas 3\n");
                        espoir = Flags_Test(1, 1, p, temp, Rd_num);
                        print_test(espoir);
                    }
                    break;

                case 4:	// ADD 0100 Rd <= Rn + opérande2
                    temp =(uint64_t)((uint64_t)Rn_valeur + (uint64_t)tmp);
                    printf("Test ADD case 4 %u + %u\n", Rn_valeur, tmp);
                    printf("Resultat test: %u\n", (uint32_t)temp);
                    printf("Resultat prgrm: %u\n", res2);
                    print_test((uint32_t)temp == res2);
                    if (bit_S == 1)
                    {
                        printf("Test flags cas 4\n");
                        espoir = Flags_Test(1, 1, p, temp, Rd_num);
                        print_test(espoir);
                    }
                    break;

                case 5:	// ADC 0101 Rd <= Rn + opérande2 + C
                    temp =(uint64_t)((uint64_t)Rn_valeur + (uint64_t)tmp + (uint64_t)old_flagC);
                    printf("Test ADC case 5 %u + %u + %u\n", Rn_valeur, tmp, old_flagC);
                    printf("Resultat test: %u\n", (uint32_t)temp);
                    printf("Resultat prgrm: %u\n", res2);
                    print_test((uint32_t)temp == res2);
                    if (bit_S == 1)
                    {
                        printf("Test flags cas 5\n");
                        espoir = Flags_Test(1, 1, p, temp, Rd_num);
                        print_test(espoir);
                    }
                    break;

                case 6:	// SBC 0110 Rd <= Rn - opérande2 + C - 1
                    temp =(uint64_t)((uint64_t)Rn_valeur - (uint64_t)tmp + (uint64_t)old_flagC - (uint64_t)1);
                    printf("Test SBC %u - %u + %u - 1 \n", Rn_valeur,tmp, old_flagC);
                    printf("Resultat test: %u\n", (uint32_t)temp);
                    printf("Resultat prgrm: %u\n", res2);
                    print_test((uint32_t)temp == res2);
                    if (bit_S == 1)
                    {
                        printf("Test flags cas 6\n");
                        espoir = Flags_Test(1, 1, p, temp, Rd_num);
                        print_test(espoir);
                    }
                    break; 

                case 7:	// RSC 0111 Rd <= opérande2- Rn + C - 1
                    temp =(uint64_t)((uint64_t)tmp - (uint64_t)Rn_valeur + (uint64_t)old_flagC - (uint64_t)1);
                    printf("Test RSC %u - %u + %u - 1 \n",tmp, Rn_valeur, old_flagC);
                    printf("Resultat test: %u\n", (uint32_t)temp);
                    printf("Resultat prgrm: %u\n", res2);
                    print_test((uint32_t)temp == res2);
                    if (bit_S == 1)
                    {
                        printf("Test flags cas 7\n");
                        espoir = Flags_Test(1, 1, p, temp, Rd_num);
                        print_test(espoir);
                    }
                    break;  

                case 8:	// TST 1000 Positionne les flags pour Rn AND Opérande2
                    temp =(uint64_t)((uint64_t)Rn_valeur & (uint64_t)tmp);
                    printf("Test TST %u & %u\n", Rn_valeur, tmp);
                    printf("Resultat: %u\n", (uint32_t)temp);
                    printf("Test flags cas 8\n");
                    espoir = Flags_Test(0, 0, p, temp, Rd_num);
                    print_test(espoir);
                    break;

                case 9:	// TEQ 1001 Positionne les flags pour Rn XOR Opérande2
                    temp = (uint64_t)((uint64_t)Rn_valeur ^ (uint64_t)tmp);
                    printf("Test TEQ %u ^ %u\n", Rn_valeur, tmp);
                    printf("Resultat: %u\n", (uint32_t)temp);
                    printf("Test flags cas 9\n");
                    espoir = Flags_Test(0, 0, p, temp, Rd_num);
                    print_test(espoir);
                    break;
    
                case 10: // CMP 1010 Positionne les flags pour Rn - Opérande2
                    temp = ((uint64_t)Rn_valeur - (uint64_t)tmp);
                    printf("Test CMP %u - %u\n", Rn_valeur, tmp);
                    printf("Resultat: %u\n", (uint32_t)temp);
                    printf("Test flags cas 10\n");
                    espoir = Flags_Test(1, 1,  p, temp, Rd_num);
                    print_test(espoir);
                    break;

                case 11: // CMN 1011 Positionne les flags pour Rn + Opérande2
                    temp = (uint64_t) ((uint64_t)Rn_valeur + (uint64_t)tmp);
                    printf("Test CMN %u + %u\n", Rn_valeur, tmp);
                    printf("Resultat: %u\n", (uint32_t)temp);
                    printf("Test flags cas 11\n");
                    espoir = Flags_Test(1, 1, p, temp, Rd_num);
                    print_test(espoir);
                    break;

                case 12: // ORR 1100 Rd <= Rn OR opérande2
                    temp =(uint64_t)((uint64_t)Rn_valeur | (uint64_t)tmp);
                    printf("Test ORR %u | %u\n", Rn_valeur, tmp);
                    printf("Resultat test: %u\n", (uint32_t)temp);
                    printf("Resultat prgrm: %u\n", res2);
                    print_test((uint32_t)temp == res2);
                    if (bit_S == 1)
                    {
                        printf("Test flags cas 12\n");
                        espoir = Flags_Test(0, 0, p, temp, Rd_num);
                        print_test(espoir);
                    }
                    break;

                case 13: // MOV Rd <= opérande2
                    temp = (uint64_t)tmp;
                    printf("Test MOV :\n");
                    print_test((uint32_t)temp == res2);
                    break;

                case 14: // BIC Rd <= Rn AND NOT opérande2
                    temp =(uint64_t)((uint64_t)Rn_valeur & (~((uint64_t)tmp)));
                    printf("Test BIC %u & ~%u\n", Rn_valeur, tmp);
                    printf("Resultat test: %u\n", (uint32_t)temp);
                    printf("Resultat prgrm: %u\n", res2);
                    print_test((uint32_t)temp == res2);
                    if (bit_S == 1)
                    {
                        printf("Test flags cas 14\n");
                        espoir = Flags_Test(0, 0, p, temp, Rd_num);
                        print_test(espoir);
                    }
                    break; 

                case 15: // MVN Rd <= NOT opérande2
                    temp = (~(uint64_t)tmp);
                    printf("Test NOT MOV:\n");
                    print_test((uint32_t)temp == res2);
                    break;   

                default:	//On ne fait rien
                    return 1;	// Nous ne sommes pas censés arriver dans ce cas
            }
        printf("###############################################################################################\n\n");
    }
    printf("Fin du test\n");
    memory_destroy(mem);
    registers_destroy(reg);
    arm_destroy(p);
    return 0;
}  