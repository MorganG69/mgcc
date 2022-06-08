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
	
	printf("expression type: ");
	print_node_type(x->type);
	printf("operator: ");
	print_token_type(x->expression.o);
	printf("lval: ");
	print_node_type(x->expression.lval->type);
	printf("rval: ");
	print_node_type(x->expression.rval->type);
	return 0;
}


