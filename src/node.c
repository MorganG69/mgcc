//
// Created by Morgan Greenhill on 25/04/2022.
//

#include "../inc/node.h"

node *new_node(node_type type) {
  node *n = calloc(1, sizeof(node));
  n->type = type;
  return n;
}



node_stack *node_stack_init(size_t size) {
  node_stack *s = calloc(1, sizeof(node_stack));
  s->st = calloc(size, sizeof(token *));
  s->sp = 0;
  return s;
}

node *pop_node(node_stack *s) {
  return s->st[--s->sp];
}

void push_node(node_stack *s, node *t) {
  s->st[s->sp++] = t;
}

bool node_stack_empty(node_stack *s) {
  return s->sp == 0 ? true : false;
}

node *peek_node_stack(node_stack *s) {
  if(s->sp == 0) {
    return NULL;
  } else {
    return s->st[(s->sp - 1)];
  }
}

node *peek_node_stack_nth(node_stack *s, size_t n) {
  if(s->sp == 0) {
    return NULL;
  } else {
    return s->st[(s->sp - n)];
  }
}

