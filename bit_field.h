#pragma once

#define INIT_SIZE_BF 1

typedef struct {
    unsigned int n;
    unsigned char* a;
}bit_field;

int contains(bit_field* m, int x);
void init(bit_field* m);
void add(bit_field* m, int x);
void del(bit_field* m, int x);
void print(bit_field* m);