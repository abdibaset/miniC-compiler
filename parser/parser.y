/**
 * @author Abdibaset Bare
 * @create date 2024-04-12 17:22:44
 * @modify date 2024-04-12 18:23:08
 * @desc - this file defines the grammar rules for a given c program 
 */
%{
#include <stdio.h>
#include <iostream>
#include "ast.h"
#include <vector>
using namespace std;''
%}

%union {
    astNode *node;
    char varname;
    int numval;
    vector <astNode*> *astvec_ptr;
}

%token IF ELSE WHILE INT CHAR RETURN VOID EXTERN PRINT READ EQ GREATER GREATEROREQ LESS LESSOREQ
%token <varname> VARID
%token <numval> NUMBER
%type <astvec_ptr> all_statements variableDeclarations block all_if_blocks all_while_blocks 
%type<node> extern_func term declaration  statement expression if_block while_block return statements condition declarations  function program
%starts program


%%
program                 : extern_func extern_func function { $$ = createProgram($1, $2, $3);}
extern_func             : EXTERN INT READ '(' ')' ';'   {$$ = createExtern("read");}
                        | EXTERN VOID PRINT '(' INT ')' ';' {$$ = createExtern("print");}

function                : INT VARID '(' VARID ')' block  { $$ = createFunc($2, $4, $6);
                                                           free($2); free($6);}   


condition               : term EQ term          { $$ = createRExpr($1, $3, "==");}
                        | term GREATER term     { $$ = createRExpr($1, $3, ">");}
                        | term LESS term        { $$ = createRExpr($1, $3, "<");}
                        | term GREATEROREQ term { $$ = createRExpr($1, $3, ">=");}
                        | term LESSOREQ term    { $$ = createRExpr($1, $3, "<=");}


block                   : '{' variableDeclarations all_statements all_if_blocks all_while_blocks'}' {   vector<astNode*> *node_vect = new vector<astNode*> ();
                                                                                                        node_vect->insert(node_vect.end(), $2->begin(), $2->end());
                                                                                                        node_vect->insert(node_vect.end(), $3->begin(), $3->end());
                                                                                                        node_vect->insert(node_vect.end(), $4->begin(), $4->end());
                                                                                                        node_vect->insert(node_vect.end(), $5->begin(), $5->end();)
                                                                                                        $$ = createBlock(node_vect);
                                                                                                        delete($2); delete($3); delete($4); delete($4);}
                        | '{' all_statements all_if_blocks all_while_blocks '}'                     {   vector<astNode*> *node_vect = new vector<astNode*> ();
                                                                                                        node_vect->insert(node_vect.end(), $2->begin(), $2->end());
                                                                                                        node_vect->insert(node_vect.end(), $3->begin(), $3->end());
                                                                                                        node_vect->insert(node_vect.end(), $4->begin(), $4->end());
                                                                                                        $$ = createBlock(node_vect);
                                                                                                        delete($2); delete($3); delete($4);}
                        | '{' all_if_blocks all_while_blocks '}'                                    {   vector<astNode*> *node_vect = new vector<astNode*> ();
                                                                                                        node_vect->insert(node_vect.end(), $2->begin(), $2->end());
                                                                                                        node_vect->insert(node_vect.end(), $3->begin(), $3->end());
                                                                                                        $$ = createBlock(node_vect);
                                                                                                        delete($2); delete($3);}
                        | '{' all_if_blocks '}'                                                     {   $$ = createBlock($2);}
                        | '{' all_while_blocks '}'                                                  {   $$ = createBlock($2);}
                        | '{' all_statements '}'                                                    {   $$ = createBlock($2);}    

all_if_blocks           : if_blocks if_block        {$$ = $1;
                                                     $$->push_back($2);}
                        | if_block                  {$$ = new vector<astNode*>();
                                                     $$->push_back($1);}

all_while_blocks        : while_blocks while_block      {$$ = $1;
                                                        $$->push_back($2);}
                        | while_block                   {$$ = new vector<astNode*>();
                                                        $$->push_back($1);} 

if_block                : IF '(' condition ')' block else_block { $$ = createIf($2, )}
else_block              : ELSE '{' block '}'  
while_block             : WHILE '(' condition ')' '{' all_statements '}' { $$ = createWhile($3, $6);}

all_statements          : statements statement      { $$ = $1
                                                      $$->push_back($1);}
                        | statement                 { $$ = new vector<astNode>();
                                                      $$->push_back($1);}

statement               : VARID '=' expression      { astNode* stmt_ptr = createVar($1);
                                                      $$ = createAsgn(stmt_ptr, $3);}
                        | VARID '=' READ '(' ')'    { $$ = createCall("read");
                                                      free($1);}
                        | PRINT '(' VARID ')'       { $$ = createCall("print", $3);
                                                      free($3);}  
            
variableDeclarations    : declarations declaration      { $$ = $1;
                                                          $$->push_back($2);}

                        | declaration                   { $$ = new vector<astNode*>();
                                                          $$->push_back($1);}
declaration             : INT VARID ';'     { $$ = createDecl($2);
                                              free($2);}
                        | CHAR VARID ';'    { $$ = createDecl($2);
                                              free($1);}

                      
expression              : term '+' term     { $$ = createBExpr($1, $3, $2);}
                        | term '-' term     { $$ = createBExpr($1, $3, $2);}
                        | term '*' term     { $$ = createBExpr($1, $3, $2);}
                        | term '/' term     { $$ = createBExpr($1, $3, $2);}
                        | term              { $$ = $1;}

term                    : NUMBER    { $$ = createCnst($1);}
                        | VARID     { $$ = createVar($1);
                                      free($1);}
                        | '-' NUMBER {$$ = createUExpr($2, "uminus");}

return                  : RETURN '(' expression ')' ';' { $$ = createRet($3);}
                        | RETURN expression ';'   { $$ = createRet($2);}
                        | RETURN ';'    { $$ = createRet(NULL);}
%%