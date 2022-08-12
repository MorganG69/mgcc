#ifndef EXPR_H
#define EXPR_H

#include "node.h"

node *parse_expr(void);
node *primary_expr(void);
node *constant_expr(void);
node *assignment_expr(node *prev);

#endif /* EXPR_H */
