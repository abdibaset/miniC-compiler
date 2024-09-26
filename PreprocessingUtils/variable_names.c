#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Types.h>
#include <stdio.h>
#include <stdlib.h>

#include <cassert>
#include <map>
#include <string>
#include <vector>

#include "../ast/ast.h"
#include "pre_processing.h"
using namespace std;

void traverse_all_statements_in_block_names(astStmt *statement);
void statement_evaluation_names(astStmt *statement);
void traverse_tree_names(astNode *node);

vector<string> NEW_NAMES_SET;

/**
 * @details helper function to traverse all astNodes in a block statement
 *
 * @param statement - block statement with a statement list to be traversed
 */
void traverse_all_statements_in_block_names(astStmt *statement) {
    vector<astNode *> statementList = *(statement->block.stmt_list);
    for (auto node = statementList.begin(); node != statementList.end();
         ++node) {
        traverse_tree_names(*node);
    }
}

/**
 * @details checks for the type of statements block or declaration to perform
 * actions
 *
 * @param statement - current statement being evaluated
 * @param isFunction - true if the current statement is function
 *
 */
void statement_evaluation_names(astStmt *statement) {
    assert(statement != NULL);

    switch (statement->type) {
        case ast_block:
            traverse_all_statements_in_block_names(statement);
            break;

        case ast_decl:
            NEW_NAMES_SET.push_back(statement->decl.name);
            break;

        case ast_call:
            if (statement->call.param != NULL) {
                traverse_tree_names(statement->call.param);
            }
            break;

        case ast_ret:
            traverse_tree_names(statement->ret.expr);
            break;

        case ast_while:
            traverse_tree_names(statement->whilen.cond);
            traverse_tree_names(statement->whilen.body);
            break;

        case ast_if:
            traverse_tree_names(statement->ifn.cond);
            traverse_tree_names(statement->ifn.if_body);
            if (statement->ifn.else_body != NULL) {
                traverse_tree_names(statement->ifn.else_body);
            }
            break;

        case ast_asgn:
            traverse_tree_names(statement->asgn.lhs);
            traverse_tree_names(statement->asgn.rhs);
            break;

        default:
            break;
    }
}

/**
 * @details This function traverses the ast tree recursively except for extern
 * nodes, starting with the root and checks:
 *  1. if node is a statement - check the for the types of statement - either
 * block or declaration are relevent
 *  2. if function - assign @param isFunction to true
 *  3. if a variable - check for declaration starting with the top of stack
 * backwards
 *  4. the rest of the nodes are traversed
 *
 * @param node - current node in the ast tree being traversed
 * @param isFunction - true if the block to be recursed over is a function -
 * this helps in the evaluation of the block statement
 */
void traverse_tree_names(astNode *node) {
    assert(node != NULL);
    astStmt *statement;

    switch (node->type) {
        case ast_prog:
            traverse_tree_names(node->prog.func);
            break;

        case ast_stmt:
            statement = &node->stmt;
            statement_evaluation_names(
                statement);  // check the statement type and content
            break;

        case ast_func:

            // add param to top symbol table
            if (node->func.param != NULL) {
                NEW_NAMES_SET.push_back(node->func.param->var.name);
            }
            statement = &(node->func.body->stmt);
            statement_evaluation_names(statement);  // evaluate the block
            break;

        case ast_var:
            break;

        case ast_bexpr:
            traverse_tree_names(node->bexpr.lhs);
            traverse_tree_names(node->bexpr.rhs);
            break;

        case ast_rexpr:
            traverse_tree_names(node->rexpr.lhs);
            traverse_tree_names(node->rexpr.rhs);
            break;

        case ast_cnst:
            break;

        case ast_uexpr:
            traverse_tree_names(node->uexpr.expr);
            break;

        default:
            fprintf(stderr, "Invalid node type: Ignored\n");
            break;
    }
}

vector<string> get_variable_names(astNode *root) {
    traverse_tree_names(root);

    NEW_NAMES_SET.push_back("ret");
    return NEW_NAMES_SET;
}