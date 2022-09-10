#include "../inc/lex.h"
#include "../inc/node.h"
#include "../inc/token.h"
#include "../inc/stmt.h"
#include "../inc/expr.h"
#include "../inc/decl.h"
#include "../inc/table.h"
#include <stdio.h>
#include <stdlib.h>

bool is_statement(token_type t);

void parser_panic(void) {
	while(get_current_token()->type != SEMI_COLON) {
		consume_token();
	}
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
		case ASTERISK:
			return true;
		
		default:
			return false;
	}
}

/* statement-list:
 * 	statement
 * 	statement-list statement
 */
node *parse_statement_decl_list(void) {
	node *tail;
	node *head;
	//print_token_type(get_current_token()->type);
	if(is_statement(get_current_token()->type)) {
		head = parse_statement();
	} else {
		head = parse_declaration();
	}

	tail = head;

	while(1) {
		if(is_statement(get_current_token()->type)) {
			tail->next = parse_statement();
		} else if(is_declaration(get_current_token()->type)) {
			tail->next = parse_declaration();
		} else {
			break;
		}
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
	enter_scope();
	node *c = new_node(COMPOUND_STMT_NODE);
	//c->statement.expr = parse_decl_list(NULL); /* Declarations can involve expressions */
	c->statement.stmt = parse_statement_decl_list();

	if(get_current_token()->type != RBRACE) {
		error("expected '}'");
	} else {
		consume_token();
	}
//	print_symbol_table();
	exit_scope();
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
		f->for_statement.expr_1 = parse_expr();
		consume_token();
		f->for_statement.expr_2 = parse_expr();
		consume_token();
		f->for_statement.expr_3 = parse_expr();

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
		case ASTERISK:
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

void print_node_type(node_type type) {
	printf("|- ");
	switch(type) {
		case INTEGER_CONSTANT_NODE:
			printf("INTEGER_CONSTANT_NODE\n");
		break;

		case STRING_LITERAL_NODE:
			printf("STRING_LITERAL_NODE\n");
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

		case DECLARATOR_NODE:
			printf("DECLARATOR_NODE"); /* no newline needed, handled elsewhere. */
		break;
		
		case ARRAY_DECL_NODE:
			printf("ARRAY_DECL_NODE\n");
		break;

		case FUNC_DECL_NODE:
			printf("FUNC_DECL_NODE\n");
		break;

		case FUNC_DEF_NODE:
			printf("FUNC_DEF_NODE\n");
		break;

		case RETURN_STMT_NODE:
			printf("RETURN_STMT_NODE\n");
		break;

		case FUNCTION_CALL_NODE:
			printf("FUNCTION_CALL_NODE\n");
		break;

		case CAST_EXPR_NODE:
			printf("CAST_EXPR_NODE\n");
		break;

		case ARRAY_ACCESS_NODE:
			printf("ARRAY_ACCESS_NODE\n");
		break;

		case CHAR_CONSTANT_NODE:
			printf("CHAR_CONSTANT_NODE\n");
		break;

		default:
			printf("Unimplemented node type: %d\n", type);
		break;
	}
}

void print_statement(node *s, int indent);

void print_statement_list(node *l, int indent) {
	node *head = l;
	while(head != NULL) {
		print_statement(head, indent);
		head = head->next;
	}
}

void print_statement(node *s, int indent) {
	//print_node_type(s->type);
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
			print_node_type(s->type);
			for(int i = 0; i <= indent*2; i++) {
				printf(" ");
			}

			printf("`- %d\n", s->constant.val);
			/*
			if(s->constant.tok != NULL) {
				printf("`- %s\n", (char *)s->constant.tok->attr);
			} else {
				printf("`- empty string literal\n");
			}
			*/
		break;

			


		case DECLARATION_NODE:
			print_node_type(s->type);
			for(int i = 0; i < indent*2; i++) {
				printf(" ");
			}
			printf("`- ");
			print_type_specifier(get_decl_type(s));
			
			indent++;
			//print_node_type(s->declaration.declarator->type);
			print_statement(s->declaration.declarator, indent);
			//if(s->type == FUNC_DEF_NODE) {
			//	printf("test568\n");
			//	print_statement(s->declaration.stmt, indent);
			//} else {
				print_statement_list(s->declaration.initialiser, indent);
			//}
			indent--;
		break;

		

		case DECLARATOR_NODE:
			print_node_type(s->type);
			if(s->declarator.is_pointer == true) {
				printf("_POINTER\n");
			} else {
				printf("\n");
			}
			indent++;
			print_statement(s->declarator.direct_declarator, indent);
			indent--;
		break;
	
		case ARRAY_ACCESS_NODE:
		case FUNCTION_CALL_NODE:
			print_node_type(s->type);
			indent++;
			print_statement(s->postfix.lval, indent);
			print_statement_list(s->postfix.params, indent);
			indent--;
		break;


	    case FUNC_DEF_NODE:
		case FUNC_DECL_NODE:
		case ARRAY_DECL_NODE:
			print_node_type(s->type);
			indent++;
			print_statement(s->direct_declarator.direct, indent);
			print_statement_list(s->direct_declarator.params, indent);
			if(s->type == FUNC_DEF_NODE) {
				print_statement(s->direct_declarator.stmt, indent);
			}
			indent--;
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
		
		case CAST_EXPR_NODE:
			print_node_type(s->type);
			indent++;
			print_statement(s->cast.a_decl, indent);
			print_statement(s->cast.expr, indent);
			indent--;
		break;

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
			
			print_statement_list(s->statement.expr, indent);
			print_statement_list(s->statement.stmt, indent);
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
			printf("Unknown statement type: %d\n", s->type);
			return;
	}
	return;
}

