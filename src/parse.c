#include "../inc/lex.h"
#include "../inc/node.h"
#include "../inc/token.h"
#include "../inc/parse.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * primary-expression
 * 	identifier
 * 	constant
 * 	string
 * 	( expression )
 */ 
node *primary_expr(void) {
	node *n;
	switch(get_current_token()->type) {
		case INTEGER_CONST:
			n = new_node(INTEGER_CONSTANT_NODE);
			n->constant.tok = get_current_token();
			consume_token();
			break;
			
		case CHAR_CONST:
			n = new_node(CHAR_CONSTANT_NODE);
			n->constant.tok = get_current_token();		
			consume_token();
			break;

		case IDENTIFIER:
			n = new_node(IDENTIFIER_NODE);
			n->identifier.tok = get_current_token();
			consume_token();
			/* 
			 * Symbol table stuff.
			 * Add the symbol table entry to the node
			 */
			break;
		
		case LPAREN:
			consume_token();
			n = parse_expr();
			if(get_current_token()->type != RPAREN) {
				error("Mismatched parenthesis within expression.");
			} else {
				consume_token();
			}
			break;

		default:
			error("Invalid token within expression.");
			n->type = ERROR_NODE;
			break;

	}
	return n;
}

/*
 * postfix-expression:
 * 	primary-expression
 * 	postfix-expression [ expression ]
 * 	postfix-expression [ argument-expression-list ]
 * 	postfix-expression . identifier
 * 	postfix-expression -> identifier
 * 	postfix-expression ++
 * 	postfix-expression --
 */
node *postfix_expr(node *prev) {
	if(prev == NULL) {
		prev = primary_expr();
	}
	node *n;
	switch(get_current_token()->type) {
		case INCREMENT:
		case DECREMENT:
			n = new_node(POSTFIX_EXPR_NODE);
			n->postfix.o = get_current_token()->type;
			consume_token();
			/* 
			 * Grammar states that the lval is a postfix expression and the previous value will always be a postfix expression. 
			 * A primary expression is also a postfix expression 
			 */
			n->postfix.lval = prev;
			return postfix_expr(n);
		
		/* If the current token is not valid postfix just return the previous node */
		default:
			return prev;
	}
}

/*
 * unary-operator: one of
 *	& * + - ~ !
 *
 * unary-expression:
 * 	postfix-expression
 * 	++ unary-expression
 * 	-- unary-expression
 * 	unary-operator cast-expression
 * 	sizeof unary-expression
 * 	sizeof ( type-name )
 */ 
node *unary_expr(void) {
	node *n;
	switch(get_current_token()->type) {
		case INCREMENT:
		case DECREMENT:
		case AMPER:
		case ASTERISK:
		case ADD:
		case SUB:
		case TILDE:
		case NOT:
			n = new_node(UNARY_EXPR_NODE);
			n->unary.o = get_current_token()->type;
			consume_token();
			n->unary.rval = unary_expr();
			break;

		default:
			n = postfix_expr(NULL);
			break;
	}
	return n;
}

/*
 * multiplicative-expression:
 * 	cast-expression
 * 	multiplicative-expression * cast-expression
 * 	multiplicative-expression / cast-expression
 * 	multiplicative-expression % cast-expression
 */ 
node *multiplicative_expr(void) {
	node *lval = unary_expr();
	if(get_current_token()->type == DIVIDE || get_current_token()->type == ASTERISK) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = lval;
		e->expression.rval = multiplicative_expr();
		return e;
	} else {
		return lval;
	}
}

/*
 * additive-expression:
 * 	multiplicative-expression
 * 	additive-expression + multiplicative-expression
 * 	additive-expression - multiplicative-expression
 */
node *additive_expr(void) {
	node *lval = multiplicative_expr();
	if (get_current_token()->type == ADD || get_current_token()->type == SUB) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = lval;
		e->expression.rval = additive_expr();
		return e;
	} else {
		return lval;
	}
}

/*
 * shift-expression:
 * 	additive-expression
 * 	shift-expression << additive-expression
 * 	shift-expression >> additive-expression
 */
node *shift_expr(void) {
	node *lval = additive_expr();
	if (get_current_token()->type == LSHIFT || get_current_token()->type == RSHIFT) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = lval;
		e->expression.rval = shift_expr();
		return e;
	} else {
		return lval;
	}
}

/*
 * relational-expression
 * 	shift-expression
 * 	relational-expression < shift-expression
 * 	relational-expression > shift-expression
 * 	relational-expression <= shift-expression
 * 	relational-expression >= shift-expression
 */
node *relational_expr(void) {
	node *lval = shift_expr();
	if (get_current_token()->type == GREATER || get_current_token()->type == GTEQ
		|| get_current_token()->type == LESS || get_current_token()->type == LTEQ) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = lval;
		e->expression.rval = relational_expr();
		return e;
	} else {
		return lval;
	}
}

/*
 * equality-expression:
 * 	relational-expression
 * 	equality-expression == relational-expression
 * 	equality-expression != relational-expression
 */
node *equality_expr(void) {
	node *lval = relational_expr();
	if (get_current_token()->type == EQUAL || get_current_token()->type == NOTEQ) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = lval;
		e->expression.rval = equality_expr();
		return e;
	} else {
		return lval;
	}
}

/*
 * AND-expression:
 * 	equality-expression
 * 	AND-expression & equality-expression
 */
node *and_expr(void) {
	node *lval = equality_expr();
	if (get_current_token()->type == AMPER) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = lval;
		e->expression.rval = and_expr();
		return e;
	} else {
		return lval;
	}
}

/*
 * exclusive-OR-expression:
 * 	AND-expression
 * 	exclusive-OR-expression ^ AND-expression
 */
node *xor_expr(void) {
	node *lval = and_expr();
	if (get_current_token()->type == CARET) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = lval;
		e->expression.rval = xor_expr();
		return e;
	} else {
		return lval;
	}
}

/*
 * inclusive-OR-expression:
 * 	exclusive-OR-expression
 * 	inclusive-OR-expression | exclusive-OR-expression
 */
node *or_expr(void) {
	node *lval = xor_expr();
	if (get_current_token()->type == PIPE) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = lval;
		e->expression.rval = or_expr();
		return e;
	} else {
		return lval;
	}
}

/*
 * logical-AND-expression:
 * 	inclusive-OR-expression
 * 	logical-AND-expression && inclusive-OR-expression
 */
node *logand_expr(void) {
	node *lval = or_expr();
	if (get_current_token()->type == LOGAND) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = lval;
		e->expression.rval = logand_expr();
		return e;
	} else {
		return lval;
	}
}

/*
 * logical-OR-expression:
 * 	logical-AND-expression
 * 	logical-OR-expression || logical-AND-expression
 */
node *logor_expr(void) {
	node *lval = logand_expr();
	if (get_current_token()->type == LOGOR) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = lval;
		e->expression.rval = logor_expr();
		return e;
	} else {
		return lval;
	}
}

/*
 * conditional-expression:
 * 	logical-OR-expression
 * 	logical-OR-expression ? expression : conditional-expression
 */
node *conditional_expr(void) {
	/* Ternary operation not yet implemented. */
	return logor_expr();
}

/*
 * assignment-expression:
 * 	conditional-expression
 * 	unary-expression assignment-operator assignment-expression
 */
node *assignment_expr(void) {
	node *lval;

	if(get_current_token()->type == ASTERISK) {
		lval = unary_expr();
	} else {
		switch(peek_next_token()->type) {
			case DOT:
			case LBRACK:
			/* case ARROW: */
				lval = unary_expr();
				break;
			
			default:
				lval = conditional_expr();
				break;
		}
	}

	if(get_current_token()->type == ASSIGN) {
		node *e = new_node(ASSIGNMENT_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = lval;
		e->expression.rval = assignment_expr();
		return e;
	} else {
		return lval;
	}
}

node *parse_expr(void) {
	node *expr = assignment_expr();
	return expr;
}

node *parse_stmt(void) {
	 return NULL;
}

#define COUNT 10
void print_tree(node *root, int depth) {
	if(root->type == INTEGER_CONSTANT_NODE) {
		if(depth > 4){
			for(int i = 1; i < depth; i++) {
				if(i % 4 == 0) {
					printf("|");
				} else {
					printf(" ");
				}	
			}
		}
		printf("`- ");	
		printf("%s\n", (char *)root->constant.tok->attr);
		return;
	} else {
		for(int i = 1; i < depth; i++) {
			if(i % 4 == 0) {
				printf("|");
			} else {
				printf(" ");
			}
		}

		if(depth > 0) {
			printf("`- ");
		}
		depth+=4;
		print_token_type(root->expression.o);
		printf("|");
		
		print_tree(root->expression.rval, depth);
		if(depth > 4){
			printf("|");
		}
		print_tree(root->expression.lval, depth);

		return;	
	}

}

void print_stack(token_stack *test) {
	if(test != NULL) {
		while(token_stack_empty(test) == false) {
			token *t = pop_token(test);
			
			print_token_type(t->type);
		}
	} else {
		error("Error while parsing expression.");
	}
}

void print_node_type(node_type type) {
	switch(type) {
		case INTEGER_CONSTANT_NODE:
			printf("INTEGER_CONSTANT_NODE\n");
			break;

		case ASSIGNMENT_EXPR_NODE:
			printf("ASSIGNMENT_EXPR_NODE\n");
			break;

		case UNARY_EXPR_NODE:
			printf("UNARY_EXPR_NODE\n");
			break;
	
		case BINARY_EXPR_NODE:
			printf("BINARY_EXPR_NODE\n");
			break;
		
		case POSTFIX_EXPR_NODE:
			printf("POSTFIX_EXPR_NODE\n");
			break;

		default:
			printf("Unimplemented node type: %d\n", type);
			break;
	}
}

int get_precedence(operation op); 
/*
	Implements the shunting yard algorithm to convert stream of tokens from
	the lexer into a postfix ordered stack.
*/
token_stack *infix_to_postfix(void) {
	/* Intialise the two stacks */
	token_stack *output_stack = token_stack_init(MAX_EXPRESSION_SIZE);		
	token_stack *operator_stack = token_stack_init(MAX_EXPRESSION_SIZE);
	token *current_token;

	while(get_current_token()->type != END) {
		current_token = get_current_token();
		//print_token_type(current_token->type);	
		switch(current_token->type) {
			case END:
			case NEWLINE:
				goto done;

			/* Only handle integer constants for now */
			case INTEGER_CONST:
				push_token(output_stack, current_token);
				break;

			case LPAREN:
				push_token(operator_stack, current_token);
				break;

			case RPAREN:
				/* Assert the stack is not empty */
				if(token_stack_empty(operator_stack) == false) {
					while(peek_stack(operator_stack)->type != LPAREN) {
						push_token(output_stack, pop_token(operator_stack));
						if(token_stack_empty(operator_stack) == true) {
							goto parens;
						}
					}
					
					if(peek_stack(operator_stack)->type == LPAREN) {
						(void)pop_token(operator_stack); /* Discard the lparen */
						
						/* While the operator stack is not empty */
						while(token_stack_empty(operator_stack) == false) {
							if(peek_stack(operator_stack)->type == LPAREN){
								goto parens; /* If a lparen is on the stack then there are mismatched parens. */
							} else {
								/* Push the operators to the output */
								push_token(output_stack, pop_token(operator_stack));
							}
						}
					
						while(token_stack_empty(operator_stack) == false) {
							if(peek_stack(operator_stack)->type == LPAREN) {
								goto parens;
							}
							push_token(output_stack, pop_token(operator_stack));
						}
					
					} else {
						goto parens;
					}	
				}
				break;
			
			case ASTERISK: case DIVIDE:
			case ADD: case SUB:
			case LSHIFT: case RSHIFT:
			case GREATER: case GTEQ: case LESS: case LTEQ:
			case EQUAL: case NOTEQ:
			case AMPER:
			case CARET:
			case PIPE:
			case LOGAND:
			case LOGOR:
				if(!token_stack_empty(operator_stack) &&
						get_precedence(peek_stack(operator_stack)->type) >= get_precedence(current_token->type)) {
					/* Note that associativity is important here. Look into this when implementing pre/postfix etc
					 * For now if the precedence is the same then pop from the operator stack.
					 */
					while(!token_stack_empty(operator_stack) &&
							peek_stack(operator_stack)->type != LPAREN &&
							get_precedence(peek_stack(operator_stack)->type) >= get_precedence(current_token->type)) {
						push_token(output_stack, pop_token(operator_stack));
					}
					push_token(operator_stack, current_token);
				} else {
					push_token(operator_stack, current_token);
				}
				break;
			}
		consume_token();
	}
	done:
		while(token_stack_empty(operator_stack) == false) {
			if(peek_stack(operator_stack)->type == LPAREN) {
				goto parens;
			}
			push_token(output_stack, pop_token(operator_stack));
		}
		free(operator_stack);
		return output_stack;

	parens:
		error("Mismatched parenthesis in expression.");
		free(output_stack);
		free(operator_stack);
		return NULL; /* Temporary */
}

int get_precedence(operation op) {
  switch(op) {
    case INCREMENT:
    case DECREMENT:
      return 13;

    case ASTERISK:
    case DIVIDE:
      return 12;

    case ADD:
    case SUB:
      return 10;

    case LSHIFT:
    case RSHIFT:
      return 9;

    case GREATER:
    case GTEQ:
    case LESS:
    case LTEQ:
      return 8;

    case EQUAL:
    case NOTEQ:
      return 7;

    case AMPER:
      return 6;

    case CARET:
      return 5;

    case PIPE:
      return 4;

    case LOGAND:
      return 3;

    case LOGOR:
      return 2;

    case ASSIGN:
      return 1;

    case COMMA:
      return 0;

    default:
      return 0;
  }
}
