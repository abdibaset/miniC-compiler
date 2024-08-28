#ifndef REGISTER_ALLOCATION_H
#define REGISTER_ALLOCATION_H

#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Types.h>

void generate_assembly_code(const char *filename, LLVMModuleRef moduleReference);

#endif // REGISTER_ALLOCATION_H