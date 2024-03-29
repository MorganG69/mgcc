//
// Created by Morgan Greenhill on 01/04/2022.
//

#include <stdio.h>
#include <stdbool.h>
#include "../inc/lex.h"

#define PRINT_ERROR   printf("\033[1;31merror: ");printf("\033[0m")
#define PRINT_DEBUG   printf("\033[1;34mmgcc-debug: ");printf("\033[0m")
#define PRINT_WARNING printf("\033[1;33mwarning: ");printf("\033[0m")

bool error_occurred = false;
bool show_debug = false;

bool has_error_occurred(void) {
	return error_occurred;
}

void error (char *err_str) {
  PRINT_ERROR;
  printf("line %d: %s\n", get_current_token()->line, err_str);
  //print_token_type(peek_next_token()->type);
  error_occurred = true;
};

void warn(char *warn_str) {
  PRINT_WARNING;
  printf("line %d: %s\n", get_current_token()->line, warn_str);
}

void file_error(char *err_str) {
	printf("mgcc: ");
	PRINT_ERROR;
	printf("%s\n", err_str);
	error_occurred = true;
}

void debug(char *debug_str) {
	if(show_debug == true) {
		PRINT_DEBUG;
		printf("line %d: %s\n", get_current_token()->line, debug_str);
	}
}

