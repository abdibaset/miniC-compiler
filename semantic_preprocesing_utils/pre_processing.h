#ifndef PRE_PROCESSING_H
#define PR_PROCESSING_H
#include "../ast/ast.h"
#include <set>
#include <vector>
#include <string>
using namespace std;

astNode *rename_variables_in_ast_tree(astNode *root);

vector<string> get_variable_names(astNode *root);
#endif // PRE_PROCESSING_H
