#ifndef STMT_H
#define STMT_H

#include "node.h"

void print_node_type(node_type type);
node *parse_expr(void);
node *parse_declarator(node *prev);
node *parse_declaration(void);
node *parse_statement(void);
void print_decl(node *d);
void print_tree(node *tree, int n_indent, int n_lvals);
char *get_decl_identifier(node *d); 
void print_statement(node *s, int indent);
node *parse_abstract_declaration(void);
node *parse_compound_statement(void);

#endif /* STMT_H */
