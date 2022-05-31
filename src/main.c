#include "../inc/lex.h"
#include "../inc/node.h"
#include "../inc/token.h"
#include "../inc/parse.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv) {
	init_lex(argv[1]);
	if(has_error_occurred()) {
		return -1;
	}
	token *tok = lex_translation_unit();

	node *x = parse_expr();
	node *lval = x->postfix.lval;

	print_token_type(x->postfix.o);
	print_node_type(lval->postfix.lval->type);

//	print_tree(x, 0);
	

	/*
	print_node_type(x->type);
	printf("lval = ");
	print_node_type(x->expression.lval->type);
	printf("rval = ");
	print_node_type(x->expression.rval->type);
*/
	return 0;
}


