#include <stdlib.h>
#include <stdio.h>
#include "arm_data_processing.h"
#include "command_creator.h"
#include "arm_core.h"

int main()
{
    // Préparition de la machine virtuelle
    memory mem = memory_create(0x200);
    registers reg = registers_create();
    arm_core p = arm_create(reg,mem);

    //Initialisation de la condition
    int cond; 
    printf("Valeur de cond: ");
    scanf("%d",&cond);
    printf("\n");

    //Initialisation de opCode
    int opCode;
    printf("Valeur d'opCode");
    scanf("%d",&opCode);
    printf("\n");

    //Initialisation du bit I
    int I;
    printf("Valeur de I");
    scanf("%d",&I);
    printf("\n");

    //Initialisation du bit S
    int S;
    printf("Valeur de s");
    scanf("%d",&S);
    printf("\n");

    //On récupère le numéro du registre qui contient la première opérande Rn
    int num_Rn;
    printf("Numero du registre de la première opérande Rn (entre 0 et 15) :");
    scanf("%d",&num_Rn);

    //On récupère la valeur à mettre dans Rn
    int Rn_value;
    printf("Valeur contenu dans le registre de la première opérande Rn :");
    scanf("%d",&Rn_value);
    arm_write_register(p, num_Rn, Rn_value);
    printf("\n");

    //On récupère le numero de registre qui va contenir le résultat
    int Rd_num;
    printf("Numero du registre qui va contenir le résultat Rd (entre 0 et 15) :");
    scanf("%d",&Rd_num);
    printf("\n");

    //On récupère la valeur à mettre dans Rd
    int Rd_value;
    printf("Valeur du registre qui va contenir le résultat Rd :");
    scanf("%d",&Rd_value);
    arm_write_register(p, Rd_num, Rd_value);
    printf("\n");

    if ( I == 0 )
    {
        // On s'occupe de Rm
        //On récupère le numero de registre de la seconde opérande
        int Rm_num;
        printf("Numero du registre de la seconde opérande Rm (entre 0 et 15) :");
        scanf("%d",&Rm_num);
        printf("\n");

        //On récupère la valeur à mettre dans Rd
        int Rm_value;
        printf("Valeur contenu dans le registre de la seconde opérande Rm :");
        scanf("%d",&Rm_value);
        arm_write_register(p, Rd_num, Rm_value);
        printf("\n");

        //On s'occupe de Shift
        
        //On récupère le type de décalage
        int decalage;
        printf("Type de décalage à effectuer LSL=0, LSR=1, ASR=2, ROR=3 : ");
        scanf("%d", &decalage);
        printf("\n");

        //On récupère le bit_4
        int bit_4;
        printf("Valeur du bit 4: ");
        scanf("%d", &bit_4);
        printf("\n");

        if ( bit_4 == 0 )
        {
            //On récupère la valeur du décalage
            int valeur_decalage;
            printf("Valeur du décalage sur 5 bits (31 max:" );
            scanf("%d", &valeur_decalage);
            printf("\n");
        }
        else // bit_4 == 1
        {
            //On met le bit 7 à 0
            int bit_7 = 0;

            //On récupère le numero du registre qui contient la valeur de décalage (5 derniers bits)
            int num_Rs;

        }

    }
    else // I==1
    {
        int rotation, val;
        printf("Decalage appliquea la valeur :");
        scanf("%d",&rotation);
        printf("Valeur de la deuxieme operande :");
        scanf("%d",&val);
        

    }


    // On vérifie que le registre Rn est à jour
    arm_write_register(p, uint8_t reg, num_Rn, Rn);
    Rn = arm_read_register(p, num_Rn);

    uint32_t ins = construire_instruction(cond, I, opCode, S, Rn ,Rd ,shifter_operand);
    prinf("init d'ins: %d\n");

    return 1;
}