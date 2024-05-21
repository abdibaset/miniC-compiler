# variable definitions
CCFLAGS=-Wall -pedantic -std=c++17 -I. -g
OFILES=ast.o

# static library
all:	$(OFILES)
		ar cr ../lib/libast.a $(OFILES)

%.o:	%.c *.h
		g++ $(CCFLAGS) -c $<

clean:
		rm -f *.o ../lib/libast.a
