#include <stdio.h>
#include <stdlib.h>
#include "../utils/ast.h"
#include "y.tab.h"

extern FILE *yyin;
extern int yylex();
extern int yylex_destroy();

int main(int argc, char** argv){
    if (argc != 2){
        printf("Too few arguments");
        exit(EXIT_FAILURE);
    }

    if (yyin == NULL){
        fprintf(stderr, "Error reading file\n");
        exit(EXIT_FAILURE);
    }

    yyin = fopen(argv[1], "r");
    yyparse();
    yylex_destroy();
    fclose(yyin);

    exit(EXIT_SUCCESS);
}