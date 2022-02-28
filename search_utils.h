#pragma once
#include "bit_field.h"
#include "read_utils.h"

//structura folosita pentru a retine
//cuvintele care sunt implicate intr-o cautare
typedef struct {
    char** mat;
    int size, count;
} token_list;

token_list* get_search_tokens(char* str);
token_list* parse_search_tokens(token_list* list);
void get_selection(token_list* word_list, site* sites, int site_count, bit_field* bf, int* nr_elems);
void remove_selection(token_list* word_list, site* sites, int site_count, bit_field* bf, int* nr_elems);
void get_indices(bit_field* bf, int* indices);
int compare_hits(site* s1, site* s2);
int compare_mixed(site* s1, site* s2);
void sort_indices(site* sites, int* indices, int count, int (*cmp)(site*, site*));
void display_results(site* sites, int* indices, int count);
void cleanup_token_list(token_list* tl);