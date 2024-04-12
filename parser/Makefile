CCFLAGS = -Wall -pedantic -std=c11 -I. -g
# static library libutils
LIBS=-lutils
pfilename = parser

ast:	
		gcc $(CCFLAGS) ast.c $(LIBS) -o $@
		
$(pfilename).out:	$(pfilename).l $(filename).y ast.o
					lex $(pfilename).l
					yacc lex.yy.c -o $(pfilename).out


clean:
		rm lex.yy.c y.tab.c y.tab.h $(pfilename).out y.output *.o ast 