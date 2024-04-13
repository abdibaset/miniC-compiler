CCFLAGS = -Wall -pedantic -std=c++11 -I. -g
# static library libutils
LIBS=-lutils
filename=parser

$(filename).out:	$(filename).l $(filename).y 
					yacc -d $(filename).y
					lex $(filename).l
					g++ $(CCFLAGS) -o $(filename).out $(LIBS) lex.yy.c y.tab.c


clean:
		rm lex.yy.c y.tab.c y.tab.h $(filename).out y.output *.o 