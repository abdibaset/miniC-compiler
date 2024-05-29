#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Types.h>
#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <string>
#include <iostream>

#include "../optimizations/optimizations_utils.h"

map<LLVMValueRef, vector<int>> CURRENT_LIVENESS_MAP;
map<LLVMValueRef, int> INST_INDEX_MAP;
map<LLVMValueRef, int> INST_REG_MAP;
const char *FILENAME;
map<LLVMBasicBlockRef, string> BASICBLOCK_NAME_MAP;
int localMem;

void find_spill_and_reallocate_registers(LLVMValueRef currInstruction, vector<int> &registers);

bool isValidOpcodeForCode(LLVMValueRef instruction)
{
    LLVMOpcode opcode = getOpcode(instruction);

    switch (opcode)
    {
    case LLVMLoad:
        return true;
    case LLVMAdd:
        return true;
    case LLVMSub:
        return true;
    case LLVMMul:
        return true;
    default:
        return false;
    }
    return false;
}

bool is_operand_live(LLVMValueRef currInstruction, LLVMValueRef instructionToCheck)
{
    return CURRENT_LIVENESS_MAP[instructionToCheck].back() > INST_INDEX_MAP[currInstruction];
}

void handle_instruction_without_return(LLVMValueRef currInstruction, vector<int> &registers)
{
    vector<LLVMValueRef> operands = getOperands(currInstruction);
    for (const auto &operand : operands)
    {
        if (CURRENT_LIVENESS_MAP.find(operand) != CURRENT_LIVENESS_MAP.end())
        {
            bool isOperandLive = INST_INDEX_MAP[currInstruction] < CURRENT_LIVENESS_MAP[operand].back();
            bool operandHasRegister = INST_REG_MAP[operand] != -1;

            if (!isOperandLive && operandHasRegister)
            {
                registers.push_back(INST_REG_MAP[operand]);
            }
        }
    }
}

void handle_binary_operation_instruction(LLVMValueRef currInstruction, vector<int> &registers)
{
    // get operands of intruction
    vector<LLVMValueRef> operands = getOperands(currInstruction);

    // check first operand in reg map and has register
    bool isFirstOperandAssignedRegister = INST_REG_MAP.find(operands[0]) != INST_REG_MAP.end() && INST_REG_MAP[operands[0]] != -1;
    bool isSecondOperandAssignedRegister = INST_REG_MAP.find(operands[1]) != INST_REG_MAP.end() && INST_REG_MAP[operands[1]] != -1;
    bool isFirstOperandLastSeen = CURRENT_LIVENESS_MAP.find(operands[0]) != CURRENT_LIVENESS_MAP.end() && CURRENT_LIVENESS_MAP[operands[0]][1] == INST_INDEX_MAP[currInstruction];
    bool isSecondOperandLive = CURRENT_LIVENESS_MAP.find(operands[1]) != CURRENT_LIVENESS_MAP.end() && is_operand_live(currInstruction, operands[1]);

    if (isFirstOperandAssignedRegister && isFirstOperandLastSeen)
    {
        INST_REG_MAP[currInstruction] = INST_REG_MAP[operands[0]];
        if (!isSecondOperandLive && isSecondOperandAssignedRegister)
        {
            registers.push_back(INST_REG_MAP[operands[1]]);
        }
    }
}

void find_spill_and_reallocate_registers(LLVMValueRef currInstruction, vector<int> &registers)
{
    int highestRange = 0;
    LLVMValueRef instructionWithHighestRange;
    for (const auto &keyValPair : INST_REG_MAP)
    {
        LLVMValueRef instInRegMap = keyValPair.first;
        if (INST_REG_MAP[instInRegMap] != -1)
        {
            int firstUseIndex = CURRENT_LIVENESS_MAP[instInRegMap][0];
            int lastUseIndex = CURRENT_LIVENESS_MAP[instInRegMap].back();
            int range = lastUseIndex - firstUseIndex;
            if (range > highestRange)
            {
                highestRange = range;
                instructionWithHighestRange = instInRegMap;
            }
        }
    }

    // check if currInstruction has lower highst
    int currInstLivessRange = CURRENT_LIVENESS_MAP[currInstruction].back() - CURRENT_LIVENESS_MAP[currInstruction][0];

    // assign instruction with lowest range's registet to currInstruction
    if (currInstLivessRange < highestRange)
    {
        INST_REG_MAP[currInstruction] = INST_REG_MAP[instructionWithHighestRange];
        INST_REG_MAP[instructionWithHighestRange] = -1;
    }
    else
    {
        INST_REG_MAP[currInstruction] = -1;
    }

    // check if operands are live; if not deallocat registers
    vector<LLVMValueRef> currInstOperands = getOperands(currInstruction);

    for (const auto &operand : currInstOperands)
    {
        if (!is_operand_live(currInstruction, operand))
        {
            registers.push_back(INST_REG_MAP[operand]);
            INST_REG_MAP[operand] = 0;
        }
    }
}

void handle_other_instructions(LLVMValueRef currInstruction, vector<int> &registers)
{
    vector<LLVMValueRef> operands = getOperands(currInstruction);
    for (const auto &operand : operands)
    {
        bool isOperandInRegMap = INST_REG_MAP.find(operand) != INST_REG_MAP.end();
        if (isOperandInRegMap && INST_REG_MAP[operand] != -1 && !is_operand_live(currInstruction, operand))
        {
            registers.push_back(INST_REG_MAP[operand]);
        }
    }
}

void check_operands_to_update_reg_map(LLVMValueRef instruction, int &instructionIndex)
{
    vector<LLVMValueRef> operands = getOperands(instruction);
    for (const auto &operand : operands)
    {
        if (CURRENT_LIVENESS_MAP.find(operand) != CURRENT_LIVENESS_MAP.end())
        {
            CURRENT_LIVENESS_MAP[operand].push_back(instructionIndex);
        }
    }
}

void allocate_registers(LLVMBasicBlockRef basicBlock)
{
    // Initialize the set of available physical registers to (ebx, ecx, edx)
    vector<int> registers = {3, 2, 1};
    for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction; instruction = LLVMGetNextInstruction(instruction))
    {
        // If Instr is an instruction that does not have a result (e.g. store, branch, call that doesn't return a value):
        LLVMOpcode opcode = getOpcode(instruction);
        if (opcode != LLVMAlloca)
        {
            if (LLVMGetTypeKind(LLVMTypeOf(instruction)) == LLVMVoidTypeKind)
            {
                handle_instruction_without_return(instruction, registers);
            }

            if (opcode == LLVMAdd || opcode == LLVMSub || opcode == LLVMMul)
            {
                handle_binary_operation_instruction(instruction, registers);
            }

            // if not present in INST_REG_MAP, but a non-void instruction
            if (INST_REG_MAP.find(instruction) == INST_REG_MAP.end() && LLVMGetTypeKind(LLVMTypeOf(instruction)) != LLVMVoidTypeKind)
            {
                // available resource
                if (!registers.empty())
                {
                    INST_REG_MAP[instruction] = registers.back();
                    registers.pop_back();
                    handle_other_instructions(instruction, registers);
                }

                else
                {
                    find_spill_and_reallocate_registers(instruction, registers);
                }
            }
        }
    }
}

void compute_liveness_and_map_instruction_to_index(LLVMBasicBlockRef basicBlock)
{
    int instructionIndex = 0;
    for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction; instruction = LLVMGetNextInstruction(instruction))
    {
        // if check for instruction use:
        LLVMOpcode opcode = getOpcode(instruction);
        if (opcode != LLVMAlloca)
        {
            if ((opcode == LLVMAdd || opcode == LLVMSub || opcode == LLVMMul || opcode == LLVMICmp) || (opcode == LLVMCall && LLVMGetTypeKind(LLVMTypeOf(instruction)) != LLVMVoidTypeKind))
            {
                check_operands_to_update_reg_map(instruction, instructionIndex);
                CURRENT_LIVENESS_MAP[instruction].push_back(instructionIndex);
            }
            else if (opcode == LLVMLoad)
            {
                CURRENT_LIVENESS_MAP[instruction].push_back(instructionIndex);
            }
            else
            {
                check_operands_to_update_reg_map(instruction, instructionIndex);
            }
        }

        INST_INDEX_MAP[instruction] = instructionIndex;
        instructionIndex++;
    }
}

void print_directives(bool overWrite = false, const char *instructionToWriteToFile = NULL)
{
    FILE *file;
    if (overWrite)
    {
        file = fopen(FILENAME, "w");
    }
    else
    {
        file = fopen(FILENAME, "a");
    }

    if (file == NULL)
    {
        fprintf(stderr, "Error: Unable to open file %s\n", FILENAME);
        exit(0);
    }

    fprintf(file, "%s", instructionToWriteToFile);
    fclose(file);
}

void print_function_end()
{
    FILE *file = fopen(FILENAME, "a");
    if (file == NULL)
    {
        fprintf(stderr, "Error: Unable to open file %s\n", FILENAME);
        exit(0);
    }

    fprintf(file, "\tleave\n\tret\n");
    fclose(file);
}

void gen_load_instruction_assembly_code(LLVMValueRef instruction, map<LLVMValueRef, int> offsetMap)
{
    LLVMValueRef operand = getOperands(instruction)[0];
    if (INST_REG_MAP[instruction] != -1)
    {
        int operandOffsetVal = offsetMap[operand];
        string directiveBuilder = "\tmovl\t" + to_string(operandOffsetVal) + "(%ebp),\t%exx\n";
        print_directives(false, directiveBuilder.c_str());
    }
}

void gen_return_instruction_assembly_code(LLVMValueRef instruction, map<LLVMValueRef, int> offsetMap)
{
    vector<LLVMValueRef> operands = getOperands(instruction);
    string directive;
    if (LLVMIsConstant(operands[0]))
    {
        long long constVal = LLVMConstIntGetSExtValue(operands[0]);
        directive = "\tmovl\t$" + to_string(constVal) + ",\t%eax\n";
    }
    else if (INST_REG_MAP[operands[0]] == -1)
    {
        int offsetOfTempVariable = offsetMap[operands[0]];
        directive = "\tmovl\t" + to_string(offsetOfTempVariable) + "(%ebp),\t%eax\n";
    }
    else if (INST_REG_MAP[operands[0]] != -1)
    {
        directive = "\tmovl\t%exx,\t%eax\n";
    }
    directive += "\tpopl\t%ebx\n";
    print_directives(false, directive.c_str());
    print_function_end();
}

void gen_store_instruction_assembly_code(LLVMValueRef instruction, map<LLVMValueRef, int> offsetMap)
{
    vector<LLVMValueRef> operands = getOperands(instruction);
    string directiveBuilder;
    if (LLVMIsConstant(operands[0]))
    {
        int secondOperandOffset = offsetMap[operands[1]];
        long long constVal = LLVMConstIntGetSExtValue(operands[0]);
        directiveBuilder = "\tmovl\t$" + to_string(constVal) + ",\t" + to_string(secondOperandOffset) + "(%ebp)\n";
        print_directives(false, directiveBuilder.c_str());
    }
    else
    {
        //        if %a has a physical register %exx assigned to it:
        int offsetOfSecondOperand = offsetMap[operands[1]];
        if (INST_REG_MAP[operands[0]] != -1)
        {
            directiveBuilder = "\tmovl\t%exx,\t" + to_string(offsetOfSecondOperand) + "(%ebp)\n";
            print_directives(false, directiveBuilder.c_str());
        }
        else
        {
            int offsetOfFirstOperand = offsetMap[operands[0]];
            string firstDirectiveBuilder = "\tmovl\t" + to_string(offsetOfFirstOperand) + ",\t%eax\n";
            print_directives(false, firstDirectiveBuilder.c_str());

            string secondDirectiveBuilder = "\tmovl\t%eax," + to_string(offsetOfSecondOperand) + "\t(%ebp)\n";
            print_directives(false, secondDirectiveBuilder.c_str());
        }
    }
}

void gen_call_instruction_assembly_code(LLVMValueRef instruction, map<LLVMValueRef, int> offsetMap)
{
    bool hasParam = false;
    string directiveBuilder = "\tpushl\t%ecx\n\tpushl\t%edx\n";
    print_directives(false, directiveBuilder.c_str());

    vector<LLVMValueRef> operands = getOperands(instruction);
    if (operands.size() > 1)
    {
        hasParam = true;
        if (LLVMIsConstant(operands[0]))
        {
            long long constVal = LLVMConstIntGetSExtValue(operands[0]);
            string directiveBuilder = "\tpushl\t$" + to_string(constVal) + "\n";
            print_directives(false, directiveBuilder.c_str());
        }
        else
        {
            if (INST_REG_MAP[operands[0]] != -1)
            {
                string directiveBuilder = "\tpushl\t%exx\n";
                print_directives(false, directiveBuilder.c_str());
            }
            else if (INST_REG_MAP[operands[0]] == -1)
            {
                int offsetOfParam = offsetMap[operands[0]];
                string directiveBuilder = "\tpushl\t" + to_string(offsetOfParam) + "(%ebp)\n";
                print_directives(false, directiveBuilder.c_str());
            }
        }
    }
    size_t length;
    const char *functioName = LLVMGetValueName2(getOperands(instruction).back(), &length);
    string directiveBuilder2 = "\tcall\t" + string(functioName) + "\n";

    if (hasParam)
    {
        directiveBuilder2 += "\taddl\t$4,\t%esp\n";
    }
    print_directives(false, directiveBuilder2.c_str());

    // has return value
    string directiveBuilder3;
    if (LLVMGetTypeKind(LLVMTypeOf(instruction)) != LLVMVoidTypeKind)
    {
        // if it has a register
        if (INST_REG_MAP[instruction] != -1)
        {
            directiveBuilder3 = "\tmovl\t%eax,\t%exx\n";
        }
        else
        {
            int offsetOfCurrInstruction = offsetMap[instruction];
            directiveBuilder3 = "\tmovl\t%eax,\t" + to_string(offsetOfCurrInstruction) + "(%ebp)\n";
        }
    }
    directiveBuilder3 += "\tpopl\t%edx\n\tpopl\t%ecx\n";
    print_directives(false, directiveBuilder3.c_str());
}

void gen_branch_instruction_assembly_code(LLVMValueRef instruction, map<LLVMValueRef, int> offsetMap)
{
    // unconditional branch
    string directiveBuilder;
    if (!LLVMIsConditional(instruction))
    {
        LLVMBasicBlockRef successor = LLVMGetSuccessor(instruction, 0);
        string basicBlockLabel = BASICBLOCK_NAME_MAP[successor];
        directiveBuilder = "jmp\t." + basicBlockLabel + "\n";
        print_directives(false, directiveBuilder.c_str());
    }
    else
    {
        // if the branch is conditional (br i1 %a, label %b, label %c)
        // get labels L1 for %b and L2 for %c from bb_labels
        vector<LLVMValueRef> operands = getOperands(instruction);
        LLVMIntPredicate predicate;
        if (getOpcode(operands[0]) == LLVMICmp)
        {
            predicate = LLVMGetICmpPredicate(operands[0]);
        }
        LLVMBasicBlockRef firstSuccesor = LLVMGetSuccessor(instruction, 0);
        LLVMBasicBlockRef secondSuccesor = LLVMGetSuccessor(instruction, 1);
        string directiveBuilder;
        string jxx;

        switch (predicate)
        {
        case LLVMIntSLT:
            jxx = "\tjlt\t.";
            break;
        case LLVMIntSGT:
            jxx = "\tjgt\t.";
            break;
        case LLVMIntSLE:
            jxx = "\tjle\t.";
            break;
        case LLVMIntSGE:
            jxx = "\tjge\t.";
            break;
        case LLVMIntEQ:
            jxx = "\tjeq\t.";
            break;
        case LLVMIntNE:
            jxx = "\tjneq\t.";
            break;
        default:
            break;
        }

        directiveBuilder += jxx + BASICBLOCK_NAME_MAP[firstSuccesor] + "\n" + "jmp\t." + BASICBLOCK_NAME_MAP[secondSuccesor] + "\n";
        print_directives(false, directiveBuilder.c_str());
    }
}

string get_print_directive_arithmetic(LLVMOpcode opcode)
{
    switch (opcode)
    {
    case LLVMAdd:
        return "addl";
    case LLVMSub:
        return "subl";
    case LLVMMul:
        return "imull";
    default:
        break;
    }

    return "";
}

void gen_binary_instruction_assembly_code(LLVMValueRef instruction, map<LLVMValueRef, int> offsetMap)
{
    string registerVal;
    if (INST_REG_MAP[instruction] != -1)
    {
        registerVal = "%exx";
    }
    else
    {
        registerVal = "%eax";
    }

    vector<LLVMValueRef> operands = getOperands(instruction);
    LLVMOpcode opcode = getOpcode(instruction);

    // first operand
    string firstOperandDirectiveBuilder;
    if (LLVMIsConstant(operands[0]))
    {
        long long constVal = LLVMConstIntGetSExtValue(operands[0]);
        firstOperandDirectiveBuilder = "\t$" + to_string(constVal) + ",\t" + registerVal + "\n";
    }
    else
    {
        if (INST_REG_MAP[operands[0]] != -1)
        {
            firstOperandDirectiveBuilder = "\tmovl\t%eyy,\t" + registerVal + "\n";
        }
        else
        {
            int offsetVal = offsetMap[operands[0]];
            firstOperandDirectiveBuilder = "\tmovl\t" + to_string(offsetVal) + "(%ebp),\t" + registerVal + "\n";
        }
    }
    print_directives(false, firstOperandDirectiveBuilder.c_str());

    // second operand
    string secondOperandDirectiveBuilder;
    string arthimeticType = get_print_directive_arithmetic(opcode);
    if (LLVMIsConstant(operands[1]))
    {
        long long constVal = LLVMConstIntGetSExtValue(operands[1]);
        secondOperandDirectiveBuilder = "\t" + arthimeticType + "\t$" + to_string(constVal) + ",\t" + registerVal + "\n";
    }
    else
    {
        if (INST_REG_MAP[operands[1]] != -1)
        {
            secondOperandDirectiveBuilder = "\t" + arthimeticType + "\t%ezz,\t" + registerVal + "\n";
        }
        else
        {
            int offsetVal = offsetMap[operands[1]];
            secondOperandDirectiveBuilder = "\t" + arthimeticType + "\t" + to_string(offsetVal) + "(%ebp),\t" + registerVal + "\n";
        }
    }

    print_directives(false, secondOperandDirectiveBuilder.c_str());

    // back to the instruction

    if (INST_REG_MAP[instruction] != -1)
    {
        int offsetVal = offsetMap[instruction];
        string instDirectiveBuilder = "\tmovl\t%eax,\t" + to_string(offsetVal) + "(%ebp)\n";
        print_directives(false, instDirectiveBuilder.c_str());
    }
}

void gen_compare_instruction_assembly_code(LLVMValueRef instruction, map<LLVMValueRef, int> offsetMap)
{
    string registerVal;
    if (INST_REG_MAP[instruction] != -1)
    {
        registerVal = "%exx";
    }
    else
    {
        registerVal = "%eax";
    }

    vector<LLVMValueRef> operands = getOperands(instruction);
    string firstOperandDirective;
    // first operand
    if (LLVMIsConstant(operands[0]))
    {
        firstOperandDirective = "\tmovl\t$" + to_string(LLVMConstIntGetSExtValue(operands[0])) + ",\t" + registerVal + "\n";
    }
    else
    {
        if (INST_REG_MAP[operands[0]] != -1)
        {
            if (strcmp("%eyy", registerVal.c_str()) == 0)
            {
                firstOperandDirective = "\tmovl\t%eyy,\t" + registerVal + "\n";
            }
        }
        else
        {
            int offsetVal = offsetMap[operands[0]];
            firstOperandDirective = "\tmovl\t" + to_string(offsetVal) + "(%ebp),\t" + registerVal + "\n";
        }
    }
    print_directives(false, firstOperandDirective.c_str());
    // second operand
    string secondOperandDirective;
    if (LLVMIsConstant(operands[1]))
    {
        secondOperandDirective = "\tcmpl\t$" + to_string(LLVMConstIntGetSExtValue(operands[1])) + ",\t" + registerVal + "\n";
    }
    else
    {
        if (INST_REG_MAP[operands[1]] != -1)
        {
            secondOperandDirective = "\tcmpl\t%ezz,\t" + registerVal + "\n";
        }
        else
        {
            int offsetVal = offsetMap[operands[1]];
            secondOperandDirective = "\tcmpl\t" + to_string(offsetVal) + "(%ebp),\t" + registerVal + "\n";
        }
    }
    print_directives(false, secondOperandDirective.c_str());
}

void gen_assembly_code_for_basic_block(LLVMBasicBlockRef basicBlock, map<LLVMValueRef, int> offsetMap)
{
    for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction; instruction = LLVMGetNextInstruction(instruction))
    {
        LLVMOpcode opcode = getOpcode(instruction);
        switch (opcode)
        {
        case LLVMRet:
            gen_return_instruction_assembly_code(instruction, offsetMap);
            break;
        case LLVMLoad:
            gen_load_instruction_assembly_code(instruction, offsetMap);
            break;
        case LLVMStore:
            gen_store_instruction_assembly_code(instruction, offsetMap);
            break;
        case LLVMCall:
            gen_call_instruction_assembly_code(instruction, offsetMap);
            break;
        case LLVMBr:
            gen_branch_instruction_assembly_code(instruction, offsetMap);
            break;
        case LLVMAdd:
        case LLVMSub:
        case LLVMMul:
            gen_binary_instruction_assembly_code(instruction, offsetMap);
            break;
        case LLVMICmp:
            gen_compare_instruction_assembly_code(instruction, offsetMap);
            break;
        default:
            break;
        }
    }
}

void walk_basic_blocks_for_register_allocation_and_codegen(LLVMValueRef function, map<LLVMValueRef, int> offsetMap)
{
    for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock))
    {
        compute_liveness_and_map_instruction_to_index(basicBlock);
        allocate_registers(basicBlock);

        gen_assembly_code_for_basic_block(basicBlock, offsetMap);
        CURRENT_LIVENESS_MAP.clear();
        INST_INDEX_MAP.clear();
        INST_REG_MAP.clear();
    }
}

void walk_functions_for_register_allocation_and_codegen(LLVMModuleRef moduleReference, map<LLVMValueRef, int> offsetMap)
{
    for (LLVMValueRef function = LLVMGetFirstFunction(moduleReference); function; function = LLVMGetNextFunction(function))
    {
        size_t length;
        const char *functionName = LLVMGetValueName2(function, &length);
        if (strcmp(functionName, "main-func") == 0)
        {
            string directives = "\t.text\n\t.globl\n\t.type\n\tfunction " + string(functionName) + "\nfunc:\n";
            string emitInstructions = "\tpushl\t%ebp\n\tmovl\t%esp,\t%ebp\n\tsubl\t$" + to_string(localMem - 4) + ",\t%esp\n\tpushl\t%ebx\n";
            string firstInstructions = (directives + emitInstructions).c_str();
            print_directives(true, firstInstructions.c_str());
        }
        walk_basic_blocks_for_register_allocation_and_codegen(function, offsetMap);
    }
}

void create_basic_block_labels(LLVMModuleRef moduleReference)
{
    int basicBlockNumber = 0;
    for (LLVMValueRef function = LLVMGetFirstFunction(moduleReference); function; function = LLVMGetNextFunction(function))
    {
        for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock))
        {
            string basicBlockName = "BB" + to_string(basicBlockNumber);
            BASICBLOCK_NAME_MAP[basicBlock] = basicBlockName;
            basicBlockNumber++;
        }
    }
}

map<LLVMValueRef, int> get_offset_map(LLVMModuleRef moduleReference)
{
    localMem = 4;
    map<LLVMValueRef, int> offsetMap;
    LLVMValueRef functionParam;
    for (LLVMValueRef function = LLVMGetFirstFunction(moduleReference); function; function = LLVMGetNextFunction(function))
    {
        // add the LLVMValueRef of the parameter as key and its associated value as 8 to the offset_map.
        for (size_t i = 0; i < LLVMCountParams(function); i++)
        {
            functionParam = LLVMGetParam(function, i); // guaranteed single param
            offsetMap[functionParam] = 8;
        }

        for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(function); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock))
        {
            // loop over instructions
            for (LLVMValueRef instruction = LLVMGetFirstInstruction(basicBlock); instruction; instruction = LLVMGetNextInstruction(instruction))
            {
                LLVMOpcode opcode = getOpcode(instruction);
                switch (opcode)
                {
                case LLVMAlloca:
                {
                    localMem += 4;
                    offsetMap[instruction] = -1 * localMem;
                }
                break;

                case LLVMStore:
                {
                    // If the first operand of the store instruction is equal to the function parameter
                    vector<LLVMValueRef> operands = getOperands(instruction);
                    bool isFirstOperandFuncParam = operands[0] == functionParam;
                    if (isFirstOperandFuncParam)
                    {
                        // Get the value associated with the first operand in offset_map. Let this be x.
                        int firstOperandOffsetFromMap = offsetMap[operands[0]];
                        // Change the value associated with the second operand in  offset_map to x
                        offsetMap[operands[1]] = firstOperandOffsetFromMap;
                    }
                    else
                    { // If the first operand of the store instruction is not equal to the function parameter
                        int secondOperandOffsetFromMap = offsetMap[operands[1]];
                        offsetMap[operands[0]] = secondOperandOffsetFromMap;
                    }
                }
                break;

                case LLVMLoad:
                {
                    // Get the value associated with the first operand in offset_map. Let this be x.
                    vector<LLVMValueRef> operands = getOperands(instruction);
                    int firstOperandOffsetFromMap = offsetMap[operands[0]];
                    offsetMap[instruction] = firstOperandOffsetFromMap;
                }
                default:
                    break;
                }
            }
        }
    }
    return offsetMap;
}

string generateOutputFilename(const string &inputPath)
{
    // Find the last '/' or '\' character to isolate the file name
    size_t lastSlash = inputPath.find_last_of("/\\");
    std::string filename = (lastSlash == std::string::npos) ? inputPath : inputPath.substr(lastSlash + 1);

    // Find the last '.' character to remove the file extension
    size_t lastDot = filename.find_last_of('.');
    std::string baseName = (lastDot == std::string::npos) ? filename : filename.substr(0, lastDot);

    // Append the new extension
    std::string outputFilename = "my_" + baseName + ".s";
    return outputFilename;
}

void generate_assembly_code(const char *filename, LLVMModuleRef moduleReference)
{
    FILENAME = generateOutputFilename(string(filename)).c_str();

    map<LLVMValueRef, int> offsetMap = get_offset_map(moduleReference);
    create_basic_block_labels(moduleReference);
    walk_functions_for_register_allocation_and_codegen(moduleReference, offsetMap);
}
