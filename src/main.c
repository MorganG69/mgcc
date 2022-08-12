#include "../inc/lex.h"
#include "../inc/node.h"
#include "../inc/token.h"
#include "../inc/stmt.h"
#include "../inc/decl.h"
#include "../inc/expr.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct symbol_table {
	node *next_scope;
	node *head;
	node *tail;
	size_t symbol_count;
};

int main(int argc, char **argv) {
	init_lex(argv[1]);
	if(has_error_occurred()) {
		return -1;
	}
	token *tok = lex_translation_unit();
	
//	node *s = parse_statement();
	node *s = parse_declaration();
	print_statement(s, 0);
	//print_node_type(s->type);

	/*
	if(s->statement.expr == NULL) {
		printf("No expression or declaration.\n");
	} else {
		print_node_type(s->statement.expr->type);
	}

	if(s->statement.stmt == NULL) {
		printf("No statement.\n");
	} else {
		print_node_type(s->statement.stmt->type);
	}
	*/
//	node *d = parse_declaration();
//	print_decl(d);
//	printf("%s\n", get_decl_identifier(d));
	/*
	node *e = parse_expr();
	reg r = gen_expression(e);
	
	printf("\nRegister state:\n");
	for(int i = 0; i < 4; i++) {
		printf("Register r%d = %s\n", i, (regfile[i] == 1 ? "RESERVED" : "FREE"));
	}
	*/

//	node *e = parse_expr();


	return 0;
}


