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
using namespace std;

#define prt(x)             \
    if (x)                 \
    {                      \
        printf("%s\n", x); \
    }

LLVMOpcode getOpcode(LLVMValueRef instruction)
{
    return LLVMGetInstructionOpcode(instruction);
}

vector<LLVMValueRef> getOperands(LLVMValueRef instruction)
{
    vector<LLVMValueRef> operands;
    for (int i = 0; i < LLVMGetNumOperands(instruction); i++)
    {
        LLVMValueRef operand = LLVMGetOperand(instruction, i);
        operands.push_back(operand);
    }
    return operands;
}

InstructionProps createInstructionProps(LLVMValueRef instruction)
{
    InstructionProps inst;
    inst.instruction = instruction;
    inst.opcode = getOpcode(instruction);
    inst.operands = getOperands(instruction);

    return inst;
}

map<LLVMBasicBlockRef, set<LLVMBasicBlockRef>> getPredcessorsMap(LLVMValueRef function)
{

    map<LLVMBasicBlockRef, set<LLVMBasicBlockRef>> predecessorsMap;
    for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock))
    {
        LLVMValueRef terminator = LLVMGetBasicBlockTerminator(basicBlock);

        if (terminator != NULL)
        {
            int numSucessors = LLVMGetNumSuccessors(terminator);
            LLVMBasicBlockRef successor = NULL;

            for (int index = 0; index < numSucessors; index++)
            {
                successor = LLVMGetSuccessor(terminator, index);
                predecessorsMap[successor].insert(basicBlock);
            }
        }
    }

    return predecessorsMap;
}

bool areAllOperandsSame(LLVMValueRef oldInst, LLVMValueRef currInst)
{
    vector<LLVMValueRef> currOperands = getOperands(currInst);
    vector<LLVMValueRef> oldOperands = getOperands(oldInst);

    if (currOperands.size() != oldOperands.size())
    {
        return false;
    }

    for (int index = 0; index < (int)currOperands.size(); index++)
    {
        if (currOperands[index] != oldOperands[index])
        {
            return false;
        }
    }
    return true;
}

bool isValidOpcode(LLVMValueRef instruction)
{
    LLVMOpcode opcode = getOpcode(instruction);

    switch (opcode)
    {
    case LLVMLoad:
        return true;
        break;
    case LLVMAdd:
        return true;
        break;
    case LLVMSub:
        return true;
        break;
    case LLVMMul:
        return true;
        break;
    default:
        return false;
        break;
    }
}

void walkGlobalValues(LLVMModuleRef module)
{
    for (LLVMValueRef gVal = LLVMGetFirstGlobal(module);
         gVal;
         gVal = LLVMGetNextGlobal(gVal))
    {
    }
}

map<LLVMValueRef, set<LLVMValueRef>> getMemoryToInstructionsMap(LLVMValueRef function)
{
    map<LLVMValueRef, set<LLVMValueRef>> memPtrToInstructionsMap = {};
    LLVMValueRef memoryOperand = NULL;
    for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock;
         basicBlock = LLVMGetNextBasicBlock(basicBlock))
    {

        for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction;
             instruction = LLVMGetNextInstruction(instruction))
        {
            if (getOpcode(instruction) == LLVMStore)
            {
                memoryOperand = LLVMGetOperand(instruction, 1);

                memPtrToInstructionsMap[memoryOperand].insert(instruction);
            }
        }
    }

    return memPtrToInstructionsMap;
}

map<LLVMBasicBlockRef, set<LLVMValueRef>> getBlockToInstructionsMap(LLVMValueRef function)
{
    map<LLVMBasicBlockRef, set<LLVMValueRef>> blockToInstructionsMap;

    for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function);
         basicBlock;
         basicBlock = LLVMGetNextBasicBlock(basicBlock))
    {
        for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction;
             instruction = LLVMGetNextInstruction(instruction))
        {
            if (getOpcode(instruction) == LLVMStore)
            {
                blockToInstructionsMap[basicBlock].insert(instruction);
            }
        }
    }

    return blockToInstructionsMap;
}