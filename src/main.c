#include "../inc/lex.h"
#include "../inc/node.h"
#include "../inc/token.h"
#include "../inc/parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main(int argc, char **argv) {
	init_lex(argv[1]);
	if(has_error_occurred()) {
		return -1;
	}
	token *tok = lex_translation_unit();

	consume_token(); /* int */
	node *d = parse_declarator(NULL);
	print_decl(d);
	printf("int\n");

	return 0;
}


