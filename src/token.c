//
// Created by morgan on 08/04/2022.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../inc/token.h"

token_stack *token_stack_init(size_t size) {
    token_stack *s = calloc(1, sizeof(token_stack));
    s->st = calloc(size, sizeof(token *));
    s->sp = 0;
    return s;
}

token *pop_token(token_stack *s) {
    return s->st[--s->sp];
}

void push_token(token_stack *s, token *t) {
    s->st[s->sp++] = t;
}

bool token_stack_empty(token_stack *s) {
    return s->sp == 0 ? true : false;
}

token *peek_stack(token_stack *s) {
    if(s->sp == 0) {
        return NULL;
    } else {
        return s->st[(s->sp - 1)];
    }
}

token *peek_stack_2nd(token_stack *s) {
  if(s->sp == 0) {
    return NULL;
  } else {
    return s->st[(s->sp - 2)];
  }
}
