# variable definitions 
CCFLAGS=-Wall -pedantic -std=c11 -I. -g
OFILES=ast.o

# static library 
all:	$(OFILES)
		ar cr ../lib/libutils.a $(OFILES) 

%.o:	%.c %.h
		gcc $(CCFLAGS) -c $<

clean:
		rm -f *.o
		