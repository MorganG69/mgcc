//
// Created by Morgan Greenhill on 25/04/2022.
//

#ifndef CC_NODE_H
#define CC_NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "lex.h"
#include "token.h"
#include "error.h"

/* AST node types */
enum {
  INTEGER_CONSTANT_NODE = 0,
  CHAR_CONSTANT_NODE,
  STRING_LITERAL_NODE,
  IDENTIFIER_NODE,
  ASSIGNMENT_EXPR_NODE,
  BINARY_EXPR_NODE,
  UNARY_EXPR_NODE,
  POSTFIX_EXPR_NODE,
  ARRAY_ACCESS_NODE,
  FUNCTION_CALL_NODE,
  DECLARATION_NODE,
  ARRAY_DECL_NODE,
  FUNC_DECL_NODE,
  DECLARATOR_NODE,
  DIRECT_DECLARATOR_NODE,
  ERROR_NODE
};

typedef token_type operation;
typedef token_type type_specifier;
typedef int node_type;


typedef struct _node node;
typedef struct _node_stack node_stack;
typedef struct _node_queue node_queue;

struct _node_stack {
  node **st;
  size_t sp;
};

struct _node_queue {
	node *head;
	node *tail;
	size_t count;
};

struct _node {
  node_type type;
  node *next;
  union {
	  struct constant_node {
		token *tok;
		char *val;
	  } constant;
	
	  struct expression_node {
		operation o;
		node *lval;
		node *rval;
	  } expression;

	  struct identifier_node {
		  bool ptr;
		  token *tok;
		  /* Symbol table entry */
	  } identifier;
 		
	  struct unary_node {
		  operation o;
		  node *rval;
	  } unary;

	  struct postfix_node {
		  operation o;
		  node *lval;
		  node *params; /* Used for array/struct access and function calls */
	  } postfix;
 	 
	  struct declaration_node {
		token_type specifier;
		/*
		 * Can be:
		 * 	init declarator
		 * 	struct declarator
		 * 	union declarator
		 * 	enum declarator
		 */	
		node *declarator;
	  } declaration;

	  struct init_decl_node {
		node *declarator;
		node *initialiser;
	  } init_decl;

	  struct declarator_node {
		bool is_pointer;
		node *direct_declarator;
	  	node *initialiser;
	  } declarator;

	  /* 
	   * Can be either array or function
	   */
	  struct direct_declarator_node {
		node *direct;
		node *params;
	  } direct_declarator;
  };
};

node *new_node(node_type type);

node_stack *node_stack_init(size_t size);
node *pop_node(node_stack *s);
void push_node(node_stack *s, node *t);
bool node_stack_empty(node_stack *s);
node *peek_node_stack(node_stack *s);
node *peek_node_stack_nth(node_stack *s, size_t n);

node_queue *node_queue_init(void);
void node_enqueue(node_queue *q, node *n);
node *node_dequeue(node_queue *q);


#endif //CC_NODE_H
