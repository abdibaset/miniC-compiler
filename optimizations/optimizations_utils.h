#ifndef OPTIMIZATIONS_UTILS_H
#define OPTIMIZATIONS_UTILS_H

// #include "optimizations_utils.h"
#include <map>
#include <set>
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

using namespace std;

typedef struct
{
    LLVMValueRef instruction;
    LLVMOpcode opcode;
    vector<LLVMValueRef> operands;
} InstructionProps;

/**
 *
 * @param instruction
 * @return LLVMOPcode - return  the opcode of the instruction provided
 */
LLVMOpcode getOpcode(LLVMValueRef instruction);

/**
 * @details helper function to extract the operands from the instruction given
 *
 * @param instruction to evaluate
 * @return vector of the operands of the instruction
 */
vector<LLVMValueRef> getOperands(LLVMValueRef instruction);

/**
 * @details creates a struct with operands and opcode properties
 * @param instruction
 *
 * @return type Instruction Props struct
 */
InstructionProps createInstructionProps(LLVMValueRef instruction);

/**
 * @details function that computes the predecessors of a basic blocks
 * @param function
 *
 * @return a map of a block to its predecessors
 */
map<LLVMBasicBlockRef, set<LLVMBasicBlockRef>> getPredcessorsMap(LLVMValueRef function);

/**
 * @details compares all the operands of two instructions
 *
 * @param oldInst - previous instruction
 * @param currInst - current instruction
 *
 * @returns true if all their operands are the same, false otherwise
 */
bool areAllOperandsSame(LLVMValueRef oldInst, LLVMValueRef currInst);

/**
 * @details checks if the opcode is valid for my analysis
 * '
 * @param instruction - instruction to evaluate
 * @return true is if opcode is valid
 */
bool isValidOpcode(LLVMValueRef instruction);

/**
 * @details utils function to step through the global variables in the module
 *
 * @param module
 *
 */
void walkGlobalValues(LLVMModuleRef module);

/**
 * @details maps memory to all the set of store instructions that write into that memory
 *
 * @param function - function to step through to build the map
 */
map<LLVMValueRef, set<LLVMValueRef>> getMemoryToInstructionsMap(LLVMValueRef function);

/**
 * @details maps to all set of store instructions
 *
 * @param function - function to step through
 *
 * @return map of block to its store instructions
 */
map<LLVMBasicBlockRef, set<LLVMValueRef>> getBlockToInstructionsMap(LLVMValueRef function);

#endif // OPTIMIZATIONS_UTILS_H