#include "./ast/ast.h"
#include "./parser/y.tab.h"
#include "./optimizations/optimizations_utils.h"
#include "./optimizations/optimizations.h"
#include "./semantic_preprocesing_utils/semantic_analysis.h"
#include "./semantic_preprocesing_utils/pre_processing.h"
#include "./IRBuilder/ir_builder.h"
#include "./CodeGen/register_allocation.h"

#include <stdio.h>
#include <stdlib.h>
#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Types.h>
#include <stdbool.h>
#include <unordered_map>
#include <map>
#include <set>
#include <algorithm>
#include <vector>
#include <string>
#include <experimental/filesystem>
#include <cassert>

extern FILE *yyin;
extern int yylex();
extern int yylex_destroy();
extern astNode *root;

using namespace std;
namespace fs = std::experimental::filesystem;

void doGlobalOptimizationWithConstantFolding(LLVMModuleRef module)
{
    bool hasModuleChanged = true;

    while (hasModuleChanged)
    {
        walkFunctionsForCommonSubExprAndDeadCodeElimination(module);

        // do constant folding
        walkFunctionsForConstantFoldingAndDeadCodeElimination(module);
        hasModuleChanged = walkFunctionsForGlobalOptimizations(module);
        walkFunctionsForConstantFoldingAndDeadCodeElimination(module);
    }
}

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        fprintf(stderr, "Two few arguments...\n");
        exit(EXIT_FAILURE);
    }

    // #ifdef YYDEBUG
    //         yydebug = 1;
    //     #endif

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
        fprintf(stderr, "semantic analysis failed!\n");
        exit(EXIT_FAILURE);
    }

    astNode *modifiedTree = rename_variables_in_ast_tree(node);
    LLVMModuleRef moduleReference = build_ir(modifiedTree, argv[1]);

    // LLVMDumpModule(moduleReference);
    // exit(1);
    // printf("Before optimization on module\n");
    assert(moduleReference != NULL);

    // optimizations
    doGlobalOptimizationWithConstantFolding(moduleReference);
    printf("optimizated module\n");

    LLVMDumpModule(moduleReference);
    printf("FILENAME %s\n", argv[1]);
    generate_assembly_code(argv[1], moduleReference);
    // clean and free memory
    fclose(yyin);
    yylex_destroy();
    freeNode(root);
    LLVMDisposeModule(moduleReference);
    LLVMShutdown();

    exit(EXIT_SUCCESS);
}