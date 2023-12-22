#include <stdlib.h>
#include <stdio.h>
#include "arm_data_processing.h"

int main()
{
    uint32_t r = 32;
    printf("Valeur de r: ");
    scanf("%x", &r);
    printf("\n");
    printf("Décalage à effectuer: ");
    int decalage = 2;
    scanf("%d", &decalage);
    printf("\n");
    printf("Valeur de val: ");
    int val = 2;
    scanf("%d", &val);
    printf("\n");
    printf("init = %d\n", r);
    r = Effectuer_Decalage(decalage, val, r);
    printf("res = %d\n", r);
    return 1;
}