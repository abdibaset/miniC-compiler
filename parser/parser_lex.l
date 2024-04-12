%{
#include<stdio.h>
#include<ast.h>
#include "y.tab.h"
extern int yyerror(char *);
%}

%%
if                          { return IF; }
else                        { return ELSE;}
while                       { return WHILE; }
for                         { return FOR; }
int                         { return INT; }
char                        { return CHAR; }
return                      { return RETURN;}
void                        { return VOID;}
extern                      { return EXTERN; }
print                       { return PRINT;}
read                        { return READ;}

"=="                        { return EQ;}
">"                         { return GREATER;}
"<"                         { return LESS;}
">="                        { return GREATEROREQ;}
"<="                        { return LESSOREQ;}
"!="                        { return NOTEQ;}


[a-zA-Z][a-zA-Z0-9]+        { yylval.sname = calloc(strlen(yytext)+1, sizeof(char));
                              strcpy(yylval.sname, yytext);)
                              return VARID;}
[0-9]+                      { yylval.int = atoi(yytext);
                              return NUMBER;}

:                           { return yytext[0]; } 
[ \n\t]
.                           { return yytext[0];}
%%

int yywrap(){
    return 1;
}