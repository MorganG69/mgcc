#ifndef TABLE_H
#define TABLE_H

#include "node.h"

typedef struct _symbol_table symbol_table;
struct _symbol_table {
	int table_num;
	
	size_t sym_count;
	node *head;
	node *tail;

	symbol_table *prev;
	symbol_table *next;
}

#endif /* TABLE_H */


