#ifndef DECL_H
#define DECL_H

#include "../inc/node.h"
#include "../inc/lex.h"
#include <stdbool.h>

node *parse_declaration(void);
node *parse_abstract_declaration(void);
void print_type_specifier(token_type s);
bool is_declaration(token_type t);
node *parse_translation_unit(void);
token_type get_decl_type(node *d);
node *parse_decl_initializers(void); 
#endif /* DECL_H */
