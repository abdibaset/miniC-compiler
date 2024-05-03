
#ifndef COMMON_SUBEXPR_DEAD_CODE_ELIMINATION_H
#define COMMON_SUBEXPR_DEAD_CODE_ELIMINATION_H

#include "optimizations_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Types.h>
#include <stdbool.h>
#include <unordered_map>
#include <map>
#include <set>
#include <algorithm>
#include <vector>
#include <cstring>
using namespace std;

/**
 * @details checks if two instructions, one already seen in store and the current instruction, have simnilar opcode and operands
 *
 * @param instruction - current instructionn
 */
bool isSamiliarInstruction(LLVMValueRef instruction);

/**
 * @details checks if the current load instruction can be replaced by previous seen load from the same memory
 * @condition - no store instruction between the two loads loading from the same memory
 *
 * @return true if safe to replace, false otherwise
 */
bool isSafeToReplace(LLVMValueRef currInst);

/**
 * @details steps through each each function in the module and calls the basic blocks for common subexpression and dead code analysis
 *
 * @param module - module to step through
 *
 */

void walkBBInstructionsForDeadCodeElimination(LLVMBasicBlockRef basicBlock);

/**
 * @details steps through each instruction in a given basic block to examine for common subexpression elimination
 *
 * @param basicBlock - steps through the instructions in basic blocks
 * */
void walkBBInstructionsForCommonSubExpression(LLVMBasicBlockRef basicBlock);

/**
 * @details walks through basic blocks for a given function
 *
 * @param function
*/
void walkBasicblocksForCommonSubExprAndDeadCode(LLVMValueRef function);

/**
 * @details steps through each instruction in a given basic block to examine for dead code elimination
 *
 * @param basicBlock - steps through the instructions in basic blocks
 * */

void walkFunctionsForCommonSubExprAndDeadCode(LLVMModuleRef module);

/**
 * @details steps through each basicblock and iteratively calls on common subexoression and dead code elimination until there is no * *further change
 *
 * @param function - steps through the basic blocks in the function
 */

#endif // COMMON_SUBEXPR_DEAD_CODE_ELIMINATION_H