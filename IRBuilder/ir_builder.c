#include "../ast/ast.h"
#include "../semantic_preprocesing_utils/semantic_analysis.h"
#include "../semantic_preprocesing_utils/pre_processing.h"
#include "../optimizations/optimizations_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Types.h>
#include <stdbool.h>
#include <map>
#include <set>
#include <algorithm>
#include <vector>
#include <string>
#include <cassert>
#include <string>
using namespace std;

typedef struct
{
    LLVMValueRef function;
    LLVMTypeRef returnType;
} readFuncStruct;

typedef struct
{
    LLVMValueRef function;
    LLVMTypeRef returnType;
} printFuncStruct;

// prototypes
void traverse_prog_func(astNode *node);
LLVMValueRef traverse_tree(astNode *node);
LLVMBasicBlockRef statement_evaluation(astStmt *statement, LLVMBasicBlockRef entryBasicBlock);
LLVMBasicBlockRef setup_and_create_variable_to_alloc_map();
LLVMValueRef create_load_instruction(string varName);
LLVMValueRef create_binary_op(op_type operation, LLVMValueRef LHSLoad, LLVMValueRef RHSLoad);
LLVMBasicBlockRef handle_block_statement(astStmt *statement, LLVMBasicBlockRef entryBasicBlock);
LLVMBasicBlockRef handle_return_statement(astStmt *statement, LLVMBasicBlockRef entryBasicBlock);
LLVMValueRef create_function(int numParams, bool hasReturn, const char *funcName, bool IsVarArg);
void remove_basic_blocks_without_predecessors(map<LLVMBasicBlockRef, set<LLVMBasicBlockRef>> predecessorsMap);

map<string, LLVMValueRef> NAME_ALLOCS_MAP;
vector<string> VARIABLE_NAMES;
LLVMModuleRef MODULE;
LLVMBuilderRef BUILDER;
LLVMBasicBlockRef RETURN_BASICBLOCK;
LLVMValueRef MAIN_FUNC;
printFuncStruct PRINT_FUNC;
readFuncStruct READ_FUNC;

void remove_basic_blocks_without_predecessors(map<LLVMBasicBlockRef, set<LLVMBasicBlockRef>> predecessorsMap)
{
    vector<LLVMBasicBlockRef> blocksToDelete;

    LLVMBasicBlockRef firstBB = LLVMGetFirstBasicBlock(MAIN_FUNC);
    for (LLVMBasicBlockRef basicBlock = LLVMGetFirstBasicBlock(MAIN_FUNC); basicBlock; basicBlock = LLVMGetNextBasicBlock(basicBlock))
    {
        if (predecessorsMap.find(basicBlock) == predecessorsMap.end() && basicBlock != firstBB)
        {
            blocksToDelete.push_back(basicBlock);
        }
    }

    // delete blocks all blocks without predecessors from parent function
    for (LLVMBasicBlockRef block : blocksToDelete)
    {
        LLVMDeleteBasicBlock(block);
    }
    blocksToDelete.clear();
    predecessorsMap.clear();
}

LLVMBasicBlockRef handle_block_statement(astStmt *statement, LLVMBasicBlockRef entryBasicBlock)
{
    // printf("BLOCK STATEMENT IS CALLED\n");
    LLVMBasicBlockRef prevBasicBlock = entryBasicBlock;

    vector<astNode *> statementList = *(statement->block.stmt_list);
    for (auto node = statementList.begin(); node != statementList.end(); ++node)
    {
        prevBasicBlock = statement_evaluation(&(*node)->stmt, prevBasicBlock);
    }

    return prevBasicBlock;
}

LLVMBasicBlockRef handle_return_statement(astStmt *statement, LLVMBasicBlockRef entryBasicBlock)
{
    LLVMPositionBuilderAtEnd(BUILDER, entryBasicBlock);

    LLVMValueRef loadedReturnValRef = traverse_tree(statement->ret.expr);
    LLVMValueRef returnAllocInstruction = NAME_ALLOCS_MAP["ret"];
    LLVMBuildStore(BUILDER, loadedReturnValRef, returnAllocInstruction);

    LLVMBuildBr(BUILDER, RETURN_BASICBLOCK);

    LLVMBasicBlockRef endBB = LLVMAppendBasicBlock(MAIN_FUNC, "");
    return endBB;
}

LLVMBasicBlockRef handle_if_statement(astStmt *statement, LLVMBasicBlockRef entryBasicBlock)
{
    LLVMPositionBuilderAtEnd(BUILDER, entryBasicBlock);
    LLVMValueRef rexprRef = traverse_tree(statement->ifn.cond);

    LLVMBasicBlockRef trueBasicBlock = LLVMAppendBasicBlock(MAIN_FUNC, "");
    LLVMBasicBlockRef falseBasicBlock = LLVMAppendBasicBlock(MAIN_FUNC, "");

    LLVMBuildCondBr(BUILDER, rexprRef, trueBasicBlock, falseBasicBlock);

    if (statement->ifn.else_body == NULL)
    {
        LLVMBasicBlockRef exitBasicBlock = statement_evaluation(&statement->ifn.if_body->stmt, trueBasicBlock);
        LLVMPositionBuilderAtEnd(BUILDER, exitBasicBlock);

        LLVMBuildBr(BUILDER, falseBasicBlock);

        return falseBasicBlock;
    }

    LLVMBasicBlockRef ifExitBasicBlock = statement_evaluation(&statement->ifn.if_body->stmt, trueBasicBlock);
    LLVMBasicBlockRef elseExitBasicBlock = statement_evaluation(&statement->ifn.else_body->stmt, falseBasicBlock);

    LLVMBasicBlockRef ifElseBlocksMerger = LLVMAppendBasicBlock(MAIN_FUNC, "");

    LLVMPositionBuilderAtEnd(BUILDER, ifExitBasicBlock);
    LLVMBuildBr(BUILDER, ifElseBlocksMerger);

    LLVMPositionBuilderAtEnd(BUILDER, elseExitBasicBlock);
    LLVMBuildBr(BUILDER, ifElseBlocksMerger);

    return ifElseBlocksMerger;
}

LLVMBasicBlockRef handle_while_statement(astStmt *statement, LLVMBasicBlockRef entryBasicBlock)
{
    LLVMPositionBuilderAtEnd(BUILDER, entryBasicBlock);

    LLVMBasicBlockRef conditionBasicBlock = LLVMAppendBasicBlock(MAIN_FUNC, "");

    LLVMBuildBr(BUILDER, conditionBasicBlock);

    LLVMPositionBuilderAtEnd(BUILDER, conditionBasicBlock);

    LLVMValueRef cmpInstruction = traverse_tree(statement->whilen.cond);

    LLVMBasicBlockRef trueBasicBlock = LLVMAppendBasicBlock(MAIN_FUNC, "");
    LLVMBasicBlockRef falseBasicBlock = LLVMAppendBasicBlock(MAIN_FUNC, "");

    LLVMBuildCondBr(BUILDER, cmpInstruction, trueBasicBlock, falseBasicBlock);

    astStmt *blockStatement = &statement->whilen.body->stmt;
    LLVMBasicBlockRef trueExitBasicBlock = statement_evaluation(blockStatement, trueBasicBlock);

    LLVMPositionBuilderAtEnd(BUILDER, trueExitBasicBlock);
    LLVMBuildBr(BUILDER, conditionBasicBlock);

    return falseBasicBlock;
}

LLVMBasicBlockRef handle_assign_statement(astStmt *statement, LLVMBasicBlockRef entryBasicBlock)
{
    LLVMPositionBuilderAtEnd(BUILDER, entryBasicBlock);
    string LHSVarName = statement->asgn.lhs->var.name;
    LLVMValueRef LHSAllocInstruction = NAME_ALLOCS_MAP[LHSVarName];

    LLVMValueRef RHSExprResult = NULL;
    if (statement->asgn.rhs != NULL && statement->asgn.rhs->type == ast_stmt)
    {

        if (statement->asgn.rhs->stmt.type == ast_call)
        {
            assert(READ_FUNC.returnType != NULL);
            vector<LLVMValueRef> args;
            RHSExprResult = LLVMBuildCall2(BUILDER, READ_FUNC.returnType, READ_FUNC.function, args.data(), args.size(), "");
        }
    }
    else
    {
        RHSExprResult = traverse_tree(statement->asgn.rhs);
    }

    LLVMBuildStore(BUILDER, RHSExprResult, LHSAllocInstruction);

    assert(entryBasicBlock != NULL);
    return entryBasicBlock;
}

LLVMValueRef create_function(int numParams, bool hasReturn, const char *funcName, bool IsVarArg)
{
    LLVMTypeRef *params = (LLVMTypeRef *)malloc(numParams * sizeof(LLVMTypeRef));
    for (int i = 0; i < numParams; i++)
    {
        params[i] = LLVMInt32Type();
    }

    LLVMTypeRef printParams[] = {LLVMInt32Type()};
    LLVMTypeRef returnType = hasReturn ? LLVMFunctionType(LLVMInt32Type(), params, numParams, IsVarArg) : LLVMFunctionType(LLVMVoidType(), printParams, numParams, IsVarArg);

    if (strcmp(funcName, "read") == 0)
    {
        READ_FUNC.returnType = returnType;
    }
    else if (strcmp(funcName, "print") == 0)
    {
        PRINT_FUNC.returnType = returnType;
    }

    free(params);
    return LLVMAddFunction(MODULE, funcName, returnType);
}

LLVMValueRef compare_lhs_rhs(rop_type operation, LLVMValueRef LHSLoad, LLVMValueRef RHSLoad)
{
    switch (operation)
    {
    case lt:
        return LLVMBuildICmp(BUILDER, LLVMIntSLT, LHSLoad, RHSLoad, "");
    case gt:
        return LLVMBuildICmp(BUILDER, LLVMIntSGT, LHSLoad, RHSLoad, "");
    case le:
        return LLVMBuildICmp(BUILDER, LLVMIntSLE, LHSLoad, RHSLoad, "");
    case ge:
        return LLVMBuildICmp(BUILDER, LLVMIntSGE, LHSLoad, RHSLoad, "");
    case eq:
        return LLVMBuildICmp(BUILDER, LLVMIntEQ, LHSLoad, RHSLoad, "");
    case neq:
        return LLVMBuildICmp(BUILDER, LLVMIntNE, LHSLoad, RHSLoad, "");
    default:
        break;
    }
    return NULL;
}

LLVMValueRef create_binary_op(op_type operation, LLVMValueRef LHSLoad, LLVMValueRef RHSLoad)
{
    switch (operation)
    {
    case add:
        return LLVMBuildAdd(BUILDER, LHSLoad, RHSLoad, "");
    case sub:
        return LLVMBuildSub(BUILDER, LHSLoad, RHSLoad, "");
    case mul:
        return LLVMBuildMul(BUILDER, LHSLoad, RHSLoad, "");
    case divide:
        return LLVMBuildUDiv(BUILDER, LHSLoad, RHSLoad, "");
    case uminus:
    {
        unsigned long long value = 0;
        LLVMValueRef constValue = LLVMConstInt(LLVMInt32Type(), value, 1);
        return LLVMBuildSub(BUILDER, constValue, RHSLoad, "");
    }
    default:
        break;
    }
    return NULL;
}

LLVMValueRef create_load_instruction(string varName)
{
    LLVMValueRef allocInstruction = NAME_ALLOCS_MAP[varName];

    return LLVMBuildLoad2(BUILDER, LLVMInt32Type(), allocInstruction, "");
}

// return position of the builder
LLVMBasicBlockRef setup_and_create_variable_to_alloc_map()
{
    MAIN_FUNC = create_function(1, true, "main-func", false);
    LLVMBasicBlockRef firstBasicBlock = LLVMAppendBasicBlock(MAIN_FUNC, "");

    // instantiate builder and set postion to keep track of position
    BUILDER = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(BUILDER, firstBasicBlock);

    // create alloc statements - the last "ret"
    for (const auto &name : VARIABLE_NAMES)
    {
        NAME_ALLOCS_MAP[name] = LLVMBuildAlloca(BUILDER, LLVMInt32Type(), "");
        LLVMSetAlignment(NAME_ALLOCS_MAP[name], 4);
    }

    // create store instruction for param
    LLVMValueRef allocInstForParam = NAME_ALLOCS_MAP["param"];
    LLVMBuildStore(BUILDER, LLVMGetParam(MAIN_FUNC, 0), allocInstForParam);

    RETURN_BASICBLOCK = LLVMAppendBasicBlock(MAIN_FUNC, "");
    LLVMPositionBuilderAtEnd(BUILDER, RETURN_BASICBLOCK);

    LLVMValueRef loadInstForReturn = create_load_instruction("ret");

    LLVMBuildRet(BUILDER, loadInstForReturn); // use?

    return firstBasicBlock;
}

LLVMBasicBlockRef statement_evaluation(astStmt *statement, LLVMBasicBlockRef entryBasicBlock)
{
    assert(statement != NULL && entryBasicBlock != NULL);
    switch (statement->type)
    {
    case ast_block:
        return handle_block_statement(statement, entryBasicBlock);

    case ast_decl:
        return entryBasicBlock;
        break;

    case ast_call:
        if (statement->call.param != NULL)
        {
            LLVMPositionBuilderAtEnd(BUILDER, entryBasicBlock);
            LLVMValueRef constToPrint = traverse_tree(statement->call.param);
            LLVMValueRef args[] = {constToPrint};

            LLVMBuildCall2(BUILDER, PRINT_FUNC.returnType, PRINT_FUNC.function, args, 1, "");
            return entryBasicBlock;
        }
        break;

    case ast_ret:
        return handle_return_statement(statement, entryBasicBlock);

    case ast_while:
        return handle_while_statement(statement, entryBasicBlock);

    case ast_if:
        return handle_if_statement(statement, entryBasicBlock);

    case ast_asgn:
        return handle_assign_statement(statement, entryBasicBlock);

    default:
        break;
    }
    return NULL;
}

LLVMValueRef traverse_tree(astNode *node)
{
    assert(node != NULL);

    switch (node->type)
    {
    case ast_cnst:
    {
        unsigned long long value = node->cnst.value;
        return LLVMConstInt(LLVMInt32Type(), value, 1); // action?
    }

    case ast_var:
        return create_load_instruction(node->var.name);

    case ast_uexpr:
    {
        // double check this
        LLVMValueRef RHSLoad = traverse_tree(node->uexpr.expr);
        return create_binary_op(uminus, NULL, RHSLoad);
    }

    case ast_bexpr:
    {
        LLVMValueRef LHSLoad = traverse_tree(node->bexpr.lhs);
        LLVMValueRef RHSLoad = traverse_tree(node->bexpr.rhs);

        return create_binary_op(node->bexpr.op, LHSLoad, RHSLoad);
    }

    case ast_rexpr:
    {
        LLVMValueRef LHSLoad = traverse_tree(node->rexpr.lhs);
        LLVMValueRef RHSLoad = traverse_tree(node->rexpr.rhs);

        return compare_lhs_rhs(node->rexpr.op, LHSLoad, RHSLoad);
    }
    default:
        break;
    }
    return NULL;
}

void traverse_prog_func(astNode *node, const char *filename)
{
    if (node->type == ast_prog)
    {
        MODULE = LLVMModuleCreateWithName(filename);
        if (MODULE == NULL)
        {
            fprintf(stderr, "Failed to create LLVM module\n");
            exit(EXIT_FAILURE);
        }
        LLVMSetTarget(MODULE, "x86_64-pc-linux-gnu");

        READ_FUNC.function = create_function(0, true, "read", false);
        PRINT_FUNC.function = create_function(1, false, "print", false);
        traverse_prog_func(node->prog.func, filename);
    }
    else if (node->type == ast_func)
    {

        LLVMBasicBlockRef firstBasicBlock = setup_and_create_variable_to_alloc_map();

        LLVMBasicBlockRef exitBasicBlock = statement_evaluation(&node->func.body->stmt, firstBasicBlock);

        if (LLVMGetBasicBlockTerminator(exitBasicBlock) == NULL)
        {
            LLVMPositionBuilderAtEnd(BUILDER, exitBasicBlock);
            LLVMBuildBr(BUILDER, RETURN_BASICBLOCK);
        }

        map<LLVMBasicBlockRef, set<LLVMBasicBlockRef>> predecessorsMap = getPredcessorsMap(MAIN_FUNC);
        remove_basic_blocks_without_predecessors(predecessorsMap);
    }
}

LLVMModuleRef build_ir(astNode *root, const char *filename)
{
    VARIABLE_NAMES = get_variable_names(root);
    traverse_prog_func(root, filename);
    LLVMDisposeBuilder(BUILDER);
    return MODULE;
}