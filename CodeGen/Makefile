CCFLAGS=-Wall -pedantic -Wno-unused-function -std=c++17 -lstdc++fs -g
LLVMPATH=/usr/include/llvm-c-15/
LLVMFLAGS=`llvm-config-15 --cxxflags --ldflags --libs core`
filename = register_allocation
OPTLI=-L../lib -lopt

all: code_gen_lib.a

code_gen_lib.a:	$(filename).o
	ar rc ../lib/libcodegen.a $(filename).o

$(filename).o:	$(filename).c $(filename).h
	g++ $(CCFLAGS) -I$(LLVMPATH) -c -o $@ $< $(LLVMFLAGS)

clean:
	rm *.o ../lib/libcodegen.a