CCFLAGS = -Wall -pedantic -Wno-unused-function -std=c++11 -I. -g
# static library libutils
LIB=-L../lib/ -lutils
filename=parser

$(filename).out:	$(filename).l $(filename).y semantic_analysis.c
					yacc -d -v --debug $(filename).y
					lex $(filename).l
					g++ $(CCFLAGS) -o $(filename).out lex.yy.c y.tab.c semantic_analysis.c $(LIB)


clean:
		rm lex.yy.c y.tab.c y.tab.h $(filename).out y.output  