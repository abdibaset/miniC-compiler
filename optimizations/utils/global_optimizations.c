#include "optimizations_utils.h"
#include "global_optimizations.h"
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

map<LLVMBasicBlockRef, set<LLVMValueRef>> BLOCK_TO_ALL_STOREINSTRUCTIONS, BLOCK_TO_OUTSET, BLOCK_TO_INSET;
map<LLVMValueRef, set<LLVMValueRef>> MEMORY_TO_INSTRUCTIONS;
map<LLVMBasicBlockRef, set<LLVMValueRef>> GEN_SET, KILL_SET;
map<LLVMBasicBlockRef, set<LLVMBasicBlockRef>> BLOCK_TO_PREDSET;
vector<LLVMValueRef> LOAD_INSTRUCTIONS_TO_REPLACE;
bool HAS_GLOBAL_CHANGED = false;

void clearVariables()
{
    BLOCK_TO_ALL_STOREINSTRUCTIONS.clear();
    BLOCK_TO_OUTSET.clear();
    BLOCK_TO_INSET.clear();
    MEMORY_TO_INSTRUCTIONS.clear();
    GEN_SET.clear();
    KILL_SET.clear();
    BLOCK_TO_PREDSET.clear();
    LOAD_INSTRUCTIONS_TO_REPLACE.clear();
}

void setUpLocalGlobalVariables(LLVMValueRef function)
{
    MEMORY_TO_INSTRUCTIONS = getMemoryToInstructionsMap(function);
    BLOCK_TO_ALL_STOREINSTRUCTIONS = getBlockToInstructionsMap(function);
    BLOCK_TO_PREDSET = getPredcessorsMap(function);
}

void replaceValidLoadInstructionsOperandsWithConstant(set<LLVMValueRef> inSet, LLVMValueRef loadInstruction)
{
    LLVMValueRef loadMemoryPtr = LLVMGetOperand(loadInstruction, 0);
    LLVMValueRef constVal = NULL;

    for (auto &instruction : inSet)
    {
        LLVMValueRef storeMemory = LLVMGetOperand(instruction, 1);
        LLVMValueRef storeOperand = LLVMGetOperand(instruction, 0);

        if (storeMemory == loadMemoryPtr)
        {
            if (!LLVMIsConstant(storeOperand))
            {
                return;
            }

            if (constVal == NULL)
            {
                constVal = storeOperand;
            }
            else if (storeOperand != constVal)
            {
                return;
            }
        }
    }

    LLVMValueRef constValue = LLVMConstInt(LLVMTypeOf(loadInstruction), LLVMConstIntGetSExtValue(constVal), false);
    LLVMReplaceAllUsesWith(loadInstruction, constValue);
    LOAD_INSTRUCTIONS_TO_REPLACE.push_back(loadInstruction);
}

set<LLVMValueRef> removeKilledStoreInstruction(set<LLVMValueRef> inSet, LLVMValueRef currentStoreInstruction)
{
    LLVMValueRef currInstMemoryOperand = LLVMGetOperand(currentStoreInstruction, 1);
    LLVMValueRef predInstMemoryOperand = NULL;

    set<LLVMValueRef> inSetCopy = inSet;
    set<LLVMValueRef>::iterator it = inSet.begin();
    while (it != inSet.end() && *it != currentStoreInstruction)
    {
        predInstMemoryOperand = LLVMGetOperand(*it, 1);

        if (predInstMemoryOperand == currInstMemoryOperand)
        {
            inSetCopy.erase(*it);
        }
        ++it;
    }
    return inSetCopy;
}

void evaluateLoadInstructionForConstantReplacement(LLVMBasicBlockRef basicBlock)
{
    set<LLVMValueRef> inSetOfThisBlock = BLOCK_TO_INSET[basicBlock];

    for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction; instruction = LLVMGetNextInstruction(instruction))
    {
        if (getOpcode(instruction) == LLVMStore)
        {
            inSetOfThisBlock.insert(instruction);
            inSetOfThisBlock = removeKilledStoreInstruction(inSetOfThisBlock, instruction);
        }

        if (getOpcode(instruction) == LLVMLoad)
        {
            replaceValidLoadInstructionsOperandsWithConstant(inSetOfThisBlock, instruction);
        }
    }
}

set<LLVMValueRef> computeGenSet(LLVMBasicBlockRef basicBlock)
{
    map<LLVMValueRef, LLVMValueRef> memoryToStoreInstructionMap = {};
    LLVMValueRef memoryOperand = NULL;
    set<LLVMValueRef> genSet = {};

    for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction;
         instruction = LLVMGetNextInstruction(instruction))
    {
        if (getOpcode(instruction) == LLVMStore)
        {
            memoryOperand = LLVMGetOperand(instruction, 1);

            // overwrite instruction if mem ptr is already present
            memoryToStoreInstructionMap[memoryOperand] = instruction;
        }
    }

    for (auto &keyValPair : memoryToStoreInstructionMap)
    {
        genSet.insert(keyValPair.second);
    }

    return genSet;
}

set<LLVMValueRef> computeKillSet(LLVMBasicBlockRef basicBlock)
{

    set<LLVMValueRef> killSet = {};
    set<LLVMValueRef> allStoreInstructions = BLOCK_TO_ALL_STOREINSTRUCTIONS[basicBlock];
    set<LLVMValueRef> allInstructionToThisMemory = {};
    LLVMValueRef memoryOperand = NULL;

    for (auto &instruction : allStoreInstructions)
    {
        memoryOperand = LLVMGetOperand(instruction, 1);
        allInstructionToThisMemory = MEMORY_TO_INSTRUCTIONS[memoryOperand];

        allInstructionToThisMemory.erase(instruction);
        killSet.insert(allInstructionToThisMemory.begin(), allInstructionToThisMemory.end());
    }
    return killSet;
}

void computeInAndOutSets(LLVMValueRef function)
{
    // Compute IN and OUT sets iteratively until convergence
    bool hasOutSetChanged = true;
    while (hasOutSetChanged)
    {
        hasOutSetChanged = false;
        // Compute IN sets for all basic blocks
        for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock))
        {
            set<LLVMBasicBlockRef> predecessors = BLOCK_TO_PREDSET[basicBlock];

            for (const auto &predecessor : predecessors)
            {
                BLOCK_TO_INSET[basicBlock].insert(BLOCK_TO_OUTSET[predecessor].begin(), BLOCK_TO_OUTSET[predecessor].end());
            }
        }

        // Compute OUT sets for all basic blocks
        for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock))
        {
            set<LLVMValueRef> inSet = BLOCK_TO_INSET[basicBlock];
            set<LLVMValueRef> currentOutSet = GEN_SET[basicBlock];

            // Calculate OUT[B] = GEN[B] union (IN[B] - KILL[B])
            set<LLVMValueRef> inMinusKillSets;
            set_difference(inSet.begin(), inSet.end(), KILL_SET[basicBlock].begin(), KILL_SET[basicBlock].end(), inserter(inMinusKillSets, inMinusKillSets.end()));

            // union
            set_union(GEN_SET[basicBlock].begin(), GEN_SET[basicBlock].end(), inMinusKillSets.begin(), inMinusKillSets.end(), inserter(currentOutSet, currentOutSet.end()));

            // Check if OUT set has changed
            if (currentOutSet != BLOCK_TO_OUTSET[basicBlock])
            {
                BLOCK_TO_OUTSET[basicBlock] = currentOutSet;
                hasOutSetChanged = true;
            }
        }
    }
}

bool walkBasicBlocksForGlobalOptimizations(LLVMValueRef function)
{
    // get and kill sets
    for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock))
    {
        GEN_SET[basicBlock] = computeGenSet(basicBlock);
        KILL_SET[basicBlock] = computeKillSet(basicBlock);
    }

    // compute ins and outs
    computeInAndOutSets(function);

    for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock))
    {
        evaluateLoadInstructionForConstantReplacement(basicBlock);
    }

    if (LOAD_INSTRUCTIONS_TO_REPLACE.size() > 0)
    {
        HAS_GLOBAL_CHANGED = true;
        for (int index = 0; index < (int)LOAD_INSTRUCTIONS_TO_REPLACE.size(); index++)
        {
            LLVMInstructionEraseFromParent(LOAD_INSTRUCTIONS_TO_REPLACE[index]);
        }
    }

    const bool hasChanged = HAS_GLOBAL_CHANGED;
    if (LOAD_INSTRUCTIONS_TO_REPLACE.size() == 0)
    {
        HAS_GLOBAL_CHANGED = false;
    }

    clearVariables();
    return hasChanged;
}

bool walkFunctionsForGlobalOptimizations(LLVMModuleRef module)
{
    bool hasChanged = false;
    for (LLVMValueRef function = LLVMGetFirstFunction(module); function;
         function = LLVMGetNextFunction(function))
    {
        setUpLocalGlobalVariables(function);
        hasChanged = walkBasicBlocksForGlobalOptimizations(function) || hasChanged;
    }
    return hasChanged;
}
