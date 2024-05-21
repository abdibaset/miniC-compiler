CCFLAGS=-Wall -pedantic -Wno-unused-function -std=c++11 -g
LLVMPATH=/usr/include/llvm-c-15/
LLVMFLAGS=`llvm-config-15 --cxxflags --ldflags --libs core`
filename=miniC_compiler

ASTLIB=-L./lib/ -last
OPTLIB=-L./lib/ -lopt
SEMANTICLIB=-L./lib/ -lsemantic
PARSERLIB=-L./lib/ -lparser
IRBUILDLIB=-L./lib -lIRBuilder

all: $(filename)

$(filename):	$(filename).c
	g++ $(CCFLAGS) -I$(LLVMPATH) -c $<
	g++ $(filename).o -I$(LLVMPATH) -o $@ $(PARSERLIB) $(ASTLIB) $(IRBUILDLIB) $(SEMANTICLIB) $(OPTLIB) -lstdc++fs $(LLVMFLAGS)

clean:
	rm -rf *.o $(filename)