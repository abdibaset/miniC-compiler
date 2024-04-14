/*parser lex file to identify tokens in a c program */
%{
#include<stdio.h>
#include "../utils/ast.h"
#include "y.tab.h"
extern int yyerror(char *);
%}

%%
if                          { return IF; }
else                        { return ELSE;}
while                       { return WHILE; }
int                         { return INT; }
char                        { return CHAR; }
return                      { return RETURN;}
void                        { return VOID;}
extern                      { return EXTERN; }
print                       { return PRINT;}
read                        { return READ;}


[a-zA-Z][a-zA-Z0-9]+        { yylval.varname = (char *) calloc(strlen(yytext)+1, sizeof(char));
                              strcpy(yylval.varname, yytext);
                              return VARID;}
[0-9]+                      { yylval.numval = atoi(yytext);
                              return NUMBER;}

:                           { return yytext[0];} 
[ \n\t]
.                           { return yytext[0];}
%%

int yywrap(){
    return 1;
}