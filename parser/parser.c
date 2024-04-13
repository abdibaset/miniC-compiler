#include <stdio.h>
#include <stdlib.h>
#include <ast.h>

int main(int argc, char** argv){
    if (argc != 2){
        printf("Too few arguments");
        exit(EXIT_FAILURE);
    }

    FILE *yyin = fopen(argv[1], "r");
    if (yyin == NULL){
        fprintf(stderr, "Error reading file\n");
    }

    yyparse();

    astNode *root = createProgram(yyin);
    yylex_destroy();

    fclose(yyin);

    exit(EXIT_SUCCESS);
}