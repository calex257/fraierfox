#include "read_utils.h"
#include "search_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <menu.h>

#define ADVANCED 1
#define SIMPLE 0

void load_start_screen(WINDOW* win);
void load_search_bar(WINDOW* win);
void start_screen();
void search_screen(WINDOW* wnd, site* sites, int count);
int result_screen(WINDOW* wnd, int mode, char* input, site* sites, int site_count);
int page_screen(site* page);
int* run_search(int mode, site* sites, char* search_str, int* index_count, int site_count);


int main()
{
	initscr();

	//se verifica daca terminalul suporta culori
	if (has_colors() == FALSE) {
		endwin();
		printf("Terminalul nu suporta culori\n");
		exit(1);
	}

	//se pregatesc setarile pentru ecranul de pornire
	start_color();
	noecho();
	curs_set(0);

	//acest set de culori va fi folosit pe tot parcursul programului
	//ca si mod default
	init_pair(1, COLOR_BLACK, COLOR_WHITE);

	//browser-ul incepe cu afisarea ecranului de pornire
	start_screen();
	endwin();
	return 0;
}

int page_screen(site* page)
{
	int midx, maxc, maxl, ch;

	//se creeaza o noua fereastra care sa se suprapuna peste cea cu meniul
	WINDOW* wnd = newwin(0, 0, 0, 0);
	wclear(wnd);
	getmaxyx(wnd, maxl, maxc);
	wbkgd(wnd, COLOR_PAIR(1));

	//se creeaza o noua pereche de culori
	//conform cu cele aflate in baza de date pentru site-ul ales
	init_pair(2, page->fg_color, page->bg_color);

	//se afiseaza legenda
	wattron(wnd, COLOR_PAIR(1));
	mvwprintw(wnd, maxl - 1, 3, "b -> inapoi |#| q -> iesire");

	//se afiseaza titlul in mijlocul paginii pe latime
	//titlul este boldat
	midx = (maxc - strlen(page->title)) / 2;
	wattron(wnd, A_BOLD);
	mvwprintw(wnd, 2, midx, page->title);
	wattroff(wnd, A_BOLD);

	//se aplica culorile specifice site-ului si se afiseaza continutul
	wattron(wnd, COLOR_PAIR(2));
	mvwprintw(wnd, 4, 0, page->content);
	wrefresh(wnd);

	//se asteapta input-ul utilizatorului
	//pana cand acesta reprezinta o comanda recunoscuta in acest mod
	do {
		ch = wgetch(wnd);
	} while (strchr("bq", ch) == NULL);
	wattroff(wnd, COLOR_PAIR(2));
	delwin(wnd);

	//se transmite rezultatul functiei la functia precedenta(result_screen)
	//pentru a se decide daca programul trebuie inchis sau nu
	if (ch == 'q') {
		return 0;
	}
	return 1;
}

int result_screen(WINDOW* wnd, int mode, char* input, site* sites, int site_count)
{
	int ch, maxl, maxc, curr_index, running = 1, index_count,
		* indices, bar_pos = 1, margin = 4;
	WINDOW* sec_wnd;
	getmaxyx(wnd, maxl, maxc);

	//se creeaza o subfereastra pentru meniu(requirement de la ncurses)
	sec_wnd = derwin(wnd, maxl - 10, maxc - 10, 5, 4);
	wclear(wnd);
	MENU* menu;
	ITEM** items;

	//se realizeaza bara de cautare folosindu-se coordonate relative
	//la latimea si inaltimea terminalului
	mvwhline(wnd, bar_pos, margin, '-', maxc - 2 * margin);
	mvwhline(wnd, bar_pos + 1, margin + 1, ' ', maxc - 2 * margin - 1);
	mvwhline(wnd, bar_pos + 2, margin, '-', maxc - 2 * margin);
	mvwprintw(wnd, bar_pos + 1, margin, "|");
	mvwprintw(wnd, bar_pos + 1, maxc - margin, "|");

	//se afiseaza sirul cautat si legenda
	mvwprintw(wnd, bar_pos + 1, margin + 1, "%s", input);
	mvwprintw(wnd, maxl - 1, 3, "sagetile sus si jos -> navigarea prin meniu |#| ENTER -> selectarea optiunii "
		"|#| b -> inapoi |#| q -> iesire");

	//se realizeaza cautarea dupa modul selectat(avansat sau simplu)
	indices = run_search(mode, sites, input, &index_count, site_count);

	//se aloca spatiu pentru item-urile din meniu
	items = (ITEM**)malloc((1 + index_count) * sizeof(ITEM*));
	for (int i = 0;i < index_count;i++) {

		//fiecare item este initializat cu url-ul si titlul site-ului aferent
		items[i] = new_item(sites[indices[i]].url, sites[indices[i]].title);
	}
	items[index_count] = NULL;

	//se creeaza un meniu cu lista de item-uri
	menu = new_menu(items);

	//se foloseste keypad pentru a putea primi sagetile ca input
	keypad(wnd, 1);
	noecho();
	curs_set(0);
	wbkgd(wnd, COLOR_PAIR(1));
	wbkgd(sec_wnd, COLOR_PAIR(1));
	wattron(wnd, COLOR_PAIR(1));
	wattron(sec_wnd, COLOR_PAIR(1));

	//se aplica configuratii pentru meniu(se modifica prompt-ul,
	//se seteaza culoarea de fundal si cea a textului)
	set_menu_win(menu, wnd);
	set_menu_sub(menu, sec_wnd);
	set_menu_mark(menu, "--->");
	set_menu_fore(menu, COLOR_PAIR(1));
	set_menu_back(menu, COLOR_PAIR(1));

	//dupa configurari se afiseaza meniul
	post_menu(menu);
	if (index_count == 0) {
		mvwprintw(wnd, 4, 4, "Cautarea nu are niciun rezultat");
	}
	while (running) {
		ch = wgetch(wnd);
		switch (ch) {

			//tastele q si b opresc executia buclei
		case 'q':
		{
			running = 0;
			break;
		}
		case 'b':
		{
			running = 0;
			break;
		}

		//pentru enter se intra in modul de afisare a unei pagini
		case '\n':
		{
			if (index_count != 0) {
				curr_index = item_index(current_item(menu));
				running = page_screen(&sites[indices[curr_index]]);
				if (running) {
					wbkgd(wnd, COLOR_PAIR(1));
					wrefresh(wnd);
					touchwin(wnd);
					wrefresh(wnd);
					post_menu(menu);
					menu_driver(menu, REQ_FIRST_ITEM);
				}
			}
			break;
		}

		//se muta cursorul in sus sau in jos in functie de input
		case KEY_UP:
		{
			menu_driver(menu, REQ_UP_ITEM);
			break;
		}
		case KEY_DOWN:
		{
			menu_driver(menu, REQ_DOWN_ITEM);
			break;
		}
		}
	}

	//se elibereaza memoria alocata in cadrul functiei
	free(indices);
	delwin(sec_wnd);
	for (int i = 0;i < index_count;i++) {
		free_item(items[i]);
	}
	free(items);
	free_menu(menu);

	//se transmite rezultatul functiei la functia precedenta(search_screen)
	//pentru a se decide daca programul trebuie inchis sau nu
	if (ch == 'b') {
		return 1;
	}
	return 0;
}

void search_screen(WINDOW* wnd, site* sites, int count)
{
	char input[100], ch;
	int running = 1;

	//se afiseaza bara de cautare si se primeste sirul de input de la utilizator
	load_search_bar(wnd);
	wgetnstr(wnd, input, 100);
	curs_set(0);
	noecho();
	while (running) {
		ch = wgetch(wnd);
		if (ch == 'q') {
			running = 0;
		} else if (ch == 'a' || ch == 's') {

			//se salveaza rezultatul primit de la modul urmator
			running = result_screen(wnd, ch == 'a' ? ADVANCED : SIMPLE, input, sites, count);
			
			//daca acesta nu este 0 se reinitializeaza pagina de cautare
			if (running) {
				load_search_bar(wnd);
				wgetnstr(wnd, input, 100);
				curs_set(0);
				noecho();
				touchwin(wnd);
				wrefresh(wnd);
			}
		}
	}
}

void start_screen()
{
	int ch, count, size;

	//se creeaza o noua fereastra care va fi utilizata in primele 3 moduri
	//(start, search si results)
	WINDOW* wnd;
	wnd = newwin(0, 0, 0, 0);
	noecho();
	curs_set(0);
	wattron(wnd, COLOR_PAIR(1));
	wbkgd(wnd, COLOR_PAIR(1));

	//se initializeaza ecranul de cautare si baza de date
	load_start_screen(wnd);
	site* sites = db_init("master.txt", &count, &size);

	//se asteapta inputul utilizatorului pana cand
	//se intalneste o comanda recunoscuta in acest mod
	do {
		ch = wgetch(wnd);
	} while (strchr("cq", ch) == NULL);
	if (ch != 'q') {
		search_screen(wnd, sites, count);
	}

	//la finalul executiei se elibereaza memoria aferenta
	delwin(wnd);
	cleanup(sites, count);
}

//in functie de modul ales aceasta functie opereaza precum task2 sau task3
int* run_search(int mode, site* sites, char* search_str, int* index_count, int site_count)
{
	int* indices;
	token_list* words = get_search_tokens(search_str), * search_items;
	bit_field bf;

	//daca modul este avansat se parseaza lista de token-uri
	//pentru cuvintele cu "", altfel se opereaza pe lista normala
	if (mode == ADVANCED) {
		search_items = parse_search_tokens(words);
		cleanup_token_list(words);
	}

	//daca modul este cautare avansata se cauta site-urile si se
	//inlatura cele care contin cuvinte cu '-' in fata
	if (mode == ADVANCED) {
		get_selection(search_items, sites, site_count, &bf, index_count);
		remove_selection(search_items, sites, site_count, &bf, index_count);
	} else {

		//altfel se ruleaza o cautare normala
		get_selection(words, sites, site_count, &bf, index_count);
	}

	//se retin indicii pentru site-urile gasite
	indices = (int*)malloc(*index_count * sizeof(int));
	get_indices(&bf, indices);

	//se sorteaza dupa criteriile specifice fiecarui task
	if (mode == ADVANCED) {
		sort_indices(sites, indices, *index_count, compare_hits);
	} else {
		sort_indices(sites, indices, *index_count, compare_mixed);
	}

	//se elibereaza memoria aferenta
	free(bf.a);
	if (mode == ADVANCED) {
		cleanup_token_list(search_items);
	} else {
		cleanup_token_list(words);
	}

	//se returneaza vectorul de indici pentru a fi folosit la meniu
	return indices;
}

void load_start_screen(WINDOW* win)
{
	//ASCII art pentru logo
	const char logo[6][51] = {
				  "______            _               __            ",
				  "|  ___|          (_)             / _|           ",
				  "| |_  _ __  __ _  _   ___  _ __ | |_  ___ __  __",
				  "|  _|| '__|/ _` || | / _ \\| '__||  _|/ _ \\\\ \\/ /",
				  "| |  | |  | (_| || ||  __/| |   | | | (_) |>  < ",
				  "\\_|  |_|   \\__,_||_| \\___||_|   |_|  \\___//_/\\_\\" },
				  version[] = "v 1.0.0 early access",
				  author_name[] = "Firstname Lastname";
	int y, x, mid_y, mid_x, i;

	//se obtin inaltimea si latimea ferestrei si
	//pentru fiecare se calculeaza mijlocul
	getmaxyx(win, y, x);
	mid_y = y / 2 - 4;
	mid_x = x / 2 - strlen(logo[0]) / 2;

	//pentru ca se intinde pe mai multe linii
	//logo-ul se afiseaza linie cu linie
	for (i = 0;i < 6;i++) {
		mvwprintw(win, mid_y + i, mid_x, logo[i]);
	}

	//se afiseaza numele si versiunea
	mvwprintw(win, 0, x - strlen(version), version);
	mvwprintw(win, 0, 0, author_name);

	//se afiseaza legenda
	mvwprintw(win, y - 1, 3, "c -> cautare |#| q -> inchidere");
	wrefresh(win);
}

void load_search_bar(WINDOW* win)
{
	int y, x, mid_y, margin = 5;

	//se goleste fereastra si se obtin latimea si inaltimea
	wclear(win);
	getmaxyx(win, y, x);
	mid_y = y / 2 - 3;

	//se aplica setul de culori pentru fundal si scris
	wbkgd(win, COLOR_PAIR(1));
	wattron(win, COLOR_PAIR(1));

	//se afiseaza legenda
	mvwprintw(win, y - 1, 3, "ENTER -> blocheaza cautarea |#| a -> cautare avansata "
		"|#| s -> cautare simpla |#| q -> iesire");

	//se realizeaza bara de cautare folosindu-se coordonate relative
	//la latimea si inaltimea terminalului
	mvwhline(win, mid_y, margin, '-', x - 2 * margin);
	mvwhline(win, mid_y + 1, margin + 1, ' ', x - 2 * margin - 1);
	mvwhline(win, mid_y + 2, margin, '-', x - 2 * margin);
	mvwprintw(win, mid_y + 1, margin, "|");
	mvwprintw(win, mid_y + 1, x - margin, "|");

	//se face cursorul vizibil si se permite scrierea pe ecran
	//a inputului venit de la utilizator
	curs_set(2);
	echo();

	//se muta cursorul la inceputul barei de cautare
	wmove(win, mid_y + 1, margin + 1);
}