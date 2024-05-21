#include "optimizations_utils.h"
#include "optimizations.h"
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

void walkInstructionsForConstantFolding(LLVMBasicBlockRef basicBlock)
{
    vector<LLVMValueRef> operands;
    LLVMValueRef constValRef = NULL;
    LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock);

    while (instruction)
    {
        LLVMOpcode opcode = getOpcode(instruction);
        switch (opcode)
        {
        case LLVMAdd:
            operands = getOperands(instruction);
            if (LLVMIsConstant(operands[0]) && LLVMIsConstant(operands[1]))
            {
                constValRef = LLVMConstAdd(operands[0], operands[1]);
                LLVMReplaceAllUsesWith(instruction, constValRef);

                // save the next instructions before removing from parent
                LLVMValueRef nextInstruction = LLVMGetNextInstruction(instruction);
                LLVMInstructionEraseFromParent(instruction);
                instruction = nextInstruction;
            }
            else
            {
                instruction = LLVMGetNextInstruction(instruction);
            }
            break;

        case LLVMSub:
            operands = getOperands(instruction);
            if (LLVMIsConstant(operands[0]) && LLVMIsConstant(operands[1]))
            {
                constValRef = LLVMConstSub(operands[0], operands[1]);
                LLVMReplaceAllUsesWith(instruction, constValRef);

                // save the next instructions before removing from parent
                LLVMValueRef nextInstruction = LLVMGetNextInstruction(instruction);
                LLVMInstructionEraseFromParent(instruction);
                instruction = nextInstruction;
            }
            else
            {
                instruction = LLVMGetNextInstruction(instruction);
            }
            break;

        case LLVMMul:
            operands = getOperands(instruction);
            if (LLVMIsConstant(operands[0]) && LLVMIsConstant(operands[1]))
            {
                constValRef = LLVMConstMul(operands[0], operands[1]);
                LLVMReplaceAllUsesWith(instruction, constValRef);
                LLVMValueRef nextInstruction = LLVMGetNextInstruction(instruction);
                LLVMInstructionEraseFromParent(instruction);
                instruction = nextInstruction;
            }
            else
            {
                instruction = LLVMGetNextInstruction(instruction);
            }
            break;

        default:
            instruction = LLVMGetNextInstruction(instruction);
            break;
        }
    }
}

void walkBasicblocksForConstantFolding(LLVMValueRef function)
{

    for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function);
         basicBlock;
         basicBlock = LLVMGetNextBasicBlock(basicBlock))
    {
        walkInstructionsForConstantFolding(basicBlock);
    }
}

void walkFunctionsForConstantFolding(LLVMModuleRef module)
{
    for (LLVMValueRef function = LLVMGetFirstFunction(module);
         function;
         function = LLVMGetNextFunction(function))
    {
        walkBasicblocksForConstantFolding(function);
    }
}
