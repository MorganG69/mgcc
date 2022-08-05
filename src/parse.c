#include "../inc/lex.h"
#include "../inc/node.h"
#include "../inc/token.h"
#include "../inc/parse.h"
#include <stdio.h>
#include <stdlib.h>

node *assignment_expr(node *prev);

void parser_panic(void) {
	while(get_current_token()->type != SEMI_COLON) {
		consume_token();
	}
}

/*
 * primary-expression
 * 	identifier
 * 	constant
 * 	string
 * 	( expression )
 */ 
node *primary_expr(void) {
	node *n = NULL;
	//print_token_type(get_current_token()->type);
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
			break;

	}
	return n;
}


node *parse_argument_expr_list(node *prev) {
	if(prev == NULL) {
		prev = assignment_expr(NULL);
	}

	if(get_current_token()->type == COMMA) {
		consume_token();
		node *e = assignment_expr(NULL);
		prev->next = e;
		return parse_argument_expr_list(e);
	} else {
		return prev;
	}
}

/*
 * postfix-expression:
 * 	primary-expression
 * 	postfix-expression [ expression ] - Array Access
 * 	postfix-expression ( argument-expression-list ) - Function call
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
			break;

		/* Array access */
		case LBRACK:
			consume_token();
			n = new_node(ARRAY_ACCESS_NODE);
			n->postfix.lval = prev;

			if(get_current_token()->type == RBRACK) {
				consume_token();
				error("Expected expression before ']' token.");
			} else {
				n->postfix.params = parse_expr();
				if(get_current_token()->type != RBRACK) {
					error("Expected ']'");
					return n;
				} else {
					consume_token();
				}
			}
			break;

		/* Function call */
		case LPAREN:
			consume_token();
			n = new_node(FUNCTION_CALL_NODE);
			n->postfix.lval = prev;

			if(get_current_token()->type == RPAREN) {
				consume_token();
			} else {
				n->postfix.params = parse_argument_expr_list(NULL);
				if(get_current_token()->type != RPAREN) {
					error("Expected ')'");
					return n;
				} else {
					consume_token();
				}
			}
			break;

		/* If the current token is not valid postfix just return the previous node */
		case DOT:
		case ARROW:
			error("Structs and Unions not yet implemented.");
		default:
			return prev;
	}

	return postfix_expr(n);
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
node *multiplicative_expr(node *prev) {
	if(prev == NULL) {
		prev = unary_expr();
	}

	if(get_current_token()->type == DIVIDE || get_current_token()->type == ASTERISK) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = prev;
		e->expression.rval = unary_expr();
		return multiplicative_expr(e);
	} else {
		return prev;
	}
}

/*
 * additive-expression:
 * 	multiplicative-expression
 * 	additive-expression + multiplicative-expression
 * 	additive-expression - multiplicative-expression
 */
node *additive_expr(node *prev) {
	if(prev == NULL) {
		prev = multiplicative_expr(NULL);
	}

	if (get_current_token()->type == ADD || get_current_token()->type == SUB) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = prev; 
		e->expression.rval = multiplicative_expr(NULL);
		return additive_expr(e); 
	} else {
		return prev;
	}
}

/*
 * shift-expression:
 * 	additive-expression
 * 	shift-expression << additive-expression
 * 	shift-expression >> additive-expression
 */
node *shift_expr(node *prev) {
	if(prev == NULL) {
		prev = additive_expr(NULL);
	}

	if (get_current_token()->type == LSHIFT || get_current_token()->type == RSHIFT) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = prev;
		e->expression.rval = additive_expr(NULL);
		return shift_expr(e);
	} else {
		return prev;
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
node *relational_expr(node *prev) {
	if(prev == NULL) {
		prev = shift_expr(NULL);
	}

	if (get_current_token()->type == GREATER || get_current_token()->type == GTEQ
		|| get_current_token()->type == LESS || get_current_token()->type == LTEQ) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = prev;
		e->expression.rval = shift_expr(NULL);
		return relational_expr(e);
	} else {
		return prev;
	}
}

/*
 * equality-expression:
 * 	relational-expression
 * 	equality-expression == relational-expression
 * 	equality-expression != relational-expression
 */
node *equality_expr(node *prev) {
	if(prev == NULL) {
		prev = relational_expr(NULL);
	}

	if (get_current_token()->type == EQUAL || get_current_token()->type == NOTEQ) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = prev;
		e->expression.rval = relational_expr(NULL);
		return equality_expr(e);
	} else {
		return prev;
	}
}

/*
 * AND-expression:
 * 	equality-expression
 * 	AND-expression & equality-expression
 */
node *and_expr(node *prev) {
	if(prev == NULL) {
		prev = equality_expr(NULL);
	}

	if (get_current_token()->type == AMPER) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = prev;
		e->expression.rval = equality_expr(NULL);
		return and_expr(e);
	} else {
		return prev;
	}
}

/*
 * exclusive-OR-expression:
 * 	AND-expression
 * 	exclusive-OR-expression ^ AND-expression
 */
node *xor_expr(node *prev) {
	if(prev == NULL) {
		prev = and_expr(NULL);
	}

	if (get_current_token()->type == CARET) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = prev;
		e->expression.rval = and_expr(NULL);
		return xor_expr(e);
	} else {
		return prev;
	}
}

/*
 * inclusive-OR-expression:
 * 	exclusive-OR-expression
 * 	inclusive-OR-expression | exclusive-OR-expression
 */
node *or_expr(node *prev) {
	if(prev == NULL) {
		prev = xor_expr(NULL);
	}

	if (get_current_token()->type == PIPE) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = prev;
		e->expression.rval = xor_expr(NULL);
		return or_expr(e);
	} else {
		return prev;
	}
}

/*
 * logical-AND-expression:
 * 	inclusive-OR-expression
 * 	logical-AND-expression && inclusive-OR-expression
 */
node *logand_expr(node *prev) {
	if(prev == NULL) {
		prev = or_expr(NULL);
	}

	if (get_current_token()->type == LOGAND) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = prev;
		e->expression.rval = or_expr(NULL);
		return logand_expr(e);
	} else {
		return prev;
	}
}

/*
 * logical-OR-expression:
 * 	logical-AND-expression
 * 	logical-OR-expression || logical-AND-expression
 */
node *logor_expr(node *prev) {
	if(prev == NULL) {
		prev = logand_expr(NULL);
	}

	if (get_current_token()->type == LOGOR) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = prev;
		e->expression.rval = logand_expr(NULL);
		return logor_expr(e);
	} else {
		return prev;
	}
}

/*
 * conditional-expression:
 * 	logical-OR-expression
 * 	logical-OR-expression ? expression : conditional-expression
 */
node *conditional_expr(void) {
	/* Ternary operation not yet implemented. */
	return logor_expr(NULL);
}

node *constant_expr(void) {
	return conditional_expr();}

bool is_assignment_operator(token *t) {
	switch(t->type) {
		case ASSIGN:
		case ADD_ASSIGN:
		case SUB_ASSIGN:
		case MUL_ASSIGN:
		case DIV_ASSIGN:
		case AMPER_ASSIGN:
		case CARET_ASSIGN:
		case PIPE_ASSIGN:
		case LSHIFT_ASSIGN:
		case RSHIFT_ASSIGN:
			return true;

		default:
			return false;
	}
}

/*
 * assignment-expression:
 * 	conditional-expression
 * 	unary-expression assignment-operator assignment-expression
 */
node *assignment_expr(node *prev) {
	if(prev == NULL) {
		prev = conditional_expr();
	}

	if(is_assignment_operator(get_current_token()) == true) {
		node *e = new_node(ASSIGNMENT_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = unary_expr();
		e->expression.rval = prev;
		return assignment_expr(e);
	} else {
		return prev;
	}
}

node *parse_expr(void) {
	return assignment_expr(NULL);
}

node *parse_stmt(void) {
	 return NULL;
}

/*
 * (Only supports these for now)
 * type-specifier:
 * 	void char int
 */
token_type parse_type_specifier(void) {
	token_type t = get_current_token()->type;
	consume_token();	
	switch(t) {
		case INT:
		case CHAR:
		case VOID:
			return t;

		default:
			error("Undefined type specifier in declaration.");
			return t;
	}
}

/*
 * (Only supports this for now)
 * declaration-specifiers:
 * 	type-specifier
 */
token_type parse_decl_specifiers(void) {
	return parse_type_specifier();	
}

void print_type_specifier(token_type s) {
	switch(s) {
		case VOID:
			printf("void");
			break;
		case CHAR:
			printf("char");
			break;
		case SHORT:
			printf("short");
			break;
		case INT:
			printf("int");
			break;
		case LONG:
			printf("long");
			break;
		case FLOAT:
			printf("float");
			break;
		case DOUBLE:
			printf("double");
			break;
		case SIGNED:
			printf("signed");
			break;
		case UNSIGNED:
			printf("unsigned");
			break;
		case STRUCT:
			printf("struct");
			break;
		case UNION:
			printf("union");
			break;
		default:
			printf("unknown type specifier");
			break;
	}
}

/* Print a declaration as a sentence */
void print_decl(node *d) {
	switch(d->type) {
		case DECLARATION_NODE:
			print_decl(d->declaration.declarator);
			print_type_specifier(d->declaration.specifier);
			printf("\n");
			return;

		case DECLARATOR_NODE:
			print_decl(d->declarator.direct_declarator);
			if(d->declarator.is_pointer == true) {
				printf("pointer to ");
			}
			break;

		case IDENTIFIER_NODE:
			printf("declare %s as ", (char *)d->identifier.tok->attr);
			return;

		case ARRAY_DECL_NODE:
			print_decl(d->direct_declarator.direct);
			printf("array of ");
			return;

		case FUNC_DECL_NODE:
			print_decl(d->direct_declarator.direct);
			printf("function returning ");
			return;

		default:
			printf("Unknown node type.\n");
			return;
	}
}

char *get_decl_identifier(node *d) {
	switch(d->type) {
		case DECLARATION_NODE:
			return get_decl_identifier(d->declaration.declarator);
			
		
		case DECLARATOR_NODE:
			return get_decl_identifier(d->declarator.direct_declarator);

		case ARRAY_DECL_NODE:
		case FUNC_DECL_NODE:
			return get_decl_identifier(d->direct_declarator.direct);
			

		case IDENTIFIER_NODE:
			return (char *)d->identifier.tok->attr;

		default:
			error("Unknown node in declaration.");
			return NULL;
	}
}


//token_type get_decl_type(node *decl) {
//}

/*
 * declarator:
 * 	pointer[opt] direct-declarator
 *
 * direct-declarator:
 * 	identifier
 * 	direct-declarator [ constant-expression[opt] ]
 *  direct-declarator ( parameter-type-list )
 *
 * 	Focus on implementing stuff thats actually useful
 */
node *parse_declarator(node *prev) {
	node *d;
	//printf("parse_declarator()\n");
	//print_token_type(get_current_token()->type);
	switch(get_current_token()->type) {
		case ASTERISK:
			d = new_node(DECLARATOR_NODE);
			d->declarator.is_pointer = true;
			consume_token();
			d->declarator.direct_declarator = parse_declarator(NULL);
			break;	
		
		case IDENTIFIER:
			d = new_node(IDENTIFIER_NODE);
			d->identifier.tok = get_current_token();
			consume_token();
			break;	

		case LPAREN:
			consume_token();
			if(prev == NULL) {
				d = parse_declarator(NULL);
				consume_token(); /* rparen */
			} else {
				d = new_node(FUNC_DECL_NODE);
				d->direct_declarator.direct = prev;

				//d->direct_declarator.params = parse_parameter_type_list();

				if(prev->type == DECLARATOR_NODE) {
					if(prev->declarator.is_pointer == true) {
						error("Function pointers are not supported.");
					}
				}
				consume_token(); /* rparen */
			}
			break;

		case LBRACK:
			consume_token();
			if(prev == NULL) {
				error("Expected identifier before '[' token.");
				return NULL;
			} else {
				d = new_node(ARRAY_DECL_NODE);
				d->direct_declarator.direct = prev;
				d->direct_declarator.params = constant_expr(); /* [x] */
				//printf("test case lbrack\n");
				//print_token_type(get_current_token()->type);			
				consume_token(); /* ] */
			}
			break;

		default:
			return prev;
	}
	return parse_declarator(d);
}

node *parse_initializer_list(node *prev) {
	//printf("parse_initializer_list()\n");
	if(prev == NULL) {
		prev = assignment_expr(NULL);
	}

	if(get_current_token()->type == COMMA) {
		consume_token();
		prev->next = assignment_expr(NULL);
		return parse_initializer_list(prev->next);
	} else {
		return prev;
	}
}

node *parse_decl_initializers(void) {
	//printf("parse_initializers()\n");
	if(get_current_token()->type == LBRACE) { /* { */
		consume_token(); /* { */
	//	printf("consume {\n");
		node *i = parse_initializer_list(NULL);
		consume_token(); /* } */
	//	printf("consume }\n");
		return i;
	} else {
		//printf("parse_decl_initializers()\n");
	//	print_token_type(get_current_token()->type);
		return assignment_expr(NULL);
	}
}

/*
 * declaration:
 * 	declaration-specifiers init-declarator-list_opt ;
 */

node *parse_declaration(void) {
	//printf("parse_declaration()\n");
	node *d = new_node(DECLARATION_NODE);
	d->declaration.specifier = parse_decl_specifiers();
	d->declaration.declarator = parse_declarator(NULL);

	if(get_current_token()->type == ASSIGN) {
		/* parse initializer */
		consume_token();
		node *dctor = d->declaration.declarator;
		dctor->declarator.initialiser = parse_decl_initializers();
	}

	return d;
}


#define COUNT 3
void print_tree(node *tree, int n_indent, int n_lvals) {

	int l = n_lvals;

	//if(n_indent > 0) {
		for(int i = 0; i < n_indent; i++) {
			if(i % COUNT == 0 && i != COUNT) {
				if(l >= 0) {
					printf("|");
					l--;
				} else {
					printf(" ");
				}
			} else {
				printf(" ");
			}
		}
	//}

	switch(tree->type) {
		case INTEGER_CONSTANT_NODE:
			printf("|-");
			printf("%s\n", (char *)tree->constant.tok->attr);	
			
			break;

		case BINARY_EXPR_NODE:
			if(n_indent > 0) {
				printf("|-");
			}
			print_token_type(tree->expression.o);

			if(n_indent > 0) {
				n_lvals++;
			}

			n_indent+=COUNT;

			print_tree(tree->expression.rval, n_indent, n_lvals);
			
			if(n_indent == COUNT) {
				print_tree(tree->expression.lval, 0, 0);
			} else {
				print_tree(tree->expression.lval, n_indent, n_lvals);
			}
			break;
	}

	return;
}



/*
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
*/
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
