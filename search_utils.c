#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bit_field.h"
#include "read_utils.h"
#include "search_utils.h"


void reallocate_char_mat(token_list* tkl);
int find_token(char* str, char* token);

//pentru sirul primit la cautare, returneaza un pointer
//catre un token_list care contine fiecare keyword/token
token_list* get_search_tokens(char* str)
{
    token_list* list = (token_list*)malloc(sizeof(token_list));
    char* p;
    p = strtok(str, " \n\t");
    list->size = 3;
    list->count = 0;
    list->mat = (char**)malloc(list->size * sizeof(char*));

    //se copiaza fiecare cuvant din sirul primit la cautare in list
    while (p != NULL) {
        list->mat[list->count] = (char*)malloc((strlen(p) + 1) * sizeof(char));
        strcpy(list->mat[list->count], p);
        list->count++;

        //se realocheaza daca nu mai este suficient spatiu
        if (list->count == list->size) {
            reallocate_char_mat(list);
        }
        p = strtok(NULL, " \n\t");
    }
    return list;
}

//realocheaza matricea de caractere din cadrul unui token_list
void reallocate_char_mat(token_list* tkl)
{
    char** ptr_aux;
    ptr_aux = (char**)realloc(tkl->mat, 2 * tkl->size * sizeof(char*));

    //se verifica rezultatul realocarii
    if (!ptr_aux) {
        fprintf(stderr, "Eroare la realocare get_search_tokens");
        exit(1);
    } else {
        tkl->mat = ptr_aux;
        tkl->size *= 2;
    }
}

//parseaza lista de token-uri, concatenandu-le pe cele dintre ghilimele
token_list* parse_search_tokens(token_list* list)
{
    token_list* aux = (token_list*)malloc(sizeof(token_list));
    aux->count = 0;
    aux->size = 3;
    aux->mat = (char**)malloc(list->size * sizeof(char*));
    char str_aux[200];
    int i, j;
    for (i = 0;i < list->count;i++) {

        //daca este token normal(fara ghilimele)
        //se adauga la lista asa cum e
        if (list->mat[i][0] != '\"') {
            aux->mat[aux->count] = malloc((1 + strlen(list->mat[i])) * sizeof(char));
            strcpy(aux->mat[aux->count], list->mat[i]);
        } else {

            //daca incepe cu '\"' se copiaza intr-un sir auxiliar
            strcpy(str_aux, list->mat[i] + 1);

            //se parcurg urmatoarele cuvinte pana la intalnirea celui care
            //are la final '\"' si se concateneaza cu spatiu la sirul auxiliar
            for (j = i + 1;j < list->count;j++) {
                strcat(str_aux, " ");
                strcat(str_aux, list->mat[j]);
                if (str_aux[strlen(str_aux) - 1] == '\"') {
                    str_aux[strlen(str_aux) - 1] = 0;
                    break;
                }
            }

            //se copiaza sirul auxiliar in lista
            aux->mat[aux->count] = malloc((1 + strlen(str_aux)) * sizeof(char));
            strcpy(aux->mat[aux->count], str_aux);

            //se trece peste cuvintele parcurse anterior
            i = j;
        }
        aux->count++;

        //se realocheaza daca nu mai este suficient spatiu
        if (aux->count == aux->size) {
            reallocate_char_mat(list);
        }
    }
    return aux;
}

//cauta in str aparitia lui token si verifica daca token
//este cuvant de sine statator
int find_token(char* str, char* token)
{
    char* p = strstr(str, token);
    if (p == NULL) {
        return 0;
    }

    //pentru a determina daca e cuvant de sine statator se verifica
    //daca este la inceputul sirului sau e precedat de un semn de punctuatie
    if (p != str && strchr("\n\t!.,;:'\"\\/?<>-_ ", p[-1]) == NULL) {
        return 0;
    }

    //sau daca este la sfarsitul sirului sau este urmat de un semn de punctuatie
    if (p[strlen(token)] != 0
    && strchr("\n\t!.,;:'\"\\/?<>-_ ", p[strlen(token)]) == NULL) {
        return 0;
    }
    return 1;
}

//retine intr-un bit_field selectia de site-uri care contin cuvintele cautate
void get_selection(token_list* word_list, site* sites, int site_count,
bit_field* bf, int* nr_elems)
{
    int i, j;
    *nr_elems = 0;
    //se initializeaza bit_field-ul
    init(bf);
    for (i = 0;i < site_count;i++) {

        //pentru fiecare site se verifica daca contine cel putin un cuvant cautat
        for (j = 0;j < word_list->count;j++) {
            if (find_token(sites[i].content, word_list->mat[j]) == 1) {

                //caz in care se adauga la bit_field
                add(bf, i);
                (*nr_elems)++;
                break;
            }
        }
    }
}

//se filtreaza site-urile selectate
//pentru a nu contine cuvinte precedate de '-'
void remove_selection(token_list* word_list, site* sites, int site_count,
bit_field* bf, int* nr_elems)
{
    int i, j;
    for (i = 0;i < word_list->count;i++) {

        //se verifica daca cuvantul cautat incepe cu '-'
        if (word_list->mat[i][0] == '-') {
            for (j = 0;j < site_count;j++) {

                //se itereaza prin site-uri si se elimina din bitfield
                //cele care contin cuvantul cu '-'
                if(find_token(sites[j].content, word_list->mat[i]+1)==1){
                    del(bf, j);
                    (*nr_elems)--;
                }
            }
        }
    }
}

//transfera valorile din bitfield intr-un vector de indici
void get_indices(bit_field* bf, int* indices)
{
    int i, k = 0;
    for (i = 0;i < bf->n;i++) {
        if (contains(bf, i)) {
            indices[k++] = i;
        }
    }
}

//compara numarul de afisari a doua site-uri 
int compare_hits(site* s1, site* s2)
{
    return s1->hits < s2->hits ? 1 : -1;
}

//compara doua site-uri conform criteriului de la task2
int compare_mixed(site* s1, site* s2)
{
    //primul criteriu este alfabetic
    int res = strcmp(s1->content, s2->content);
    if (res != 0) {
        return res;
    }
    //pentru al doilea se apeleaza functia pentru afisari
    return compare_hits(s1, s2);
}

//implementarea unuri algoritm de sortare pentru indici
void sort_indices(site* sites, int* indices, int count, int (*cmp)(site*, site*))
{
    int i, j, aux;
    for (i = 0;i < count - 1;i++) {
        for (j = i+1 ;j < count;j++) {

            //valorile sunt interschimbate in functie de
            //rezultatul functiei de comparare
            if (cmp(&sites[indices[i]], &sites[indices[j]]) > 0) {

                //pentru a lasa intacta baza de date
                //am operat exclusiv pe vectorul de indici
                aux = indices[i];
                indices[i] = indices[j];
                indices[j] = aux;
            }
        }
    }
}

//afiseaza rezultatele in ordinea precizata in vectorul de indici
void display_results(site* sites, int* indices, int count)
{
    int i;
    for (i = 0;i < count;i++) {
        printf("%s\n", sites[indices[i]].url);
    }
}

//elibereaza memoria alocata pentru un token_list
void cleanup_token_list(token_list* tl)
{
    int i;
    for (i = 0; i < tl->count; i++) {
        free(tl->mat[i]);
    }
    free(tl->mat);
    free(tl);
}