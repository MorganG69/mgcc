#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/files.h"

char *input_fname;
char *output_fname;

void set_input_fname(char *n) {
  input_fname = n;
}

char *get_input_fname(void) {
  return input_fname;
}

void set_output_fname(char *n) {
  output_fname = malloc(strlen(n));
  memcpy(output_fname, n, (strlen(n) - 2));
  strcat(&output_fname[strlen(output_fname)], ".o");
}

char *get_output_fname(void) {
  return output_fname;
}

void write_output_file(uint8_t *bin, int size) {
  FILE *ofp = fopen(output_fname, "w+");
  fwrite(bin, sizeof(uint8_t), size, ofp);
  fclose(ofp);
  free(output_fname);
}

long get_file_size(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);
    return fsize;
}

FILE *open_input_file(char *fname) {
  FILE *ifp = fopen(fname, "r");

  if(ifp == NULL) {
	  file_error("Input file open error.");
  }
  return ifp;
}

char *read_input_file(FILE *f) {
  long input_fsize = get_file_size(f);
  char *ipbuf = (char *)malloc(input_fsize);
  size_t fs = fread(ipbuf, sizeof(char), input_fsize, f);
  fclose(f);

  if(fs != input_fsize) {
   	file_error("Input file read error."); 
	free(ipbuf);
    exit(0);
  }

  ipbuf[input_fsize] = '\0';
  return ipbuf;
}
