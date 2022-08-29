/* TODO: tidy these up */
#include "../inc/lex.h"
#include "../inc/node.h"
#include "../inc/token.h"
#include "../inc/stmt.h"
#include "../inc/expr.h"
#include "../inc/table.h"
#include "../inc/triple.h"
#include <stdio.h>
#include <stdlib.h>

triple *new_triple(void) {
	return calloc(1, sizeof(triple));
}

t_list *new_t_list(void) {
	return calloc(1, sizeof(t_list));
}

void add_triple(t_list *tl, triple *t) {
	if(tl->count == 0) {
		tl->head = t;
		tl->tail = t;
	} else {
		tl->tail->next = t;
		tl->tail = t;
	}
	t->id = tl->count;
	tl->count++;
}

/*
triple *gen_triple_expr(node *expr) {
	switch(node->type) {
		case _NODE:
				
	}
}
*/
