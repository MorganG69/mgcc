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


struct __symbol {
	unsigned int is_int : 1;		// |
	unsigned int is_char : 1;		// | - Specifies the type of the declaration
	unsigned int is_void : 1;		// |
	unsigned int is_ptr : 1;		// - Declaration is a pointer
	unsigned int is_function : 1;	// |
	unsigned int is_array : 1;		// |
	unsigned int is_struct : 1;		// | - Can only be one of these at a time
	unsigned int is_union : 1;		// |
	//unsigned int num_args;			// Used in structs, functions, arrays and unions
	unsigned int indirection_count; // Number of levels of pointer derefences. eg ***ptr == 3.
	unsigned int array_dimensions;
	node *array_expr_list;

	char *identifier;				// Identifier associated with this symbol
	node *initialiser;				// Parameters may be an initialiser expression
	symbol_table *params;			// Functions or structs have named internal declarations
	symbol *next;					// Symbol tables are just linked lists.
};

void init_symbol_table(void);
void enter_scope(void);
void exit_scope(void);
void add_symbol(node_type n, token_type t, char *id, node *params);
symbol *get_symbol(node_type n, token_type t, char *id);
void print_symbol_table(void);
symbol_table *get_global_table(void);
symbol *new_symbol(void);

#endif /* TABLE_H */


