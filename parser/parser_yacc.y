%{
#include <stdio.h>
#include <iostream>
#include "ast.h"
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
%type <astvec_ptr> all_statements variableDeclarations block
%type<node> extern_func term declaration  statement expression if_block while_block return statements condition declarations  function
%starts extern_func


%%
extern_func             : EXTERN INT READ '(' ')' ';'   {$$ = createExtern("read");}
                        | EXTERN VOID PRINT '(' INT ')' ';' {$$ = createExtern("print");}

function                : INT VARID '(' VARID ')' block  { $$ = createFunc($2, $4, $6);
                                                           free($2); free($6);}

if_block                : IF '(' condition ')' block else_block { $$ = createIf($2, )}
else_block              : ELSE '{' block'}'  
while_block             : WHILE '(' condition ')' '{' all_statements '}' { $$ = createWhile($3, $6);}

condition               : term EQ term          { $$ = createRExpr($1, $3, "==");}
                        | term GREATER term     { $$ = createRExpr($1, $3, ">");}
                        | term LESS term        { $$ = createRExpr($1, $3, "<");}
                        | term GREATEROREQ term { $$ = createRExpr($1, $3, ">=");}
                        | term LESSOREQ term    { $$ = createRExpr($1, $3, "<=");}

block                   : '{' variableDeclarations all_statements   { vector<astNode*> *node_vect = new vector<astNode*> ();
                                                                      node_vect->insert(node_vect.end(), $2->begin(), $2->end());
                                                                      node_vect->insert(node_vect.end(), $3->begin(), $3->end());
                                                                      $$ = createBlock(node_vect);
                                                                      delete($2); delete($3);}
                        | '{' all_statements '}'                    { $$ = createBlock($2);}    


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
                        | '-' NUMBER {$$ = createUExpr($2, uminus);}

return                  : RETURN '(' expression ')' ';' { $$ = createRet($3);}
                        | RETURN expression ';'   { $$ = createRet($2);}
                        | RETURN ';'    { $$ = createRet(NULL);}

%%