#include "bit_field.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//initializeaza un bit_field
void init(bit_field* m)
{
    //se aloca INIT_SIZE_BF bytes de memorie
    m->a = (unsigned char*)calloc(INIT_SIZE_BF, sizeof(unsigned char));
    m->n = 8 * INIT_SIZE_BF;
}

void add(bit_field* m, int x)
{
    //daca x este mai mare decat numarul maxim care poate fi stocat
    //se realocheaza spatiul pentru bit_field
    if (x >= m->n) {
        m->a = realloc(m->a, (x / 8 + 1) * sizeof(unsigned char));

        //spatiul nou este initializat cu 0
        memset(m->a + m->n / 8, 0, x / 8 + 1 - m->n / 8);

        //se actualizeaza valoarea lui n
        m->n = (x / 8 + 1) * 8;
    }
    //se foloseste o masca cu 1 pe pozitia lui x
    //si 0 in rest care se aplica pe byte-ul in care trebuie sa fie x
    (*m).a[x / 8] |= (1 << (x % 8));
}

void del(bit_field* m, int x)
{
    //daca numarul cautat este mai mare decat
    //numarul maxim care poate fi stocat functia nu are efect
    if (x >= m->n) {
        return;
    }

    //se foloseste o masca cu 0 la pozitia lui x
    //si 1 in rest care se aplica pe byte-ul in care este x
    //valoarea de pe pozitia lui x devenind 0
    m->a[x / 8] &= (~(1 << (x % 8)));
}

int contains(bit_field* m, int x)
{
    //daca numarul cautat este mai mare decat
    //numarul maxim posibil de elemente se returneaza 0
    if (x >= m->n) {
        return 0;
    }
    //se foloseste o masca cu 1 pe pozitia lui x
    //si 0 in rest care se aplica pe byte-ul in care este x
    return (m->a[x / 8] & (1 << (x % 8))) != 0;
}

void print(bit_field* m)
{
    int i, j;
    for (i = 0;i < m->n / 8;i++) {
        for (j = 0;j < 8;j++) {
            //daca numarul se afla in bit_field este afisat
            if (contains(m, i * 8 + j)) {
                printf("%d ", i * 8 + j);
            }
        }
    }
    printf("\n");
}
