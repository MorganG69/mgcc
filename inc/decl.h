#ifndef DECL_H
#define DECL_H

#include "../inc/node.h"
#include "../inc/lex.h"
#include <stdbool.h>

node *parse_declaration(void);
void print_type_specifier(token_type s);
bool is_declaration(token_type t);

#endif /* DECL_H */
