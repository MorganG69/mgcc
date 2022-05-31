//
// Created by morgan on 08/04/2022.
//

#ifndef CC_TOKEN_H
#define CC_TOKEN_H

#include "lex.h"
#include <stdbool.h>

#define MAX_EXPRESSION_SIZE 32

typedef struct {
    token **st;
    size_t sp;
} token_stack;

token_stack *token_stack_init(size_t size);
token *pop_token(token_stack *s);
void push_token(token_stack *s, token *t);
bool token_stack_empty(token_stack *s);
token *peek_stack(token_stack *s);
token *peek_stack_2nd(token_stack *s);
#endif //CC_TOKEN_H
