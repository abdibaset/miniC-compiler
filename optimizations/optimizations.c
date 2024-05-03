#include "./utils/optimizations_utils.h"
#include "./utils/common_subexpr_dead_code_elimination.h"
#include "./utils/constant_folding.h"
#include "./utils/global_optimizations.h"
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
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

void doGlobalOptimizationWithConstantFolding(LLVMModuleRef module)
{
    bool hasModuleChanged = true;

    while (hasModuleChanged)
    {
        walkFunctionsForCommonSubExprAndDeadCode(module);

        // do constant folding
        walkFunctionsForConstantFolding(module);
        hasModuleChanged = walkFunctionsForGlobalOptimizations(module);
        walkFunctionsForConstantFolding(module);
    }
}

void doLocalOptimizations(LLVMModuleRef module)
{
    // start with common subexpression and dead code which is done in a loop until no further change
    walkFunctionsForCommonSubExprAndDeadCode(module);

    // do constant folding
    walkFunctionsForConstantFolding(module);
}

std::string getOptimizedFilename(const std::string &filename)
{
    std::filesystem::path filePath(filename);
    std::string basename = filePath.stem().string();

    basename += "_optimized.ll";

    printf("filename %s\n", basename.c_str());
    return basename;
}

string removeExtraSlashes(const std::string &path)
{
    std::string result;
    for (size_t i = 0; i < path.size(); ++i)
    {
        if (path[i] != '/' || (i > 0 && path[i - 1] != '/'))
        {
            result += path[i];
        }
    }
    return result;
}

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        fprintf(stderr, "Two few arguments...\n");
        exit(EXIT_FAILURE);
    }
    string cleanPath = removeExtraSlashes(argv[1]);
    LLVMModuleRef moduleReference = createLLVMModel(argv[1]);
    string opt_filename = getOptimizedFilename(argv[1]);

    if (moduleReference != NULL)
    {
        walkGlobalValues(moduleReference);

        if (strcmp(cleanPath.c_str(), "./opt_tests/p2_common_subexpr.ll") == 0 || strncmp(cleanPath.c_str(), "./opt_tests/cfold_", strlen("./opt_tests/cfold_")) == 0)
        {
            doLocalOptimizations(moduleReference); 
        }
        else
        {
            doGlobalOptimizationWithConstantFolding(moduleReference);
        }

        LLVMDumpModule(moduleReference);
        LLVMPrintModuleToFile(moduleReference, opt_filename.c_str(), NULL);
    }
    else
    {
        fprintf(stderr, "module reference is NULL\n");
    }

    exit(EXIT_SUCCESS);
}
