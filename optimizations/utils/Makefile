
CCFLAGS=-Wall -pedantic -Wno-unused-function -std=c++11 -lstdc++fs -g
LLVMPATH=/usr/include/llvm-c-15/
LLVMFLAGS=`llvm-config-15 --cxxflags --ldflags --libs core`

# Library file names
HEADER_FILE=optimizations
COMMON_DEADCODE=common_subexpr_dead_code_elimination
CONST_FOLDING=constant_folding
GLOBAL_OPTIMIZATIONS=global_optimizations
OPT_UTILS=optimizations_utils

OFILES = $(COMMON_DEADCODE).o $(CONST_FOLDING).o $(GLOBAL_OPTIMIZATIONS).o $(OPT_UTILS).o

all: opt_lib.a

# Static library definition
opt_lib.a: $(OFILES)
	ar cr ../../lib/libopt.a $(OFILES)

$(COMMON_DEADCODE).o: $(COMMON_DEADCODE).c $(HEADER_FILE).h $(OPT_UTILS).o
	g++ $(CCFLAGS) -I$(LLVMPATH) -c -o $@ $< $(LLVMFLAGS)

$(CONST_FOLDING).o: $(CONST_FOLDING).c $(HEADER_FILE).h $(OPT_UTILS).o
	g++ $(CCFLAGS) -I$(LLVMPATH) -c -o $@ $< $(LLVMFLAGS)

$(GLOBAL_OPTIMIZATIONS).o: $(GLOBAL_OPTIMIZATIONS).c $(HEADER_FILE).h $(OPT_UTILS).o
	g++ $(CCFLAGS) -I$(LLVMPATH) -c -o $@ $< $(LLVMFLAGS)

$(OPT_UTILS).o: $(OPT_UTILS).c $(OPT_UTILS).h
	g++ $(CCFLAGS) -I$(LLVMPATH) -c -o $@ $< $(LLVMFLAGS)

clean:
		rm -rf *.o ../../lib/libopt.a