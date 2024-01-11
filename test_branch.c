#include <stdlib.h>
#include <stdio.h>
#include <time.h>
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
    int debut,l,nb,fin = 0,test,fail=0;
    srand(time(NULL));
    uint32_t ins; //0xEA00000000 pour L = 0
    printf("quel test voulez vous effectuer?\n");
    scanf("%d",&test);
    while (fin<10000){
        l=0;
        debut = ((rand()%0x200)/4)*4;
        nb = rand()%0xFFFFF;
        switch (fin%3)
        {
        case BL: // 0
            l=1;
        case B: // 1
            ins = creation_ins(l,nb,debut,p,0);
            arm_branch(p,ins);
            printf("debut:%d nb:%d\n",debut,nb);
            printf("contenue du reg15: %d\ncontenue du reg14: %d\n",arm_read_register(p,15)-4,arm_read_register(p,14));
            if(!((arm_read_register(p,15)-4) == (debut + nb*4))){printf("test fail reg15:%d\n",(debut + nb*4));fail++;}
            if(l && !((arm_read_register(p,14) == debut+4))){printf("test fail reg14:%d\n",debut+4);fail++;}
            printf("##########################################################\n");
            break;

        case BLX: // 2 
            l = rand()%2;
            ins = creation_ins(l,nb,debut,p,0);
            ins = set_bit(ins,28);
            arm_branch(p,ins);
            printf("contenue du reg15: %d\ncontenue du reg14: %d\n",arm_read_register(p,15)-4,arm_read_register(p,14));
            if(!((arm_read_register(p,15)-4) == (debut + nb*4))){printf("test fail reg15:%d\n",(debut + nb*4));fail++;}
            if(!(arm_read_register(p,14) == debut+4)){printf("test fail reg14:%d\n",debut+4);fail++;}
            printf("##########################################################\n");
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
        fin++;
    }
    printf("il y a eu au total %d erreur de branchement\n",fail);
    memory_destroy(mem);
    registers_destroy(reg);
    arm_destroy(p);
    return 0;
}