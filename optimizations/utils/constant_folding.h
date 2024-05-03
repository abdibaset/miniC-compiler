#ifndef CONSTANT_FOLDING_H
#define CONSTANT_FOLDING_H

#include "optimizations_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Types.h>
#include <stdbool.h>
#include <algorithm>
#include <vector>
#include <cstring>
#include <map>
#include <set>

/**
 * @details walks through instructions and folds the constants by adding substracting or multuplying as specified
 *
 * @param basicBlock to step its instructions
 */
void walkInstructionsForConstantFolding(LLVMBasicBlockRef basicBlock);

/**
 * @details walks through each basic block in a function
 *
 * @param function to step through
 */
void walkBasicblocksForConstantFolding(LLVMValueRef function);

/**
 * @details walks through the functions in the module to check for constant folding
 *
 * @param moduleReference
 */
void walkFunctionsForConstantFolding(LLVMModuleRef moduleReference);

#endif /* CONSTANT_FOLDING_H */