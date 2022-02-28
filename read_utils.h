#pragma once
#include <stdio.h>
#include <string.h>

//macro-uri utile
#define STR_EQ(s1, s2) (strcmp((s1), (s2))==0)
#define max(a, b) ((a)>(b)?a:b)
#define min(a, b) ((a)<(b)?a:b)
#define sgn(x) (((x)>0)-((x)<0))

//macro-uri pentru debugging
#define LOG_STR(s) puts(s)
#define LOG_N(nr) printf("%d\n", nr)

#define STR_MAX_DIM 200
#define FILE_NAME_MAX_DIM 31
#define FOREGROUND 1
#define BACKGROUND 0
#define INIT_SIZE 3
#define SIZE_STEP 3
#define BEGIN_TAG 0
#define END_TAG 1

//pentru enum s-au folosit valorile care
//corespund cu codurile culorilor din ncurses pentru simplitate
enum colors{
    black=0, red=1, green=2, yellow=3, blue=4, white=7
};

/*
*content: continutul tagului <p>
*html: continutul integral, de la <html> la </html>
*hits: accesari
*bg_color si fg_color: culorile pentru background respectiv foreground
*/
typedef struct SITE{
    char url[STR_MAX_DIM], title[STR_MAX_DIM], *content, *html;
    unsigned int hits, size, bg_color, fg_color;
    unsigned char checksum;
} site;

char* read_raw_data(FILE* input, int size);
void read_header(FILE* input, site* data);
void get_title(site* data, char* src);
void get_content(site* data, char* src);
void get_colors(site* data, char* src);
site* db_init(char* file_list, int* cnt, int* sz);
void cleanup(site* data, int length);