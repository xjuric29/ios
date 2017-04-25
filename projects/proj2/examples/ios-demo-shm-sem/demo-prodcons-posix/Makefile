#**************************************************
# Makefile pro ukazkove kody do predmetu IOS na FIT VUT.
# Prace se semafory a sdilenou pameti.
# Kody jsou poskytnuty tak jak jsou.
# (C) Radek Koci, 2016
#*******************************************

NAME = demo_sync_posix
EXEC = $(NAME)
sources = $(NAME).c common.c solution.c

CXX = gcc
RM = rm -f

CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic
LDFLAGS = -lrt -pthread
#CFLAGS = -std=gnu99 -Wall -Wextra -pedantic
#CFLAGS = -std=gnu99 -Wall -pedantic
#CFLAGS = -std=gnu99 -Wall 
#CFLAGS = -std=gnu99
#-g0 -s -O3

OBJFILES = $(sources:.c=.o)

.PHONY : all

all : $(EXEC) 

%.o : %.c 
	$(CXX) $(CFLAGS) -c $< -o $@

$(EXEC) : $(OBJFILES)
	$(CXX) $(CFLAGS) -o $@ $(OBJFILES) $(LDFLAGS)

clean:
	$(RM) *.o core *.out

cleanall: clean
	$(RM) $(EXEC)
