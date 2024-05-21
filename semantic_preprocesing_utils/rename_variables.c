/**
 * @author Abdibaset Bare
 * @create date 2024-04-12 17:22:44
 * @modify date 2024-04-18 16:50:48
 * @desc - this file checks the correctness of the ast tree built
 */
#include "pre_processing.h"
#include "../ast/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Types.h>
#include <cassert>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <set>
using namespace std;

// prototypes
void traverse_all_statements_in_block_pre_processing(astStmt *statement);
void statement_evaluation_pre_processing(astStmt *statment);
void traverse_tree_pre_processing(astNode *node);

vector<map<string, string>> OLD_NEW_NAMES;
int BLOCK_NUM = 0;

void add_param_to_map(astNode *node, char *varName)
{
    string paramName = "param";
    string oldParamName = varName;

    map<string, string> &curr_scope_map = OLD_NEW_NAMES.back();

    const char *paramNameCStr = paramName.c_str();
    curr_scope_map[oldParamName] = paramName;
    OLD_NEW_NAMES.push_back(curr_scope_map);
    // replace name
    // strcpy(node->func.param->var.name, paramNameCStr);

    free(node->func.param->var.name);
    node->func.param->var.name = strdup(paramNameCStr);
}

void add_declared_var_to_map(astStmt *statement)
{
    string oldname = statement->decl.name;
    string newName = oldname + to_string(BLOCK_NUM);

    map<string, string> &curr_scope_map = OLD_NEW_NAMES.back();
    curr_scope_map[oldname] = newName;

    // strcpy(statement->decl.name, newName.c_str());

    free(statement->decl.name);
    statement->decl.name = strdup(newName.c_str());
}

/**
 * @details helper function to traverse all astNodes in a block statement
 *
 * @param statement - block statement with a statement list to be traversed
 */
void traverse_all_statements_in_block_pre_processing(astStmt *statement)
{
    vector<astNode *> statementList = *(statement->block.stmt_list);
    for (auto node = statementList.begin(); node != statementList.end(); ++node)
    {
        traverse_tree_pre_processing(*node);
    }
}

/**
 * @details checks for the type of statements block or declaration to perform actions
 *
 * @param statement - current statement being evaluated
 * @param isFunction - true if the current statement is function
 *
 */
void statement_evaluation_pre_processing(astStmt *statement)
{
    assert(statement != NULL);
    string oldname;
    string newName;
    map<string, string> curr_scope;

    switch (statement->type)
    {
    case ast_block:

        // names map old to new
        OLD_NEW_NAMES.push_back(curr_scope);
        BLOCK_NUM++; // number of blocks in scope

        traverse_all_statements_in_block_pre_processing(statement);
        OLD_NEW_NAMES.pop_back();
        break;

    case ast_decl:
        add_declared_var_to_map(statement);
        break;

    case ast_call:
        if (statement->call.param != NULL)
        {
            traverse_tree_pre_processing(statement->call.param);
        }
        break;

    case ast_ret:
        traverse_tree_pre_processing(statement->ret.expr);
        break;

    case ast_while:
        traverse_tree_pre_processing(statement->whilen.cond);
        traverse_tree_pre_processing(statement->whilen.body);
        break;

    case ast_if:
        traverse_tree_pre_processing(statement->ifn.cond);
        traverse_tree_pre_processing(statement->ifn.if_body);
        if (statement->ifn.else_body != NULL)
        {
            traverse_tree_pre_processing(statement->ifn.else_body);
        }
        break;

    case ast_asgn:
        traverse_tree_pre_processing(statement->asgn.lhs);
        traverse_tree_pre_processing(statement->asgn.rhs);
        break;

    default:
        break;
    }
}

/**
 * @details This function traverses the ast tree recursively except for extern nodes, starting with the root and checks:
 *  1. if node is a statement - check the for the types of statement - either block or declaration are relevent
 *  2. if function - assign @param isFunction to true
 *  3. if a variable - check for declaration starting with the top of stack backwards
 *  4. the rest of the nodes are traversed
 *
 * @param node - current node in the ast tree being traversed
 * @param isFunction - true if the block to be recursed over is a function - this helps in the evaluation of the block statement
 */
void traverse_tree_pre_processing(astNode *node)
{
    assert(node != NULL);
    astStmt *statement;
    map<string, string> curr_scope_map;

    switch (node->type)
    {
    case ast_prog:
        traverse_tree_pre_processing(node->prog.func);
        break;

    case ast_stmt:
        statement = &node->stmt;
        statement_evaluation_pre_processing(statement); // check the statement type and content
        break;

    case ast_func:
        statement = &(node->func.body->stmt);

        // add param to top symbol table
        if (node->func.param != NULL)
        {
            // old new names
            OLD_NEW_NAMES.push_back(curr_scope_map);
            add_param_to_map(node, node->func.param->var.name);
        }

        statement_evaluation_pre_processing(statement); // evaluate the block
        OLD_NEW_NAMES.pop_back();
        break;

    case ast_var:
        for (int i = OLD_NEW_NAMES.size() - 1; i >= 0; --i)
        {
            map<string, string> &scopeMap = OLD_NEW_NAMES[i];
            if (scopeMap.find(node->var.name) != scopeMap.end())
            {

                string newName = scopeMap[string(node->var.name)];
                // strcpy(node->var.name, newName.c_str());z
                free(node->var.name);
                node->var.name = strdup(newName.c_str());
                break;
            }
        }
        break;

    case ast_bexpr:
        traverse_tree_pre_processing(node->bexpr.lhs);
        traverse_tree_pre_processing(node->bexpr.rhs);
        break;

    case ast_rexpr:
        traverse_tree_pre_processing(node->rexpr.lhs);
        traverse_tree_pre_processing(node->rexpr.rhs);
        break;

    case ast_cnst:
        break;

    case ast_uexpr:
        traverse_tree_pre_processing(node->uexpr.expr);
        break;

    default:
        fprintf(stderr, "Invalid node type: Ignored\n");
        break;
    }
}

astNode *rename_variables_in_ast_tree(astNode *root)
{
    assert(root != NULL);
    astNode *rootCopy = root;
    traverse_tree_pre_processing(root);

    return rootCopy;
}
