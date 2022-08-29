#ifndef TABLE_H
#define TABLE_H

#include "node.h"
#include "lex.h"

typedef struct _symbol symbol;
typedef struct _symbol_table symbol_table;
struct _symbol_table {
	int scope_num;
	
	size_t sym_count;
	symbol *head;
	symbol *tail;

	symbol_table *prev;
	symbol_table *next;
};

struct _symbol {
	node_type n_type;
	token_type type;
	int scope;
	char *ident;
	node *params;
	symbol *next;
};

void init_symbol_table(void);
void enter_scope(void);
void exit_scope(void);
void add_symbol(node_type n, token_type t, char *id, node *params);
symbol *get_symbol(node_type n, token_type t, char *id);
void print_symbol_table(void);
symbol_table *get_global_table(void);

#endif /* TABLE_H */


