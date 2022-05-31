//
// Created by Morgan Greenhill on 01/04/2022.
//

#include <stdio.h>
#include <stdbool.h>
#include "../inc/lex.h"

#define PRINT_ERROR   printf("\033[1;31merror: ");printf("\033[0m")



bool error_occurred = false;

bool has_error_occurred(void) {
	return error_occurred;
}

void error (char *err_str) {
  PRINT_ERROR;
  printf("line %d: %s\n", get_line(), err_str);
  error_occurred = true;
};

void file_error(char *err_str) {
	printf("mgcc: ");
	PRINT_ERROR;
	printf("%s\n", err_str);
	error_occurred = true;
}

