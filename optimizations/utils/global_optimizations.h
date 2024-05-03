#ifndef GLOBAL_OPTIMIZATIONS_H
#define GLOBAL_OPTIMIZATIONS_H



/**
 * @details steps through the basicBlocks in a function
 *
 * @param function to step through for global optimizations
 */
bool walkBasicBlocksForGlobalOptimizations(LLVMValueRef function);


/**
 * @details steps through the module and calls on functions to constant propagration
 *
 * @param module to step through
 */
bool walkFunctionsForGlobalOptimizations(LLVMModuleRef module);

/**
 * @details computes the gen set of the basic block
 *
 * @param basicBlock
 *
 */
set<LLVMValueRef> computeGenSet(LLVMBasicBlockRef basicBlock);

/**
 * @details computes the kill set of the basic block
 *
 * @param basicBlock
 *
 */
set<LLVMValueRef> computeKillSet(LLVMBasicBlockRef basicBlock);

/**
 * @details computes the in and outs of a fuction
 *
 * @param function
 *
 */
void computeInAndOutSets(LLVMValueRef function);

/**
 * @details evaluates and replaces loads for which all the stores instruction in the block's in set store instructions have the same constant value
 *
 * @param basicBlock
 */
void evaluateLoadInstructionForConstantReplacement(LLVMBasicBlockRef basicBlock);

/**
 * @details removes killed stores in the in set after appending the current store instruction
 *
 * @param inSet of the current block
 * @param currentStoreInstruction - appending instruction into the inset
 */
set<LLVMValueRef> removeKilledStoreInstruction(set<LLVMValueRef> inSet, LLVMValueRef currentStoreInstruction);

/**
 * @details helper function for @evaluateLoadInstructionForConstantReplacement
 * checks all the stores in the have write to the same memory the loaded instruction loads and their values are constant and equivalent
 *
 *
 * @param inSet of the current block
 * @param loadInstruction to inspect
 */
void replaceValidLoadInstructionsOperandsWithConstant(set<LLVMValueRef> inSet, LLVMValueRef loadInstruction);


void setUpLocalGlobalVariables(LLVMValueRef function);

#endif /* GLOBAL_OPTIMIZATIONS_H */
