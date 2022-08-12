//
// Created by Morgan Greenhill on 01/04/2022.
//

#ifndef MGG_8_LEX_H
#define MGG_8_LEX_H

#include <stdbool.h>
#include <stdlib.h>

#define MAX_STRING_LITERAL_LEN 512
#define EXPECT_TOKEN(t) (get_current_token()->type == (t))
#define NEW_TOKEN calloc(1, sizeof(token))

typedef int token_type;
typedef int operation;
/* Keywords */
/** DO NOT MOVE THESE DEFINITIONS */
enum {
  /* storage-class-specifier */
  AUTO, REGISTER, STATIC, EXTERN, TYPEDEF,
  
  /* type-specifier */
  VOID, CHAR, SHORT, INT, LONG, FLOAT, DOUBLE, SIGNED, UNSIGNED,
  STRUCT, UNION, ENUM,
  
  /* type-qualifier */
  CONST, VOLATILE,

  /* statement keywords */
  SWITCH, CASE, DEFAULT,
  IF, ELSE,
  WHILE, DO, FOR,
  GOTO, CONTINUE, BREAK, RETURN,

  /* In precedence order. C book page 53 */
  LBRACK, RBRACK, LBRACE, RBRACE, LPAREN, RPAREN, DOT, ARROW, /* Highest */
  INCREMENT, DECREMENT,
  ASTERISK, DIVIDE,
  ADD, SUB,
  LSHIFT, RSHIFT,
  GREATER, GTEQ, LESS, LTEQ,
  EQUAL, NOTEQ,
  AMPER,
  CARET,
  PIPE,
  LOGAND,
  LOGOR,
  /*
   * Ternary operator
   */
  ASSIGN, ADD_ASSIGN, SUB_ASSIGN, MUL_ASSIGN, DIV_ASSIGN, AMPER_ASSIGN,
  CARET_ASSIGN, PIPE_ASSIGN, LSHIFT_ASSIGN, RSHIFT_ASSIGN,
  COMMA, /* Lowest */

  TILDE,
  NOT,

  /* Primary expressions */
  IDENTIFIER,
  INTEGER_CONST,
  CHAR_CONST,
  STRING_LITERAL,

  STRING,
  SEMI_COLON,
  COLON,
  NEWLINE,
  APOSTROPHE,
  QUOTE,
  END,

  UNKNOWN
};

#define NUM_KEYWORDS 31
static const char *keywords[NUM_KEYWORDS] = {
	"auto", "register", "static", "extern", "typedef",
	"void", "char", "short", "int", "long", "float", "double", "signed", "unsigned",
	"struct", "union", "enum",
	"const", "volatile",
	"switch", "case", "default",
	"if", "else",
	"while", "do", "for",
	"goto", "continue", "break", "return"
};

#define MAX_TOK_LEN 32

typedef struct source_position source_pos;
struct source_position {
  int line;
  int col;
};

typedef struct token_struct token;
struct token_struct {
  token_type type;
  void *attr; /* Is interpreted by the type of the token */
  int line;
  bool prefix;
  token *next;
};

typedef struct {
  char *source;
  char *read_head;
} lexer;

int get_line(void);
void init_lex(char *input_file);
token *lex_token(void);
token *lex_translation_unit(void);
void print_token_type(token_type t);
void consume_token(void);
token *get_current_token(void); 
token *peek_next_token(void);
token *peek_2nd_token(void);

#endif //MGG_8_LEX_H
