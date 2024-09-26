#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Types.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
#include <cstring>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

#include "optimizations.h"
#include "optimizations_utils.h"
using namespace std;

bool HAS_CHANGED = false;

void walkInstructionsForConstantFolding(LLVMBasicBlockRef basicBlock) {
    vector<LLVMValueRef> operands;
    LLVMValueRef constValRef = NULL;
    for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock);
         instruction; instruction = LLVMGetNextInstruction(instruction)) {
        LLVMOpcode opcode = getOpcode(instruction);
        switch (opcode) {
            case LLVMAdd:
                operands = getOperands(instruction);
                if (LLVMIsConstant(operands[0]) &&
                    LLVMIsConstant(operands[1])) {
                    constValRef = LLVMConstAdd(operands[0], operands[1]);
                    LLVMReplaceAllUsesWith(instruction, constValRef);
                    HAS_CHANGED = true;
                }
                break;

            case LLVMSub:
                operands = getOperands(instruction);
                if (LLVMIsConstant(operands[0]) &&
                    LLVMIsConstant(operands[1])) {
                    constValRef = LLVMConstSub(operands[0], operands[1]);
                    LLVMReplaceAllUsesWith(instruction, constValRef);
                    HAS_CHANGED = true;
                }
                break;

            case LLVMMul:
                operands = getOperands(instruction);
                if (LLVMIsConstant(operands[0]) &&
                    LLVMIsConstant(operands[1])) {
                    constValRef = LLVMConstMul(operands[0], operands[1]);
                    LLVMReplaceAllUsesWith(instruction, constValRef);
                    HAS_CHANGED = true;
                }
                break;

            default:
                break;
        }
    }
}

void walkBasicblocksForConstantFoldingAndDeadCodeElimination(
    LLVMValueRef function) {
    for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function);
         basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock)) {
        walkInstructionsForConstantFolding(basicBlock);
        if (HAS_CHANGED) {
            walkBBInstructionsForDeadCodeElimination(basicBlock);
        }
    }
}

void walkFunctionsForConstantFoldingAndDeadCodeElimination(
    LLVMModuleRef module) {
    for (LLVMValueRef function = LLVMGetFirstFunction(module); function;
         function = LLVMGetNextFunction(function)) {
        walkBasicblocksForConstantFoldingAndDeadCodeElimination(function);
    }
}
