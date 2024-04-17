#include <stdio.h>
#include <stdlib.h>
#include "../utils/ast.h"
#include "y.tab.h"
#include <cassert>
#include <cstring>
#include <vector>
using namespace std;

extern FILE *yyin;
extern int yylex();
extern int yylex_destroy();
extern astNode *root;

vector<vector<astNode*>> stack; 
astNode *node = root;

void traverse_tree(astNode *node, bool isFunction);
int isVariableDeclared();
void statement_eval(astStmt *stmt, bool isFunction);

/**
 * @details This function traverses the ast tree recursively and does the following
 *  1. if node is a statement - check the for the types of statement - either block or declaration are relevent 
 *  2. if function - assign @param isFunction to true 
 *  3. if a variable - check for declaration starting with the top of stack backwards 
 * 
 * @param node - current node in the ast tree being traversed 
 * @param isFunction - true if the block to be recursed over is a function - this helps in the evaluation of the block statement
*/
void traverse_tree (astNode *node, bool isFunction=false){
    assert(node != NULL);
    vector<astNode*> *curr_symbol_table = NULL;
    astStmt stmt;

    switch (node->type) {
        case ast_stmt:
            stmt = node->stmt;
            statement_eval(&stmt, isFunction);
        case ast_func:
            curr_symbol_table = new vector<astNode*>();
            stack.push_back(*curr_symbol_table);
            if (node->func.param != NULL){ stack.back().push_back(node->func.param);}

            node = node->func.body;
            stack.pop_back();
            traverse_tree(node, isFunction=true); 
        case ast_var:
            if (isVariableDeclared()) {
                fprintf(stderr, "Variable not declared in scope\n");
                exit(EXIT_FAILURE);
            }
        default:
            fprintf(stderr, "Invalid node\n");
            exit(EXIT_FAILURE);
            break;
    }
}

/**
 * @details this is  helper method that checks if a variable has been declared from top of stack to bottom 
 * 
*/
int isVariableDeclared(){
    assert (!stack.empty());

    for (int stackIndex = stack.size()-1; stackIndex >= 0; stackIndex--) {
        vector<astNode*> symTable = stack[stackIndex];
        for (int symTableIndex = 0; symTableIndex < (int)symTable.size(); symTableIndex++){
            astNode* element = symTable[symTableIndex];
            if (strcmp(element->var.name, node->var.name))
                return 1;
        }
    }
    return 0;
}

/**
 * @details checks for the type of statements block or declaration to perform actions
 * 
 * @param statement - current statement being evaluated 
 * @param isFunction - true if the current statement is function
 * 
*/
void statement_eval(astStmt *statement, bool isFunction){
    vector<astNode*> *curr_symbol_table = NULL;
    switch (statement->type){
        case ast_block:
            if (isFunction) {
                curr_symbol_table = &stack.back();
                for (astNode* stmt : *(statement->block.stmt_list)) {
                    traverse_tree(stmt, isFunction);
                }
            } 
            else {
                curr_symbol_table = new vector<astNode*>();
                stack.push_back(*curr_symbol_table);
                for (astNode* stmt : *(statement->block.stmt_list)) {
                    traverse_tree(stmt, isFunction);
                }
                stack.pop_back();
            }
        case ast_decl:
            // check if declared
            curr_symbol_table = &stack.back();
            for (astNode* node : *curr_symbol_table) {
                if (node->type == ast_stmt && node->stmt.type == ast_decl && strcmp(node->stmt.decl.name, statement->decl.name) == 0) {
                    fprintf(stderr, "undeclared error\n");
                    break;
                }
            }
            stack.back().push_back((astNode*)(&statement->decl)); // check with TA
        default:
            break;
    }
}


int main(int argc, char** argv) {
    if (argc != 2){
        printf("Too few arguments");
        exit(EXIT_FAILURE);
    }

    #ifdef YYDEBUG
        yydebug = 1;
    #endif

    yyin = fopen(argv[1], "r");
    if (yyin == NULL){
        fprintf(stderr, "Error reading file\n");
        exit(EXIT_FAILURE);
    }

    yyparse();
    fclose(yyin);

    void snytax_analysis();
    free(root);
    yylex_destroy();

    exit(EXIT_SUCCESS);
}