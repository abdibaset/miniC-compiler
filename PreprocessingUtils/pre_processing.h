#ifndef PRE_PROCESSING_H
#define PR_PROCESSING_H
#include <set>
#include <string>
#include <vector>

#include "../ast/ast.h"
using namespace std;

astNode *rename_variables_in_ast_tree(astNode *root);

vector<string> get_variable_names(astNode *root);
#endif  // PRE_PROCESSING_H
