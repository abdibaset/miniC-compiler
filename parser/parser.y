/**
 * @author Abdibaset Bare
 * @create date 2024-04-12 17:22:44
 * @modify date 2024-04-18 16:50:48
 * @desc - this file defines the grammar rules for a given c program
 */
%{
#include <stdio.h>
#include <iostream>
#include "./utils/ast.h"
#include <vector>
using namespace std;
extern int yylex();
extern int yylex_destroy();
int yyerror(const char*);
astNode* root;
%}

%union {
    astNode *node;
    char *varname;
    int numval;
    vector <astNode*> *astvec_ptr;
}

%token IF ELSE WHILE INT RETURN VOID EXTERN PRINT READ
%token <varname> VARID
%token <numval> NUMBER
%type <astvec_ptr> statements declarations
%type<node> externFunc term declaration  statement expression ifBlock whileBlock return condition function program block elseBlock
%start program
%nonassoc REDUCE
%nonassoc ELSE

%%
program                 : externFunc externFunc function    { $$ = createProg($1, $2, $3);
                                                                root = $$; printNode(root);}
externFunc              : EXTERN INT READ '(' ')' ';'       {$$ = createExtern("read");}
                        | EXTERN VOID PRINT '(' INT ')' ';' {$$ = createExtern("print");}

function                : INT VARID '(' INT term ')' block   { $$ = createFunc($2, $5, $7); free($2);}
                        | INT VARID '(' ')' block            { $$ = createFunc($2, NULL, $5); free($2);}


condition               : term '=' '=' term    { $$ = createRExpr($1, $4, eq);}
                        | term '>' term        { $$ = createRExpr($1, $3, gt);}
                        | term '<' term        { $$ = createRExpr($1, $3, lt);}
                        | term '>' '=' term    { $$ = createRExpr($1, $4, ge);}
                        | term '<' '=' term    { $$ = createRExpr($1, $4, le);}
                        | term '!' '=' term    { $$ = createRExpr($1, $4, le);}


block                   : '{' declarations statements '}' { vector<astNode*> *node_vect = new vector<astNode*> ();
                                                            node_vect->insert(node_vect->end(), $2->begin(), $2->end());
                                                            node_vect->insert(node_vect->end(), $3->begin(), $3->end());
                                                            $$ = createBlock(node_vect);
                                                            delete($2); delete($3);}
                        | '{' statements '}'              { $$ = createBlock($2);}


ifBlock                 : IF '(' condition ')' statement elseBlock     { $$ = createIf($3, $5, $6);}
                        | IF '(' condition ')' statement           { $$ = createIf($3, $5, NULL);} %prec REDUCE

elseBlock               : ELSE statement        { $$ = $2;}

whileBlock              : WHILE '(' condition ')'  statement            { $$ = createWhile($3, $5);}
statements              : statements statement      { $$ = $1;
                                                      $$->push_back($2);}
                        | statement                 { $$ = new vector<astNode*>();
                                                      $$->push_back($1);}

statement               : VARID '=' expression ';'     { astNode* stmt_ptr = createVar($1);
                                                      $$ = createAsgn(stmt_ptr, $3); free($1);}
                        | VARID '=' READ '(' ')' ';'    { $$ = createCall("read"); free($1);}
                        | PRINT '(' term ')' ';'       { $$ = createCall("print", $3);}
                        | ifBlock                   { $$ = $1;}
                        | whileBlock                { $$ = $1;}
                        | return                    { $$ = $1;}
                        | block                     { $$ = $1;}

declarations            : declarations declaration      { $$ = $1;
                                                          $$->push_back($2);}

                        | declaration                   { $$ = new vector<astNode*>();
                                                          $$->push_back($1);}

declaration             : INT VARID ';'     { $$ = createDecl($2); free($2);}


expression              : term '+' term     { $$ = createBExpr($1, $3, add);}
                        | term '-' term     { $$ = createBExpr($1, $3, sub);}
                        | term '*' term     { $$ = createBExpr($1, $3, mul);}
                        | term '/' term     { $$ = createBExpr($1, $3, divide);}
                        | term              { $$ = $1;}
            

term                    : NUMBER    { $$ = createCnst($1);}
                        | VARID     { $$ = createVar($1); free($1);}
                        | '-' term {$$ = createUExpr($2, uminus);}

return                  : RETURN '(' expression ')' ';' { $$ = createRet($3);}
                        | RETURN expression ';'   { $$ = createRet($2);}
                        | RETURN ';'    { $$ = createRet(NULL);}
%%

int yyerror(const char *message){
    fprintf(stderr, "%s\n", message);
    return 1;
}