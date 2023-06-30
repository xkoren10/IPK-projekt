# IPK projekt 1, Matej Koreň [xkoren10]

PRJ=hinfosvc
#
PROGS=$(PRJ)
CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -pedantic -lm

.PHONY: run clean 

all: $(PROGS)

$(PRJ): 
	$(CC) $(CFLAGS) -o $@ $(PRJ).c -lm

clean:
	rm -f *.o $(PROGS)
#