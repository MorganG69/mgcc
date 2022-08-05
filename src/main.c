#include "../inc/lex.h"
#include "../inc/node.h"
#include "../inc/token.h"
#include "../inc/parse.h"
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
	

	
	node *d = parse_declaration();
	print_decl(d);
	printf("%s\n", get_decl_identifier(d));
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


