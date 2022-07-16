#ifndef PARSE_H
#define PARSE_H

#include "node.h"

void print_node_type(node_type type);
node *parse_expr(void);
node *parse_declarator(node *prev);
void print_decl(node *d);
void print_tree(node *tree, int n_indent, int n_lvals);
#endif /* PARSE_H */
