#include "../inc/lex.h"
#include "../inc/node.h"
#include "../inc/token.h"
#include "../inc/parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define RESERVE_REG(reg) regfile[(reg)] = 1
#define FREE_REG(reg) regfile[(reg)] = 0

#define REG_COUNT 6
typedef uint8_t reg;
uint8_t regfile[REG_COUNT];

reg get_free_reg(void) {
	for(int i = 0; i < REG_COUNT; i++) {
		//printf("Register r%d = %d\n", i, regfile[i]);
		if(regfile[i] == 0) {
			regfile[i] = 1;
			return i;
		}
	}
	printf("No free registers available.\n");
	return 0;
}

void print_instruction(operation o) {
	switch(o) {
		case ADD:
			printf("add ");
			break;

		case SUB:
			printf("sub ");
			break;

		case ASTERISK:
			printf("mul ");
			break;

		case AMPER:
			printf("and ");
			break;

		case PIPE:
			printf("orr ");
			break;

		default:
			printf("error\n");
			break;
	}
}

reg gen_expression(node *e) {
	reg rd = 0;
	reg rr = 0;
	switch(e->type) {
		case INTEGER_CONSTANT_NODE:
			rd = get_free_reg();
			//printf("rd = r%d\n", rd);
			//RESERVE_REG(rd);
			printf("mov r%d, %s\n", rd, (char *)e->constant.tok->attr);
			return rd;

		case BINARY_EXPR_NODE:
			rd = gen_expression(e->expression.lval);
			rr = gen_expression(e->expression.rval);
			FREE_REG(rr);
			print_instruction(e->expression.o);
			printf("r%d, r%d, r%d\n", rd, rd, rr);
			return rd;
		
		default:
			return UINT8_MAX;
	}
}

int main(int argc, char **argv) {
	init_lex(argv[1]);
	if(has_error_occurred()) {
		return -1;
	}
	token *tok = lex_translation_unit();
	

	/*
	consume_token(); // int
	node *d = parse_declarator(NULL);
	print_decl(d);
	printf("int\n");
	*/
	node *e = parse_expr();
	reg r = gen_expression(e);
	
	printf("\nRegister state:\n");
	for(int i = 0; i < 4; i++) {
		printf("Register r%d = %s\n", i, (regfile[i] == 1 ? "RESERVED" : "FREE"));
	}

	return 0;
}


