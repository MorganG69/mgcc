#include "../inc/node.h"
#include "../inc/table.h"
#include "../inc/lex.h"
#include "../inc/error.h"
#include "../inc/decl.h"
#include <stdlib.h>
#include <string.h>

#define NEW_TABLE calloc(1, sizeof(symbol_table))

int scope_count = 0;

symbol_table *global_scope;
symbol_table *current_scope;


void init_symbol_table(void) {
	global_scope = NEW_TABLE;
	global_scope->scope_num = scope_count;
	current_scope = global_scope;
	scope_count++;
}

void enter_scope(void) {
	symbol_table *prev_scope = current_scope;
	current_scope->next = NEW_TABLE;
	current_scope = current_scope->next;
//	printf("entering scope %d\n", scope_count);
	current_scope->prev = prev_scope;
	current_scope->scope_num = scope_count;
	scope_count++;
}

void exit_scope(void) {
//	printf("exiting scope %d\n", current_scope->scope_num);
	if(current_scope != global_scope) {
		symbol *ptr = current_scope->head;

		while(ptr != NULL) {
			symbol *tmp = ptr->next;
			free(ptr);
			ptr = tmp;
		}

		symbol_table *next_scope = current_scope->prev;
		free(current_scope);
		current_scope = next_scope;
		scope_count--;
	} else {
		error("cannot exit the global scope");
	}
}

void add_symbol(token_type t, char *id, node *params) {
	symbol *s = calloc(1, sizeof(symbol));
	s->type = t;
	s->ident = id;
	s->params = params;

	if(current_scope->sym_count == 0) {
		current_scope->head = s;
		current_scope->tail = s;
	} else {
		current_scope->tail->next = s;
		current_scope->tail = s;
	}
	current_scope->sym_count++;
}

/* Searches for the symbol from the current scope upwards */
symbol *get_symbol(token_type t, char *id) {
	symbol_table *scope = current_scope;
	symbol *ptr = NULL;
	while(scope->prev != NULL) {
		symbol *ptr = scope->head;
		while(ptr != NULL) {
			if(!strcmp(id, ptr->ident) && ptr->type == t) {
				return ptr;
			} else {
				ptr = ptr->next;
			}
		}
		scope = scope->prev;
	}
	return ptr;
}

symbol_table *get_global_table(void) {
	return global_scope;
}

void print_indent(void) {
	for(int i = 0; i < scope_count; i++) {
		printf(" ");
	}
}

void print_symbol_table(void) {
	symbol_table *t = current_scope;
	printf("Scope:	%d\n", t->scope_num);
	symbol *ptr = t->head;
	
	while(ptr != NULL) {
		printf("Type:	");
		print_type_specifier(ptr->type);
		printf("ID:	%s\n", ptr->ident);
		ptr = ptr->next;
	}
}


