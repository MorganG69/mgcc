#include "../inc/lex.h"
#include "../inc/node.h"
#include "../inc/token.h"
#include "../inc/stmt.h"
#include "../inc/expr.h"
#include "../inc/table.h"
#include "../inc/decl.h"
#include <stdio.h>
#include <stdlib.h>

node *parse_struct_union(token_type s_or_u);

token_type get_decl_type(node *d) {
	if(d->declaration.specifier->type == STRUCT_DECL_NODE) {
		return STRUCT;
	} else if(d->declaration.specifier->type == UNION_DECL_NODE) {
		return UNION;
	} else {
		return d->declaration.specifier->declaration_spec.s_type;
	}
}

/*
 * (Only supports these for now)
 * type-specifier:
 * 	void char int
 */
token_type parse_type_specifier(void) {
	token_type t = get_current_token()->type;
//	print_token_type(t);
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

/* TODO: WIP 18/8 */
node *parse_decl_specifiers(void) {
	node *s = NULL; 
	token_type t = get_current_token()->type;
	switch(t) {
		case AUTO:
		case REGISTER:
		case STATIC:
		case EXTERN:
		case TYPEDEF:
			warn("storage-class-specifiers not supported");
			consume_token();
			return parse_decl_specifiers();

		case CONST:
		case VOLATILE:
			warn("type-qualifiers not supported");
			consume_token();
			return parse_decl_specifiers();

		case SHORT:
		case LONG:
		case FLOAT:
		case DOUBLE:
		case SIGNED:
		case UNSIGNED:
			error("type-specifier not supported");
			consume_token();
			return parse_decl_specifiers();

		case STRUCT:
		case UNION:
			consume_token();
			s = parse_struct_union(t);
			return s;

		case ENUM:
		case INT:
		case VOID:
		case CHAR:
			s = new_node(DECLARATION_SPEC_NODE);
			s->declaration_spec.s_type = get_current_token()->type;
			consume_token();
			return s;

		default:
			return NULL;
	}
}



void print_type_specifier(token_type s) {
	//print_token_type(s);
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
		case ENUM:
			printf("enum");
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
			print_type_specifier(get_decl_type(d));
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
		case FUNC_DEF_NODE:
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
			d->constant.tok_str = (char *)get_current_token()->attr;
			consume_token();	
		break;	

		case LPAREN:
			consume_token();
			if(prev == NULL) {
				d = parse_declarator(NULL);
			    consume_token();	
			} else {
				d = new_node(FUNC_DECL_NODE);
				d->direct_declarator.direct = prev;
				d->direct_declarator.params = parse_parameter_list();
				consume_token(); /* rparen */
				if(EXPECT_TOKEN(LBRACE)) {
					d->type = FUNC_DEF_NODE;
					consume_token();
					d->direct_declarator.stmt = parse_compound_statement();
				}
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
	node *il = new_node(INITIALIZER_LIST_NODE);
	il->init_list.head = parse_decl_initializers();//assignment_expr(NULL);
	il->init_list.tail = il->init_list.head;
	il->init_list.count++;

	while(!EXPECT_TOKEN(RBRACE)) {
		if(!EXPECT_TOKEN(COMMA)) {
			error("expected '}' before expression");
			break;
		} else {
			consume_token();
		}
		il->init_list.tail->next = parse_decl_initializers();
		il->init_list.tail = il->init_list.tail->next;
		il->init_list.count++;
	}
	return il;
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

node *parse_enumerator_list(void) {
	node *head = parse_expr();
	node *tail = head;

	while(!EXPECT_TOKEN(RBRACE)) {
		if(!EXPECT_TOKEN(COMMA)) {
			error("expected ,");
		} else {
			consume_token();
			/* Handles the case of the final enumerator having a comma. */
			if(get_current_token()->type == RBRACE) {
				break;
			}
		}
		tail->next = parse_expr();
		tail = tail->next;
	}
	return head;
}

node *parse_enum(void) {
	node *e = new_node(ENUM_DECL_NODE);	
	//print_token_type(get_current_token()->type);	
	if(get_current_token()->type == IDENTIFIER) {
		e->comp_declarator.identifier = (char *)get_current_token()->attr;
	//	printf("%s\n", (char *)get_current_token()->attr);
		consume_token();
	}

	if(get_current_token()->type == LBRACE) {
		consume_token();
		e->comp_declarator.decl_list = parse_enumerator_list();
		if(!EXPECT_TOKEN(RBRACE)) {
			error("expected }");
		} else {
			consume_token();
		}
	}
	return e;
}



node *parse_specifier_qualifier_list(void) {
	node *s = NULL;
	token_type t = get_current_token()->type;
	switch(t) {
		case AUTO:
		case REGISTER:
		case STATIC:
		case EXTERN:
		case TYPEDEF:
			error("expected specifier-qualifier-list");
			consume_token();
			return parse_decl_specifiers();

		case CONST:
		case VOLATILE:
			warn("type-qualifiers not supported");
			consume_token();
			return parse_decl_specifiers();

		case SHORT:
		case LONG:
		case FLOAT:
		case DOUBLE:
		case SIGNED:
		case UNSIGNED:
			error("type-specifier not supported");
			consume_token();
			return parse_decl_specifiers();

		case STRUCT:
		case UNION:
			consume_token();
			s = parse_struct_union(t);
			return s;

		case ENUM:
		case INT:
		case VOID:
		case CHAR:
			s = new_node(DECLARATION_SPEC_NODE);
			s->declaration_spec.s_type = get_current_token()->type;
			consume_token();
			return s;

		default:
			return NULL;
	}
}


node *parse_struct_decl(void) {
	node *s = new_node(DECLARATION_NODE);

	s->declaration.specifier = parse_specifier_qualifier_list();
	
	if(get_decl_type(s) == STRUCT || get_decl_type(s) == UNION) {
		s->declaration.declarator = parse_struct_union(get_decl_type(s));	
	} else if(get_decl_type(s) == ENUM) {
		s->declaration.declarator = parse_enum();
	} else {
		s->declaration.declarator = parse_declarator(NULL);
		if(get_current_token()->type == COLON) {
			consume_token();
			s->type = BITFIELD_DECL_NODE;
			s->declaration.initialiser = parse_expr();
		}
	}

	if(!EXPECT_TOKEN(SEMI_COLON)) {
		error("expected ';' at end of declaration");
		consume_token();
	} else {
		consume_token();
	}

	return s;
}

node *parse_struct_decl_list(void) {
	node *head = parse_struct_decl();
	node *tail = head;

	while(!EXPECT_TOKEN(RBRACE)) {
		tail->next = parse_struct_decl();
		tail = tail->next;
	}
	return head;
}

node *parse_struct_union(token_type s_or_u) {
	node *su;
	if(s_or_u == STRUCT) {
		su = new_node(STRUCT_DECL_NODE);
	} else {
		su = new_node(UNION_DECL_NODE);
	}

	if(get_current_token()->type == IDENTIFIER) {
		su->comp_declarator.identifier = (char *)get_current_token()->attr;
		consume_token();
	}

	//print_token_type(get_current_token()->type);
	if(get_current_token()->type == LBRACE) {
		consume_token();
		su->comp_declarator.decl_list = parse_struct_decl_list();
		if(!EXPECT_TOKEN(RBRACE)) {
			error("expected }");
		} else {
			consume_token();
		}
	}

	return su;
}

/*
 * declaration:
 * 	declaration-specifiers init-declarator-list_opt ;
 */
node *parse_declaration(void) {
	debug("parse_declaration()");
	if(get_current_token()->type == END) {
		return NULL;
	}
	node *d = new_node(DECLARATION_NODE);
	d->declaration.specifier = parse_decl_specifiers();
	//print_token_type(get_current_token()->type);
	
	//print_node_type(d->declaration.specifier->type);
	
	switch(get_decl_type(d)) {
		case ENUM:
			d->declaration.declarator = parse_enum();
			
			if(!EXPECT_TOKEN(SEMI_COLON)) {
				error("expected ';' at end of declaration");
			} else {
				consume_token();
			}
		break;

		/*
		case STRUCT:
		case UNION:
			d->declaration.declarator = parse_struct_union(get_decl_type(d));
			if(!EXPECT_TOKEN(SEMI_COLON)) {
				error("expected ';' at end of declaration");
			} else {
				consume_token();
			}
		break;
		*/

		default:
			d->declaration.declarator = parse_declarator(NULL);
			if(d->declaration.declarator != NULL) {
				if(get_current_token()->type == ASSIGN) {
					/* parse initializer */
					consume_token();
					d->declaration.initialiser = parse_decl_initializers();
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
				if((!EXPECT_TOKEN(COMMA) && !EXPECT_TOKEN(LBRACE))) {
					if(d->declaration.declarator->type != FUNC_DEF_NODE) {
						error("expected ';' at end of declaration");
					}
				} /* otherwise leave it as it's part of a list or function definition */	
			} else {
					consume_token();
			}
		break;
	}


	/* TODO: dont allow multiple definitions */
//	add_symbol(d->declaration.declarator->type, get_decl_type(d), get_decl_identifier(d), d->declaration.declarator);
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
	//print_token_type(get_current_token()->type);
	d->declaration.declarator = parse_abstract_declarator(NULL);
	return d;
}



node *parse_translation_unit(void) {
	if(get_current_token()->type != END) {
		node *head = parse_declaration();
		node *tail = head;

		while(!EXPECT_TOKEN(END)) {
			tail->next = parse_declaration();
			tail = tail->next;
		}

		return head;
	} else {
		return NULL;
	}
}
