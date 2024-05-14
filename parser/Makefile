CCFLAGS = -Wall -pedantic -Wno-unused-function -std=c++11 -I. -g
# static library libutils
LIB=-L../lib/ -last
filename=parser

all:	$(filename).out

$(filename).out:	$(filename).l $(filename).y parser.c
					yacc -d -v --debug -Wcounterexample $(filename).y
					lex $(filename).l
					g++ $(CCFLAGS) -o $(filename).out lex.yy.c y.tab.c parser.c $(LIB)


clean:
		rm -rf lex.yy.c y.tab.c y.tab.h $(filename).out y.output