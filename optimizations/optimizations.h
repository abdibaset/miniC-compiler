#ifndef OPTIMIZATIONS_H
#define OPTIMIZATIONS_H

/**
 * @details steps through each instruction in a given basic block to examine for dead code elimination
 *
 * @param basicBlock - steps through the instructions in basic blocks
 * */

void walkFunctionsForCommonSubExprAndDeadCode(LLVMModuleRef module);

/**
 * @details walks through the functions in the module to check for constant folding
 *
 * @param moduleReference
 */
void walkFunctionsForConstantFolding(LLVMModuleRef moduleReference);

/**
 * @details steps through the basicBlocks in a function
 *
 * @param function to step through for global optimizations
 */
bool walkFunctionsForGlobalOptimizations(LLVMModuleRef moduleReference);

#endif // OPTIMIZATIONS_H