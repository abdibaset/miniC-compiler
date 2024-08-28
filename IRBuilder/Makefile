CCFLAGS = -Wall -pedantic -Wno-unused-function -std=c++11
# static library libutils
LLVMPATH=/usr/include/llvm-c-15/
LLVMFLAGS=`llvm-config-15 --cxxflags --ldflags --libs core`
OFILES=ir_builder.o

all: ir_builder_lib.a

ir_builder_lib.a:	$(OFILES)
	ar cr ../lib/libIRBuilder.a $(OFILES)

ir_builder.o:	ir_builder.c ir_builder.h
	g++ -g $(CCFLAGS) -I$(LLVMPATH) -c -o $@ $< $(LLVMFLAGS)

clean:
	rm -rf ir_builder.o ir_builder ../lib/libIRBuilder.a