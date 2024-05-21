#ifndef IR_BUILDER_H
#define IR_BUILDER_H

#include "../ast/ast.h"
#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Types.h>
#include <stdio.h>

LLVMModuleRef build_ir(astNode *root, const char *filename);

#endif // IR_BUILDER_H
