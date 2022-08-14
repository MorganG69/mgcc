#include "../inc/lex.h"
#include "../inc/node.h"
#include "../inc/token.h"
#include "../inc/stmt.h"
#include "../inc/expr.h"
#include <stdio.h>
#include <stdlib.h>

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

node *parse_parameter_list(void) {
	debug("parse_parameter_list()");
	node *head = parse_abstract_declaration();
	node *tail = head;

	while(!EXPECT_TOKEN(RPAREN)) {
		if(!EXPECT_TOKEN(COMMA)) {
			error("expected ')' before expression");
			break;
		} else {
			consume_token();
		}
		tail->next = parse_abstract_declaration();
		tail = tail->next;
	}
	return head;
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
			} else {
				d = new_node(FUNC_DECL_NODE);
				d->direct_declarator.direct = prev;
				d->direct_declarator.params = parse_parameter_list();
			}
			consume_token(); /* rparen */
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
	node *head = assignment_expr(NULL);
	node *tail = head;

	while(!EXPECT_TOKEN(RBRACE)) {
		if(!EXPECT_TOKEN(COMMA)) {
			error("expected '}' before expression");
			break;
		} else {
			consume_token();
		}
		tail->next = assignment_expr(NULL);
		tail = tail->next;
	}
	return head;
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
			d->declaration.initialiser = parse_decl_initializers();
		} else if(get_current_token()->type == LBRACE) {
			d->type = FUNC_DEF_NODE;
			consume_token();
			d->declaration.stmt = parse_compound_statement();
			return d;
		}
	} else {
		/* Does this handle branch actually handle abstract decls? */
		/* UPDATE:
		 * It does! However this needs to be it's own function
		 * as the context is important.
		 */
		error("expected identifier or '('");
	}

	if(!EXPECT_TOKEN(SEMI_COLON)) {
		if(!EXPECT_TOKEN(COMMA) && !EXPECT_TOKEN(LBRACE)) {
			//printf("test: %s\n", (char *)get_current_token()->attr);
			error("expected ';' at end of declaration");
		} /* otherwise leave it */	
	} else {
			consume_token();
	}
	return d;
}




node *parse_abstract_declarator(node *prev);

/*
 * abstract-declarator
 * 	pointer
 * 	pointer[opt] direct-abstract-declarator
 *
 * direct-abstract-declarator
 * 	( abstract-declarator )
 * 	direct-abstract-declarator[opt] [ constant-expression ]
 * 	direct-abstract-declarator[opt] ( parameter-type-list )
 */
 node *parse_abstract_declarator(node *prev) {
	node *d;
	debug("parse_abstract_declarator()");
	switch(get_current_token()->type) {
		case ASTERISK:
			d = new_node(DECLARATOR_NODE);
			d->declarator.is_pointer = true;
			consume_token();
			d->declarator.direct_declarator = parse_abstract_declarator(NULL);
		break;	
		
		case IDENTIFIER:
			d = new_node(IDENTIFIER_NODE);
			d->identifier.tok = get_current_token();
			consume_token();	
		break;	

		case LPAREN:
			consume_token();
			if(prev == NULL) {
				d = parse_abstract_declarator(NULL);
			} else {
				d = new_node(FUNC_DECL_NODE);
				d->direct_declarator.direct = prev;
				d->direct_declarator.params = parse_parameter_list();
			}
			consume_token(); /* rparen */
		break;

		case LBRACK:
			consume_token();
			d = new_node(ARRAY_DECL_NODE);
			d->direct_declarator.direct = prev;
			d->direct_declarator.params = constant_expr(); /* [x] */
			if(!EXPECT_TOKEN(RBRACK)) {
				error("expected ']' at end of statement");
			} else {
				consume_token(); /* ] */
			}
		break;

		default:
			return prev;
	}
	return parse_abstract_declarator(d);
}


/*
 * declaration:
 * 	declaration-specifiers init-declarator-list_opt ;
 */
node *parse_abstract_declaration(void) {
	debug("parse_abstract_declaration()");
	node *d = new_node(DECLARATION_NODE);
	d->declaration.specifier = parse_decl_specifiers();
	/* This could be NULL, but that doesn't matter because this is an abstract decl */
	d->declaration.declarator = parse_abstract_declarator(NULL);
	return d;
}
