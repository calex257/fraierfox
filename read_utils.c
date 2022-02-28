#include "read_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void form_tag(char* tag, char* inner_html, int endtag);
char* get_inner_text(char* src, char* element, int* length);
void pick_color(site* data, char* src, int elm);

site* db_init(char* file_list, int* cnt, int* sz)
{
    int size = INIT_SIZE, count = 0;
    char file_name[FILE_NAME_MAX_DIM], * raw_data;
    site* sites = (site*)malloc(size * sizeof(site)), * ptr_aux;
    FILE* master = fopen("master.txt", "r"), * input;

    //se itereaza prin continutul fisierului master.txt
    while (fgets(file_name, FILE_NAME_MAX_DIM, master) != NULL) {

        //se suprascrie '\n' de la finalul sirului daca exista
        if (file_name[strlen(file_name) - 1] == '\n') {
            file_name[strlen(file_name) - 1] = 0;
        }
        input = fopen(file_name, "r");
        if (!input) {
            fprintf(stderr, "eroare la deschiderea fisierului");
            exit(1);
        }

        //se citesc datele pentru site-ul curent
        read_header(input, &sites[count]);

        //continutul de la <html> la </html> este stocat in raw_data
        raw_data = read_raw_data(input, sites[count].size);
        sites[count].html=raw_data;
        get_colors(&sites[count], raw_data);
        get_title(&sites[count], raw_data);
        get_content(&sites[count], raw_data);
        count++;

        //daca nu mai este spatiu in vector se realocheaza
        if (count == size) {
            ptr_aux = realloc(sites, (SIZE_STEP + size) * sizeof(site));
            
            //se verifica rezultatul realocarii
            if (ptr_aux == NULL) {
                fprintf(stderr, "Eroare la realocarea memoriei");
                free(sites);
                exit(1);
            } else {
                sites = ptr_aux;
            }

            //se actualizeaza dimensiunea maxima
            size += SIZE_STEP;
        }
        fclose(input);
    }
    fclose(master);
    *cnt = count;
    *sz = size;
    return sites;
}

//parseaza primul rand(header) din fisierul input
void read_header(FILE* input, site* data)
{
    int res;

    //se citesc informatiile conform tipului lor
    res = fscanf(input, "%200s%u%u%hhu", data->url, &data->size, &data->hits, &data->checksum);
    
    //se verifica daca toate cele 4 informatii au fost citite cu succes
    if (res != 4) {
        fprintf(stderr, "eroare la citirea antetului");
        exit(1);
    }

    //se trece peste trailing whitespaces si '\n' pentru ca urmatoarea citire
    //sa aiba loc pe randul urmator
    while(fgetc(input)!='\n');
}

//functie pentru citirea a size caractere din fisierul input
char* read_raw_data(FILE* input, int size)
{
    char* str = (char*)malloc((size + 1) * sizeof(char));
    //se verifica daca memoria a fost alocata corespunzator
    if (str == NULL) {
        return NULL;
    }
    //se citesc size byti din fisier si se adauga NULL la finalul sirului
    fread(str, sizeof(char), size, input);
    str[size] = 0;
    return str;
}


//parseaza textul pentru a gasi culorile pentru foreground/background
void get_colors(site* data, char* src)
{
    char* p, bg_text[] = "background-color:", fg_text[] = "color:";

    //daca tagul p nu are parametrul style se utilizeaza culorile default
    if ((p = strstr(src, "style")) == NULL) {
        data->bg_color = white;
        data->fg_color = black;
        return;
    }
    if ((p = strstr(src, bg_text)) != NULL) {
        p += strlen(bg_text);

        //se cauta inceputul keywordului care reprezinta culoarea
        while(!isalpha(p[0])){
            p++;
        }
        pick_color(data, p, BACKGROUND);
    } else {

        //daca nu se gaseste, se utilizeaza culoarea default
        data->bg_color = white;
    }
    if ((p = strstr(src, fg_text)) != NULL && p[-1] != '-') {
        p += strlen(fg_text);
        while(!isalpha(p[0])){
            p++;
        }
        pick_color(data, p, FOREGROUND);
    } else {
        data->fg_color = black;
    }
}

//obtine titlul pentru un site dat
void get_title(site* data, char* src)
{
    int nr;
    char* p;
    p = get_inner_text(src, "title", &nr);

    //se utilizeaza adresa de inceput si lungimea pentru a stoca titlul
    strncpy(data->title, p, nr);
    data->title[nr] = 0;
}

//obtine continutul tagului p
void get_content(site* data, char* src)
{
    int nr;
    char* p;
    p = get_inner_text(src, "p", &nr);
    data->content = (char*)malloc((nr + 2) * sizeof(char));
    strncpy(data->content, p, nr);
    data->content[nr] = 0;
}

//obtine textul dintre doua taguri html
char* get_inner_text(char* src, char* element, int* length)
{
    char begin_tag[7], end_tag[8], * begin, * end;

    //se formeaza tagul pentru elementul dorit
    form_tag(begin_tag, element, BEGIN_TAG);
    form_tag(end_tag, element, END_TAG);

    //se cauta in text inceputul tagului
    begin = strstr(src, begin_tag);

    //se cauta finalul tagului si apoi inceputul textului efectiv
    begin = strchr(begin, '>');
    begin++;

    //se cauta endtagul si se calculeaza lungimea textului
    end = strstr(begin, end_tag);
    *length = end - begin;

    //se returneaza adresa la care incepe testul
    return begin;
}

//asambleaza un tag html
void form_tag(char* tag, char* inner_html, int endtag)
{
    //se copiaza inceputul de tag corespunzator tipului de tag dorit
    if (endtag) {
        strcpy(tag, "</");
    } else {
        strcpy(tag, "<");
    }

    //se concateneaza elementul dorit in tag(title sau p)
    strcat(tag, inner_html);
}

//obtine culoarea pentru foregrorund/background
void pick_color(site* data, char* src, int elm)
{
    int color;

    //selectia se face dupa primul caracter
    switch (src[0]) {
    case 'w':
    {
        color = white;
        break;
    }
    case 'b':
    {
        //pentru black si blue se cauta al treilea caracter
        if (src[2] == 'a') {
            color = black;
        } else {
            color = blue;
        }
        break;
    }
    case 'r':
    {
        color = red;
        break;
    }
    case 'g':
    {
        color = green;
        break;
    }
    case 'y':
    {
        color = yellow;
        break;
    }
    default:
    {
        //daca culoarea nu este recunoscuta se afiseaza un mesaj de eroare
        fprintf(stderr, "Culoare nerecunoscuta de browser");

        //si se folosesc culorile default
        if(elm==FOREGROUND){
            color=black;
        } else {
            color=white;
        }
        break;
    }
    }
    if (elm == FOREGROUND) {
        data->fg_color = color;
    } else {
        data->bg_color = color;
    }
}


//elibereaza memoria alocata pentru baza de date
void cleanup(site* data, int length)
{
    int i;
    for (i = 0;i < length;i++) {
        free(data[i].content);
        free(data[i].html);
    }
    free(data);
}