#include "./utils/ast.h"
#include "./utils/semantic_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include "y.tab.h"
#include <cassert>
#include <cstring>
#include <vector>

extern FILE *yyin;
extern int yylex();
extern int yylex_destroy();
extern astNode *root;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Too few arguments");
        exit(EXIT_FAILURE);
    }

    // commented out for later reference
    // #ifdef YYDEBUG
    //     yydebug = 1;
    // #endif

    yyin = fopen(argv[1], "r");
    if (yyin == NULL)
    {
        fprintf(stderr, "Error reading file\n");
        exit(EXIT_FAILURE);
    }

    yyparse();
    astNode *node = root;

    if (!is_semantically_correct(node))
    {
        fprintf(stderr, "Semantic analysis failed\n");
        freeNode(root);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Semantic analysis success!\n");
        freeNode(root);
    }
    fclose(yyin);
    yylex_destroy();
    exit(EXIT_SUCCESS);
}