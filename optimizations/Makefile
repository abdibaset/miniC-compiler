CCFLAGS=-Wall -pedantic -Wno-unused-function -std=c++17 -g
LLVMPATH=/usr/include/llvm-c-15/
LLVMFLAGS=`llvm-config-15 --cxxflags --ldflags --libs core`
LOCALLLVMCODE=optimizations

COMMONSUBEXPR_TEST=./opt_tests/p2_common_subexpr
GLOBALOPT_P3_TEST=./opt_tests/p3_const_prop
GLOBALOPT_P4_TEST=./opt_tests/p4_const_prop
GLOBALOPT_P5_TEST=./opt_tests/p5_const_prop
LIB=-L../lib/ -lopt

all: $(LOCALLLVMCODE) common_subexpr_llvm_file global_opt_p3_llvm_file global_opt_p4_llvm_file global_opt_p5_llvm_file

$(LOCALLLVMCODE): $(LOCALLLVMCODE).c
	g++ $(CCFLAGS) -I$(LLVMPATH) -c $(LOCALLLVMCODE).c
	g++ $(LOCALLLVMCODE).o $(LLVMFLAGS) -I $(LLVMPATH) -o $@ $(LIB)

common_subexpr_llvm_file: $(COMMONSUBEXPR_TEST).c
	clang -S -emit-llvm $(COMMONSUBEXPR_TEST).c -o $(COMMONSUBEXPR_TEST).ll

global_opt_p3_llvm_file: $(GLOBALOPT_P3_TEST).c
	clang -S -emit-llvm $(GLOBALOPT_P3_TEST).c -o $(GLOBALOPT_P3_TEST).ll

global_opt_p4_llvm_file: $(GLOBALOPT_P4_TEST).c
	clang -S -emit-llvm $(GLOBALOPT_P4_TEST).c -o $(GLOBALOPT_P4_TEST).ll

global_opt_p5_llvm_file: $(GLOBALOPT_P5_TEST).c
	clang -S -emit-llvm $(GLOBALOPT_P5_TEST).c -o $(GLOBALOPT_P5_TEST).ll

clean:
	rm -rf *.o *.out $(COMMONSUBEXPR_TEST).ll $(GLOBALOPT_P3_TEST).ll $(GLOBALOPT_P4_TEST).ll $(GLOBALOPT_P5_TEST).ll
