CCFLAGS = -Wall -pedantic -Wno-unused-function -std=c++17 -I. -g
LIB=-L../lib/ -last
filename = parser
OFILES=y.tab.o lex.yy.o

all:	libparser.a

libparser.a:	$(OFILES)
	ar cr ../lib/libparser.a $(OFILES)

lex.yy.o:	$(filename).l $(filename).y
	yacc -d -v --debug -Wcounterexample $(filename).y
	lex $(filename).l
	g++ $(CCFLAGS) -c -o lex.yy.o lex.yy.c $(LIB)

y.tab.o:	$(filename).y $(filename).h
	g++ $(CCFLAGS) -c -o y.tab.o y.tab.c $(LIB)

$(filename).h:	$(filename).y
	yacc -d $(filename).y

clean:
	rm -rf lex.yy.c y.tab.c y.tab.h $(OFILES) ../lib/libparser.a y.output
