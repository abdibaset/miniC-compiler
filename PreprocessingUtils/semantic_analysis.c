/**
 * @author Abdibaset Bare
 * @create date 2024-04-12 17:22:44
 * @modify date 2024-04-18 16:50:48
 * @desc - this file checks the correctness of the ast tree built
 */

#include "../ast/ast.h"
#include "semantic_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <vector>
using namespace std;

vector<vector<char *>> stack;
bool IS_VARIABLE_DECLARE_MORE_THAN_ONCE_IN_SCOPE = false;
bool IS_REFERENCED_B4_DECL = false;

// prototypes
void traverse_tree(astNode *node, bool isFunction = false);
int isVariableDeclared(astNode *node);
void statement_evaluation(astStmt *statement, bool isFunction);
void traverse_all_statements_in_block(astStmt *statement);

/**
 * @details this is  helper method that checks if a variable has been declared from top of stack to bottom
 *
 * @return 1 - variable has been declared in a valid scope - current scope or global scope
 * @return 0 - variable not declared in valid scope/ referenced before declaration
 *
 */
int isVariableDeclared(astNode *node)
{
    assert(!stack.empty());

    for (int stackIndex = stack.size() - 1; stackIndex >= 0; stackIndex--)
    {
        vector<char *> symTable = stack[stackIndex];
        for (int symTableIndex = 0; symTableIndex < (int)symTable.size(); symTableIndex++)
        {
            char *varName = symTable[symTableIndex];
            if (strcmp(varName, node->var.name) == 0)
            {
                return 1;
            }
        }
    }
    return 0;
}

/**
 * @details helper function to traverse all astNodes in a block statement
 *
 * @param statement - block statement with a statement list to be traversed
 */
void traverse_all_statements_in_block(astStmt *statement)
{
    vector<astNode *> statementList = *(statement->block.stmt_list);
    for (auto node = statementList.begin(); node != statementList.end(); ++node)
    {
        traverse_tree(*node);
    }
}
/**
 * @details checks for the type of statements block or declaration to perform actions
 *
 * @param statement - current statement being evaluated
 * @param isFunction - true if the current statement is function
 *
 */
void statement_evaluation(astStmt *statement, bool isFunction)
{
    assert(statement != NULL);
    vector<char *> curr_symbol_table;
    vector<char *>::iterator it;
    switch (statement->type)
    {
    case ast_block:
        if (isFunction)
        {
            traverse_all_statements_in_block(statement);
        }
        else
        {
            // curr_symbol_table = new vector<char*>();
            curr_symbol_table.clear();
            stack.push_back(curr_symbol_table);
            traverse_all_statements_in_block(statement);
            stack.pop_back();
        }
        break;

    case ast_decl:
        curr_symbol_table = stack.back();
        it = curr_symbol_table.begin();
        while (it != curr_symbol_table.end())
        {
            if (strcmp(*it, statement->decl.name) == 0)
            {
                fprintf(stderr, "Variable %s has been declared more than once in this scope\n", statement->decl.name);
                IS_VARIABLE_DECLARE_MORE_THAN_ONCE_IN_SCOPE = true;
                break;
            }
            it++;
        }
        stack.back().push_back(statement->decl.name);
        break;

    case ast_call:
        if (statement->call.param != NULL)
        {
            traverse_tree(statement->call.param);
        }
        break;

    case ast_ret:
        traverse_tree(statement->ret.expr);
        break;

    case ast_while:
        traverse_tree(statement->whilen.cond);
        traverse_tree(statement->whilen.body);
        break;

    case ast_if:
        traverse_tree(statement->ifn.cond);
        traverse_tree(statement->ifn.if_body);
        if (statement->ifn.else_body != NULL)
        {
            traverse_tree(statement->ifn.else_body);
        }
        break;

    case ast_asgn:
        traverse_tree(statement->asgn.lhs);
        traverse_tree(statement->asgn.rhs);
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
void traverse_tree(astNode *node, bool isFunction)
{
    assert(node != NULL);
    vector<char *> curr_symbol_table;
    astStmt *statement;

    switch (node->type)
    {
    case ast_prog:
        traverse_tree(node->prog.func);
        break;

    case ast_stmt:
        statement = &node->stmt;
        statement_evaluation(statement, isFunction); // check the statement type and content
        break;

    case ast_func:
        statement = &(node->func.body->stmt);
        curr_symbol_table.clear();
        stack.push_back(curr_symbol_table);

        // add param to top symbol table
        if (node->func.param != NULL)
        {
            stack.back().push_back(node->func.param->var.name);
        }

        statement_evaluation(statement, true); // evaluate the block
        stack.pop_back();
        break;

    case ast_var:
        if (!isVariableDeclared(node))
        {
            fprintf(stderr, "Variable %s referenced before declaration in valid scope\n", node->var.name);
            IS_REFERENCED_B4_DECL = true; // if variable is being referenced before declared in a valid scope
        }
        break;

    case ast_bexpr:
        traverse_tree(node->bexpr.lhs);
        traverse_tree(node->bexpr.rhs);
        break;

    case ast_rexpr:
        traverse_tree(node->rexpr.lhs);
        traverse_tree(node->rexpr.rhs);
        break;

    case ast_cnst:
        break;

    case ast_uexpr:
        traverse_tree(node->uexpr.expr);
        break;

    default:
        fprintf(stderr, "Invalid node type: Ignored\n");
        break;
    }
}

/**
 * @details endpoint function for the semantic analysis module
 *
 * @param root - root of the ast tree
 *
 * @return false - if not successful, true otherwise
 */

bool is_semantically_correct(astNode *root)
{
    astNode *node = root;

    traverse_tree(node, false);
    if (IS_REFERENCED_B4_DECL || IS_VARIABLE_DECLARE_MORE_THAN_ONCE_IN_SCOPE)
    {
        return false;
    }

    return true;
}
