#ifndef FILES_H
#define FILES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void set_input_fname(char *n);
char *get_input_fname(void);
void set_output_fname(char *n);
char *get_output_fname(void);
void write_output_file(uint8_t *bin, int size);
FILE *open_input_file(char *fname);
char *read_input_file(FILE *f);
long get_file_size(FILE *fp);
void file_error(char *err_str);

#endif /* FILES_H */
