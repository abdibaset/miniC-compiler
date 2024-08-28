CCFLAGS=-Wall -pedantic -Wno-unused-function -std=c++17
ASTLIB=-L../lib -last
OFILES=rename_variables.o variable_names.o semantic_analysis.o

all:	$(OFILES)
	ar cr ../lib/libsemantic.a $(OFILES)

rename_variables.o: rename_variables.c pre_processing.h
	g++ $(CCFLAGS) -c -I /usr/include/llvm-c-15/ -o $@ $< $(ASTLIB)

variable_names.o: variable_names.c pre_processing.h
	g++ $(CCFLAGS) -c -I /usr/include/llvm-c-15/ -o $@ $< $(ASTLIB)

semantic_analysis.o: semantic_analysis.c semantic_analysis.h
	g++ $(CCFLAGS) -c -I /usr/include/llvm-c-15/ -o $@ $< $(ASTLIB)

clean:
	rm -rf rename_variables.o variable_names.o ../lib/libsemantic.a