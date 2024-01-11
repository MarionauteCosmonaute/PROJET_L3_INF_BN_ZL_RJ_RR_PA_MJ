#include <stdlib.h>
#include <stdio.h>
#include "arm_branch_other.h"
#include "util.h"


#define BL 0
#define B 1
#define BLX 2
#define BX 3
#define MSR 4

 


uint32_t creation_ins(int l,int nb,int debut, arm_core p, int m){
    arm_write_register(p,15,debut);
    arm_write_register(p,14,0);
    uint32_t ins;
    if(!m){
        ins = 0xEA000000;
        ins = set_bits(ins,23,0,get_bits(nb,23,0));
    }
    else{
        ins = 0xE1200000;
        ins = set_bits(ins,3,0,get_bits(nb,23,0));
    }
        
    if(l){ins = set_bit(ins,24);}
    
    return ins;
}



int main(){
    memory mem = memory_create(0x200);
    registers reg = registers_create();
    arm_core p = arm_create(reg,mem);
    int debut,l,nb,fin,test;
    uint32_t ins; //0xEA00000000 pour L = 0
    while (fin){
        printf("quel test voulez vous effectuer?\n");
        scanf("%d",&test);
        l=0;
        switch (test)
        {
        case BL: // 0
            l=1;
        case B: // 1
            printf("A quel adresse commencer:\n");
            scanf("%d",&debut);
            printf("Combien de pas a faire:\n");
            scanf("%d",&nb);
            if(nb > 0xFFFFFF){printf("ERREUR VALEUR TROP GRANDE\n");break;}
            ins = creation_ins(l,nb,debut,p,0);
            arm_branch(p,ins);
            printf("contenue du reg15: %d\ncontenue du reg14: %d\n",arm_read_register(p,15)-4,arm_read_register(p,14));
            break;

        case BLX: // 2 
            printf("A quel adresse commencer:\n");
            scanf("%d",&debut);
            printf("Combien de pas a faire:\n");
            scanf("%d",&nb);
            if(nb > 0xFFFFFF){printf("ERREUR VALEUR TROP GRANDE\n");break;}
            printf("Valeur de H:\n");
            scanf("%d",&l);
            ins = creation_ins(l,nb,debut,p,0);
            ins = set_bit(ins,28);
            arm_branch(p,ins);
            printf("contenue du reg15: %d\ncontenue du reg14: %d\n",arm_read_register(p,15)-4,arm_read_register(p,14));
            break;
        case BX: // 3
            printf("Quel adresse atterire?:\n");
            scanf("%d",&nb);
            arm_write_register(p,4,nb);
            ins = creation_ins(l,4,0,p,1);
            arm_miscellaneous(p,ins);
            printf("contenue du reg15: %d\ncontenue du reg14: %d\n",arm_read_register(p,15)-4,arm_read_register(p,14));

            break;
        case MSR: // 4
        default:
            printf("pas encore implementer\n");
            break;
        }
        printf("voulez vous continuer? 1 pour oui 0 pour non\n");
        scanf("%d",&fin);
    }
    memory_destroy(mem);
    registers_destroy(reg);
    arm_destroy(p);
    return 0;
}