#ifndef TRIPLE_H
#define TRIPLE_H

#include "lex.h"
#include <stdint.h>

typedef int arg_type;

/* Other arguments use the token type definitions such as constant or identifier */
enum {
	TRIPLE
};

/* Arguments can either be references to nodes or other triples. */
typedef struct _argument argument;
typedef struct _triple triple;
typedef struct _triple_list t_list;

struct _argument {
	arg_type a_type;	
	union {
		node *n_arg;
		triple *t_arg;
	};
};

struct _triple {
	size_t id;
	token_type op;
	argument arg_1;
	argument arg_2;
	triple *next;
};

struct _triple_list {
	size_t count;
	triple *head;
	triple *tail;
};


#endif /* TRIPLE_H */
