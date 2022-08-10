#include "../inc/lex.h"
#include "../inc/node.h"
#include "../inc/token.h"
#include "../inc/parse.h"
#include <stdio.h>
#include <stdlib.h>

#define EXPECT_TOKEN(t) (get_current_token()->type == (t))

node *assignment_expr(node *prev);
node *parse_statement(void);
node *parse_declaration(void);
bool is_declaration(token_type t); 
bool is_statement(token_type t);

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

		case SEMI_COLON:
		case RPAREN:
		/* ignore */
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

/**
 * Converts a hexadecimal string to int.
 * @param str ptr to string
 * @return int value of string
 */
int hex_str_to_int(char *str) {
  return strtoul(str, NULL, 16);
}

/**
 * Converts a decimal string to int.
 * @param str ptr to string
 * @return int value of string
 */
int dec_str_to_int(char *str) {
  return strtoul(str, NULL, 10);
}


bool is_statement(token_type t) {
	switch(t) {
		case IDENTIFIER:
		case CASE:
		case DEFAULT:
		case IF:
		case SWITCH:
		case WHILE:
		case DO:
		case FOR:
		case GOTO:
		case CONTINUE:
		case BREAK:
		case RETURN:
			return true;
		
		default:
			return false;
	}
}

/* statement-list:
 * 	statement
 * 	statement-list statement
 */
node *parse_statement_list(void) {
	node *head = parse_statement();
	node *tail = head;

	while(is_statement(get_current_token()->type)) {
		tail->next = parse_statement();
		tail = tail->next;
	}
	return head;
}

/* case constant-expression : statement */
node *parse_case_statement(void) {
	node *c = new_node(CASE_STMT_NODE);
	c->statement.expr = constant_expr();
	if(get_current_token()->type != COLON) {
		error("Expected ':' after 'case'");
	} else {
		consume_token();
		c->statement.stmt = parse_statement();
	}
	return c;
}

/* default : statement */
node *parse_default_statement(void) {
	node *d = new_node(DEFAULT_STMT_NODE);
	if(get_current_token()->type != COLON) {
		error("Expected ':' after 'default'");
	} else {
		consume_token();
		d->statement.stmt = parse_statement();
	}
	return d;
}




node *parse_decl_list(node *prev) {
	if(is_declaration(get_current_token()->type)) {
		if(prev == NULL) {
			prev = parse_declaration();
		} 

		if(is_declaration(get_current_token()->type)) {
			prev->next = parse_declaration();
			return parse_decl_list(prev->next);
		} else {
			return prev;
		}
	} else {
		//debug("found no declaration in compound statement");
		return prev;
	}
}

/*
 * { declaration-list[opt] statement-list[opt] }
 */ 
node *parse_compound_statement(void) {
	node *c = new_node(COMPOUND_STMT_NODE);
	//c->statement.expr = parse_decl_list(NULL); /* Declarations can involve expressions */
	c->statement.stmt = parse_statement_list();

	if(get_current_token()->type != RBRACE) {
		error("expected '}'");
	} else {
		consume_token();
	}
	return c;
}


/*
 * if ( expression ) statement
 * if ( expression ) statement else statement
 */
node *parse_if_statement(void) {
	node *i = new_node(IF_STMT_NODE);
	if(!EXPECT_TOKEN(LPAREN)) {
		error("expected '(' before expression");
	} else {
		consume_token();
		i->if_statement.expr = parse_expr();
		if(!EXPECT_TOKEN(RPAREN)) {
			error("expected ')' before statement");
		} else {
			consume_token();
			i->if_statement.i_stmt = parse_statement();
			if(EXPECT_TOKEN(ELSE)) {
				debug("found else in if statement");
				i->type = IF_ELSE_STMT_NODE;
				consume_token();
				i->if_statement.e_stmt = parse_statement();
			}
		}
	}
	return i;
}

/* switch ( expression ) statement */
node *parse_switch_statement(void) {
	node *s = new_node(SWITCH_STMT_NODE);
	if(!EXPECT_TOKEN(LPAREN)) {
		error("expected '(' before expression");
	} else {
		consume_token();
		s->statement.expr = parse_expr();
		if(!EXPECT_TOKEN(RPAREN)) {
			error("expected ')' before statement");
		} else {
			consume_token();
			s->statement.stmt = parse_statement();
		}
	}
	return s;
}

/*
 * while ( expression ) statement
 */
node *parse_while_statement(void) {
	node *w = new_node(WHILE_STMT_NODE);
	if(!EXPECT_TOKEN(LPAREN)) {
		error("expected '(' before expression");
	} else {
		consume_token();
		w->statement.expr = parse_expr();
		if(!EXPECT_TOKEN(RPAREN)) {
			error("expected ')' before statement");
		} else {
			consume_token();
			w->statement.stmt = parse_statement();
		}
	}
	return w;
}

/*
 * do statement while ( expression ) ;
 */
node *parse_do_statement(void) {
	node *d = new_node(DO_STMT_NODE);
	d->statement.stmt = parse_statement();
	if(!EXPECT_TOKEN(WHILE)) {
		error("expected 'while'");
	} else {
		consume_token();
		if(!EXPECT_TOKEN(LPAREN)) {
			error("expected '(' before expression");
		} else {
			consume_token();
			d->statement.expr = parse_expr();
			if(!EXPECT_TOKEN(RPAREN)) {
				error("expected ')' after expression");
			} else {
				consume_token();
				if(!EXPECT_TOKEN(SEMI_COLON)) {
					error("expected ';' at the end of statement");
				} else {
					consume_token();
				}
			}
		}
	}
	return d;
}

/* 
 * for ( expression[opt] ; expression[opt] ; expression[opt] ) statement
 */
node *parse_for_statement(void) {
	node *f = new_node(FOR_STMT_NODE);
	if(!EXPECT_TOKEN(LPAREN)) {
		error("expected '(' before expression");
	} else {
		consume_token();
		node *e_ptr = f->for_statement.expr_1;
		for(int i = 0; i < 2; i++) {
			e_ptr = parse_expr(); /* this will return null if there's no expression */
			e_ptr++;
			if(!EXPECT_TOKEN(SEMI_COLON)) {
				error("expected ';' before expression");
			} else {
				consume_token();
			}
		}
		e_ptr = parse_expr();
		if(!EXPECT_TOKEN(RPAREN)) {
			error("expected ')' before statement");
		} else {
			consume_token();
			f->for_statement.stmt = parse_statement();
		}
	}
	return f;
}

/* goto identifier ; */
node *parse_goto_statement(void) {
	node *g = new_node(GOTO_STMT_NODE);
	if(!EXPECT_TOKEN(IDENTIFIER)) {
		error("expected identifier before ';'");
	} else {
		/* We know its an identifier. primary_expr() consumes the token */
		g->statement.expr = primary_expr(); 
		if(!EXPECT_TOKEN(SEMI_COLON)) {
			error("expected ';' at the end of statement");
		} else {
			consume_token();
		}
	}
	return g;
}

/*
 * continue ;
 * break ;
 */
node *parse_continue_break(node_type t) {
	node *n = new_node(t);
	if(!EXPECT_TOKEN(SEMI_COLON)) {
		error("expected ';' at the end of statement");
	} else {
		consume_token();
	}
	return n;
}

/* return expression[opt] ; */
node *parse_return_statement(void) {
	node *r = new_node(RETURN_STMT_NODE);
	r->statement.expr = parse_expr(); /* returns null if stmt is just return; */
	if(!EXPECT_TOKEN(SEMI_COLON)) {
		error("expected ';' at the end of statement");
	} else {
		consume_token();
	}
	return r;
}

/* identifier : statement */
node *parse_label_statement(void) {
	node *l = new_node(LABEL_STMT_NODE);
	l->statement.expr = parse_expr(); /* known to be an identifier */
	consume_token(); /* known to be a colon */
	l->statement.stmt = parse_statement();
	return l;
}

node *parse_statement(void) {
	node *s = NULL;
	switch(get_current_token()->type) {
		case IDENTIFIER:
			if(peek_next_token()->type == COLON) {
				debug("parse_label_statement()");
				s = parse_label_statement();
			} else {
				s = parse_expr();
				if(get_current_token()->type != SEMI_COLON) {
					error("expected ';' at end of statement");
				} else {
					consume_token();
				}
			}
		break;
		
		case CASE:
			debug("parse_case_statement()");
			consume_token();
			s = parse_case_statement();
		break;

		case DEFAULT:
			debug("parse_default_statement()");
			consume_token();
			s = parse_default_statement();
		break;

		case LBRACE:
			debug("parse_compound_statement()");
			consume_token();
			s = parse_compound_statement();
		break;

		case IF:
			debug("parse_if_statement()");
			consume_token();
			s = parse_if_statement();
		break;

		case SWITCH:
			debug("parse_switch_statement()");
			consume_token();
			s = parse_switch_statement();
		break;

		case WHILE:
			debug("parse_while_statement()");
			consume_token();
			s = parse_while_statement();
		break;

		case DO:
			debug("parse_do_statement()");
			consume_token();
			s = parse_do_statement();
		break;
		
		case FOR:
			debug("parse_for_statement()");
			consume_token();
			s = parse_for_statement();
		break;

		case GOTO:
			debug("parse_goto_statement()");
			consume_token();
			s = parse_goto_statement();
		break;

		case CONTINUE:
			debug("parse_continue_statement()");
			consume_token();
			s = parse_continue_break(CONTINUE_STMT_NODE);
		break;

		case BREAK:
			debug("parse_break_statement()");
			consume_token();
			s = parse_continue_break(BREAK_STMT_NODE);
		break;

		case RETURN:
			debug("parse_return_statement()");
			consume_token();
			s = parse_return_statement();
		break;

		/* 
		 * Solves the case of default trying to parse a statement and finding nothing. 
		 * Will this cause trouble later???
		 */
		case RBRACE:
		break;

		default:
			error("expected expression");
		break;
	}
	return s;
}

/*
 * (Only supports these for now)
 * type-specifier:
 * 	void char int
 */
token_type parse_type_specifier(void) {
	token_type t = get_current_token()->type;
	//print_token_type(t);
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

bool is_declaration(token_type t) {
	/* Probably need to do some checks for typedef here */
	switch(t) {
		case AUTO:
		case REGISTER:
		case STATIC:
		case EXTERN:
		case TYPEDEF:
		case VOID:
		case CHAR:
		case SHORT:
		case INT:
		case LONG:
		case FLOAT:
		case DOUBLE:
		case SIGNED:
		case UNSIGNED:
		case STRUCT:
		case UNION:
		case ENUM:
		case CONST:
		case VOLATILE:
			return true;
		default:
			return false;
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
	printf("\n");
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
	debug("parse_declarator()");
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

//TODO				//d->direct_declarator.params = parse_parameter_type_list();

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
	debug("parse_initializer_list()");
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
	debug("parse_initializers()");
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
	debug("parse_declaration()");
	//print_type_specifier(get_current_token()->type);
	node *d = new_node(DECLARATION_NODE);
	d->declaration.specifier = parse_decl_specifiers();
	d->declaration.declarator = parse_declarator(NULL);

	if(d->declaration.declarator != NULL) {
		if(get_current_token()->type == ASSIGN) {
			/* parse initializer */
			consume_token();
			node *dctor = d->declaration.declarator;
			dctor->declarator.initialiser = parse_decl_initializers();
		}
	} else {
		error("expected identifier or '('");
	}

	if(get_current_token()->type != SEMI_COLON) {
			error("expected ';' at end of declaration");
	} else {
			consume_token();
	}
	return d;
}

void print_node_type(node_type type) {
	printf("|- ");
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

		case COMPOUND_STMT_NODE:
			printf("COMPOUND_STMT_NODE\n");
		break;

		case DECLARATION_NODE:
			printf("DECLARATION_NODE\n");
		break;
		
		case IDENTIFIER_NODE:
			printf("IDENTIFIER_NODE\n");
		break;
		
		case SWITCH_STMT_NODE:
			printf("SWITCH_STMT_NODE\n");
		break;

		case IF_STMT_NODE:
			printf("IF_STMT_NODE\n");
		break;

		case IF_ELSE_STMT_NODE:
			printf("IF_ELSE_STMT_NODE\n");
		break;

		case FOR_STMT_NODE:
			printf("FOR_STMT_NODE\n");
		break;

		case WHILE_STMT_NODE:
			printf("WHILE_STMT_NODE\n");
		break;

		case DO_STMT_NODE:
			printf("DO_STMT_NODE\n");
		break;

		case CASE_STMT_NODE:
			printf("CASE_STMT_NODE\n");
		break;

		case BREAK_STMT_NODE:
			printf("BREAK_STMT_NODE\n");
		break;

		case DEFAULT_STMT_NODE:
			printf("DEFAULT_STMT_NODE\n");
		break;

		default:
			printf("Unimplemented node type: %d\n", type);
		break;
	}
}

void print_statement(node *s, int indent);
void print_statement(node *s, int indent) {
	node *head;
	if(s == NULL) {
		return;
	}

	for(int i = 0; i < indent*2; i++) {
		printf(" ");
	}
	
	switch(s->type)	{
		case IDENTIFIER_NODE: 
			print_node_type(s->type);
			for(int i = 0; i <= indent*2; i++) {
				printf(" ");
			}
			printf("`- %s\n", (char *)s->identifier.tok->attr);
		break;

		case INTEGER_CONSTANT_NODE:
		case CHAR_CONSTANT_NODE:
		case STRING_LITERAL_NODE:
			print_node_type(s->type);
			for(int i = 0; i <= indent*2; i++) {
				printf(" ");
			}
			printf("`- %s\n", (char *)s->constant.tok->attr);
		break;
		
		case ASSIGNMENT_EXPR_NODE:
		case BINARY_EXPR_NODE:
			print_node_type(s->type);
			indent++;
			print_statement(s->expression.lval, indent);
			print_statement(s->expression.rval, indent);
			indent--;
		break;

		case UNARY_EXPR_NODE:
			print_node_type(s->type);
			indent++;
			print_statement(s->unary.rval, indent);
			indent--;
		break;

		case POSTFIX_EXPR_NODE:
			print_node_type(s->type);
			indent++;
			print_statement(s->postfix.lval, indent);
			indent--;
		break;
		
	//	case 

		case LABEL_STMT_NODE:
		case CASE_STMT_NODE:
		case DEFAULT_STMT_NODE:
		case EXPR_STMT_NODE:
		case COMPOUND_STMT_NODE:
		case SWITCH_STMT_NODE:
		case WHILE_STMT_NODE:
			print_node_type(s->type);
			indent++;
			//print_statement(s->statement.expr, indent);
			//print_statement(s->statement.stmt, indent);
			
			head = s->statement.expr;
			while(head != NULL) {
				print_statement(head, indent);
				head = head->next;
			}

			head = s->statement.stmt;
			while(head != NULL) {
				print_statement(head, indent);
				head = head->next;
			}

			indent--;
		break;

		case GOTO_STMT_NODE:
		case RETURN_STMT_NODE:
			print_node_type(s->type);
			indent++;
			print_statement(s->statement.expr, indent);
			indent--;
		break;

		case CONTINUE_STMT_NODE:
		case BREAK_STMT_NODE:
			print_node_type(s->type);
		break;

		case DO_STMT_NODE:
			print_node_type(s->type);
			indent++;
			print_statement(s->statement.stmt, indent);
			print_statement(s->statement.expr, indent);
			indent--;
		break;

		case IF_STMT_NODE:
			print_node_type(s->type);
			indent++;
			print_statement(s->if_statement.expr, indent);
			print_statement(s->if_statement.i_stmt, indent);
			indent--;
		break;

		case IF_ELSE_STMT_NODE:
			print_node_type(s->type);
			indent++;
			print_statement(s->if_statement.expr, indent);
			print_statement(s->if_statement.i_stmt, indent);
			print_statement(s->if_statement.e_stmt, indent);
			indent--;
		break;
	
		case FOR_STMT_NODE:
			print_node_type(s->type);
			indent++;
			print_statement(s->for_statement.expr_1, indent);
			print_statement(s->for_statement.expr_2, indent);
			print_statement(s->for_statement.expr_3, indent);
			print_statement(s->for_statement.stmt, indent);
			indent--;
		break;
		
		



		default:
			printf("Unknown statement type\n");
			return;
	}
	return;
}

