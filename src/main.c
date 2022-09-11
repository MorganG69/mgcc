#include "../inc/lex.h"
#include "../inc/node.h"
#include "../inc/token.h"
#include "../inc/stmt.h"
#include "../inc/decl.h"
#include "../inc/expr.h"
#include "../inc/table.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


int main(int argc, char **argv) {
	init_lex(argv[1]);
	if(has_error_occurred()) {
		return -1;
	}
	init_symbol_table();

	token *tok = lex_translation_unit();
	node *s = parse_translation_unit();
	
	if(s == NULL) {
		error("empty source file");
	} else {
		print_statement_list(s, 0);
	}
	return 0;
}


