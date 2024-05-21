
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

bool HAS_REPLACED = false;
int MATCH_INDEX = 0;
vector<InstructionProps> INSTRUCTIONS_STORE;

bool isSafeToReplace(LLVMValueRef currInst)
{
    LLVMOpcode oldOpcode = INSTRUCTIONS_STORE[MATCH_INDEX].opcode;
    LLVMOpcode currOpcode = getOpcode(currInst);

    if (LLVMLoad == oldOpcode && oldOpcode == currOpcode)
    {
        vector<LLVMValueRef> currOperands = getOperands(currInst);

        for (int index = MATCH_INDEX + 1; index < (int)INSTRUCTIONS_STORE.size(); index++)
        {
            LLVMValueRef instInVect = INSTRUCTIONS_STORE[index].instruction;
            LLVMOpcode instOpcode = getOpcode(instInVect);

            if (instOpcode == LLVMStore)
            {
                vector<LLVMValueRef> oldOperands = getOperands(instInVect);

                // Check if the operands of the current instruction and the stored value are the same
                if (oldOperands.size() > 1 && currOperands.size() > 0 && oldOperands[1] == currOperands[0])
                {
                    return false; // Unsafe to replace
                }
            }
        }
    }

    return true; // Safe to replace
}

bool isSamiliarInstruction(LLVMValueRef instruction)
{
    if (INSTRUCTIONS_STORE.empty())
    {
        return false;
    }

    for (int index = 0; index < (int)INSTRUCTIONS_STORE.size(); index++)
    {
        bool areOpcodesSame = INSTRUCTIONS_STORE[index].opcode == getOpcode(instruction);
        if (areOpcodesSame && areAllOperandsSame(INSTRUCTIONS_STORE[index].instruction, instruction))
        {
            MATCH_INDEX = index;
            return true;
        }
    }

    return false;
}

void walkBBInstructionsForDeadCodeElimination(LLVMBasicBlockRef basicBlock)
{
    LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock);
    LLVMValueRef nextInstruction = NULL;
    while (instruction != NULL)
    {
        LLVMOpcode opcode = getOpcode(instruction);
        if (opcode != LLVMStore && opcode != LLVMRet && opcode != LLVMAlloca && opcode != LLVMCall)
        {
            InstructionProps instProp = createInstructionProps(instruction);
            LLVMUseRef use = LLVMGetFirstUse(instruction);

            if (use == NULL && LLVMIsATerminatorInst(instruction) == NULL)
            {
                nextInstruction = LLVMGetNextInstruction(instruction);
                LLVMInstructionEraseFromParent(instruction);
            }
        }
        if (nextInstruction != NULL)
        {
            instruction = nextInstruction;
            nextInstruction = NULL;
        }
        else
        {
            instruction = LLVMGetNextInstruction(instruction);
        }
    }
}

void walkBBInstructionsForCommonSubExpression(LLVMBasicBlockRef basicBlock)
{

    MATCH_INDEX = 0;
    INSTRUCTIONS_STORE.clear();

    for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction;
         instruction = LLVMGetNextInstruction(instruction))
    {
        if (isValidOpcode(instruction) && isSamiliarInstruction(instruction))
        {
            if (isSafeToReplace(instruction))
            {
                HAS_REPLACED = true;
                LLVMReplaceAllUsesWith(instruction, INSTRUCTIONS_STORE[MATCH_INDEX].instruction);
            }
        }
        if (getOpcode(instruction) == LLVMStore || isValidOpcode(instruction))
        {
            INSTRUCTIONS_STORE.push_back(createInstructionProps(instruction));
        }
    }
}

void walkBasicblocksForCommonSubExprAndDeadCode(LLVMValueRef function)
{
    HAS_REPLACED = true;
    while (HAS_REPLACED)
    {
        HAS_REPLACED = false;
        for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock))
        {
            walkBBInstructionsForCommonSubExpression(basicBlock);
            walkBBInstructionsForDeadCodeElimination(basicBlock);
        }
    }
}

void walkFunctionsForCommonSubExprAndDeadCode(LLVMModuleRef module)
{
    for (LLVMValueRef function = LLVMGetFirstFunction(module);
         function;
         function = LLVMGetNextFunction(function))
    {
        walkBasicblocksForCommonSubExprAndDeadCode(function);
    }
    INSTRUCTIONS_STORE.clear();
}