
#Declararea variabilelor

SEARCH_FILES = search_utils.c bit_field.c
C_FLAGS = -g -Wall -std=c99
NCURSES_FLAGS = -lncurses -lmenu

#Regulile pentru fiecare task
build: task1 task2 task3 task4

task1: task1.c read_utils.c
	gcc $(C_FLAGS) -o task1 task1.c read_utils.c

task2: task2.c read_utils.c $(SEARCH_FILES)
	gcc $(C_FLAGS) -o task2 task2.c read_utils.c $(SEARCH_FILES)

task3: task3.c read_utils.c $(SEARCH_FILES)
	gcc $(C_FLAGS) -o task3 task3.c read_utils.c $(SEARCH_FILES)

task4: task4.c read_utils.c hasher.c
	gcc $(C_FLAGS) -o task4 task4.c read_utils.c hasher.c

browser: browser.c read_utils.c $(SEARCH_FILES)
	gcc $(C_FLAGS) -o browser browser.c read_utils.c $(SEARCH_FILES) $(NCURSES_FLAGS)

#Regula de clean
.PHONY: clean
clean:
	rm -f *.o
	rm -f task1 task2 task3 task4 browser
