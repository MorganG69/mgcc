#include "../inc/lex.h"
#include "../inc/node.h"
#include "../inc/token.h"
#include "../inc/stmt.h"
#include "../inc/expr.h"
#include "../inc/decl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int hex_str_to_int(char *str) {
  return strtoul(str, NULL, 16);
}

int dec_str_to_int(char *str) {
  return strtol(str, NULL, 10);
}

int oct_str_to_int(char *str) {
	return strtol(str, NULL, 8);
}

int constant_str_to_int(char *str) {
	int val;
	if(str[0] == '0') {
		/* Could be a hex or an octal constant. */
		if(str[1] == 'x') {
			val = hex_str_to_int(str);
		} else {
			val = oct_str_to_int(str);
		}
	} else {
		val = dec_str_to_int(str);
	}
	return val;
}

void parse_suffix(node *c, char *s) {
	if(!strcmp(s, "u")) {
		c->constant.is_unsigned = true;
	} else if(!strcmp(s, "l")) {
		c->constant.is_long = true;
	} else if(!strcmp(s, "U")) {
		c->constant.is_unsigned = true;
	} else if(!strcmp(s, "L")) {
		c->constant.is_long = true;
	} else if(!strcmp(s, "ul")) { 
		c->constant.is_unsigned = true;
		c->constant.is_long = true;
	} else if(!strcmp(s, "lu")) { 
		c->constant.is_unsigned = true;
		c->constant.is_long = true;
	} else if(!strcmp(s, "UL")) { 
		c->constant.is_unsigned = true;
		c->constant.is_long = true;
	} else if(!strcmp(s, "LU")) { 
		c->constant.is_unsigned = true;
		c->constant.is_long = true;
	} else if(!strcmp(s, "Ul")) { 
		c->constant.is_unsigned = true;
		c->constant.is_long = true;
	} else if(!strcmp(s, "Lu")) { 
		c->constant.is_unsigned = true;
		c->constant.is_long = true;
	} else if(!strcmp(s, "uL")) { 
		c->constant.is_unsigned = true;
		c->constant.is_long = true;
	} else if(!strcmp(s, "lU")) { 
		c->constant.is_unsigned = true;
		c->constant.is_long = true;
	} else {
		error("invalid suffix on integer constant");
	}
}

node *parse_integer_constant(void) {
	node *n = new_node(INTEGER_CONSTANT_NODE);
	n->constant.tok_str = (char *)get_current_token()->attr;
	consume_token(); /* Eat the token */
	n->constant.val = constant_str_to_int(n->constant.tok_str);
	
	char *ptr = n->constant.tok_str;
	
	while(*ptr) {
		if(*ptr == 'u' || *ptr == 'U' || *ptr == 'l' || *ptr == 'L') {
			parse_suffix(n, ptr);
			break;
		} else {
			ptr++;
		}
	}
	return n;
}

node *parse_character_constant(void) {
	node *n = new_node(CHAR_CONSTANT_NODE);
	char *c = (char *)get_current_token()->attr;
	char *err = NULL;
	int val = 0;
	consume_token();

	//printf("test = %s\n", c);
	
	if(c[0] == '\\') {
		switch(c[1]) {
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
				/* err is used to detect if any non octal digits are in the constant. */
				val = strtol(&c[1], &err, 8);

				if(val > 255) {
					error("octal constant is out of range");
					val = 255; /* set it to the max safe value */
				}
				/* octal number: \ooo */
				if(strlen(c) > 4 || *err != '\0') {
					error("multi-character character constant");
				}
			break;

			case 'x':
				if(c[2] == '\0') {
					error("\\x used with no following hex digits");
				} else {
					val = strtol(&c[2], &err, 16);
					//printf("val = %d\n", val);	
					/* hex number: \xhh */
					if(strlen(c) > 4 || *err != '\0') {
						error("multi-character character constant");
					}
				}
			break;

			case 'n':
				val = '\n';
			break;

			case 't':
				val = '\t';
			break;
			
			case 'v':
				val = '\v';
			break;

			case 'b':
				val = '\b';
			break;

			case 'r':
				val = '\r';
			break;

			case 'f':
				val = '\f';
			break;
			
			case 'a':
				val = '\a';
			break;

			case '\\':
				val = '\\';
			break;

			case '?':
				val = '?';
			break;

			case '\'':
				val = '\'';
			break;

			case '\"':
				val = '\"';
			break;

			default:
				error("unknown escape sequence");
			break;
		}
	} else {
		if(strlen(c) > 1) {
			error("multi-character character constant");
		} else {
			val = c[0];
		}
	}

	n->constant.val = val;
	return n; 
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
			n = parse_integer_constant();	
		break;

		/* string literals */
		case QUOTE:
			consume_token();
			n = new_node(STRING_LITERAL_NODE);
		
			if(get_current_token()->type == STRING_LITERAL) {
				n->constant.tok_str = (char *)get_current_token()->attr;
				consume_token();
			}
			
			if(get_current_token()->type == QUOTE) {
				consume_token();
			} else {
				error("Missing terminating \" character");
			}
		break;
	    
		/* character constants */
		case APOSTROPHE:
			consume_token();
			if(get_current_token()->type == CHAR_CONST) {
				n = parse_character_constant();

				if(get_current_token()->type != APOSTROPHE) {
					error("Missing terminating \' character");
				} else {
					consume_token();
				}
			} else {
				error("Empty character constant");
			}
		break;

		case IDENTIFIER:
			n = new_node(IDENTIFIER_NODE);
			n->constant.tok_str = (char *)get_current_token()->attr;
			consume_token();	
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

		case SEMI_COLON:
		case RPAREN:
		/* ignore */
		break;

		default:
			error("invalid token within expression");
		break;

	}
	return n;
}


node *parse_argument_expr_list(node *prev) {
/*
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
*/
	node *head = assignment_expr(NULL);
	node *tail = head;

	while(!EXPECT_TOKEN(RPAREN)) {
		if(!EXPECT_TOKEN(COMMA)) {
			error("expected ')' before expression");
			break;
		} else {
			consume_token();
		}
		tail->next = assignment_expr(NULL);
		tail = tail->next;
	}
	return head;
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
 * cast-expression
 * 	unary-expression
 * 	( type-name ) cast-expression
 */

node *cast_expr(node *prev) {
	if(!EXPECT_TOKEN(LPAREN)) {
		if(prev == NULL) {
			return unary_expr();
		} else {
			return prev;
		}
	} else {
		if(is_declaration(peek_next_token()->type)) {
			consume_token();
			node *c = new_node(CAST_EXPR_NODE);
			c->cast.a_decl = parse_abstract_declaration();
			
			if(!EXPECT_TOKEN(RPAREN)) {
				error("expected ')' before expression");
			} else {
				consume_token();
			}

			c->cast.expr = cast_expr(NULL);
			return cast_expr(c);
		} else {
			return unary_expr();
		}	
	}
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
		prev = cast_expr(NULL);
	}

	if(get_current_token()->type == DIVIDE || get_current_token()->type == ASTERISK || get_current_token()->type == MOD) {
		node *e = new_node(BINARY_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = prev;
		e->expression.rval = cast_expr(NULL);
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

	//printf("additive_expr(): ");
	//print_token_type(get_current_token()->type);
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
	return conditional_expr();
}

bool is_assignment_operator(token *t) {
	switch(t->type) {
		case ASSIGN:
		case ADD_ASSIGN:
		case SUB_ASSIGN:
		case MOD_ASSIGN:
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

	//printf("assignment_expr(): ");
	//print_token_type(get_current_token()->type);
	if(is_assignment_operator(get_current_token()) == true) {
		node *e = new_node(ASSIGNMENT_EXPR_NODE);
		e->expression.o = get_current_token()->type;
		consume_token();
		e->expression.lval = prev;
		e->expression.rval = assignment_expr(NULL);
		return assignment_expr(e);
	} else {
		return prev;
	}
}

node *parse_expr(void) {
	debug("parse_expr()");
	return assignment_expr(NULL);
}
