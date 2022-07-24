//
// Created by Morgan Greenhill on 25/03/2022.
//
#include "../inc/files.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../inc/lex.h"
#include "../inc/error.h"


#define READ_LEX_HEAD *source_ptr
#define CONSUME_CHAR(n) source_ptr+=(n)


static int line = 0;
static char *source;
static char *source_ptr;

int get_line(void) {
  return line;
}

void inc_line(void) {
  line++;
}

bool is_valid_identifier(char n) {
  if(((n >= 'a') && (n <= 'z')) || ((n >= 'A') && (n <= 'Z')) || (n == '_') || ((n >= '0') && (n <= '9'))) {
    return true;
  } else {
    return false;
  }
}

bool is_valid_integer_constant(char n) {
  if(((n >= '0') && (n <= '9')) || ((n >= 'A') && (n <= 'F')) || ((n >= 'a') && (n <= 'f'))) {
    return true;
  } else {
    return false;
  }
}

char *lex_identifier(void) {
  size_t id_len = 0;
  char *id_start = source_ptr;

  char *ptr = id_start;
  while(is_valid_identifier(*ptr)) {
    ptr++;
    id_len++;
  }

  char *id_str = malloc(id_len);
  memcpy(id_str, id_start, id_len);
  id_str[id_len] = '\0';
  CONSUME_CHAR(id_len);

  return id_str;
}

token_type match_keyword(char *s) {
  size_t id_len = strlen(s);
  for(int i = 0; i < NUM_KEYWORDS; i++) {
    if(id_len == strlen(keywords[i])) {
      if(!strcmp(s, keywords[i])) {
        return i;
      }
    }
  }
  return IDENTIFIER;
}

char *lex_integer_constant(void) {
  size_t ic_len = 0;
  char *ic_start = source_ptr;
  char *ptr = ic_start;

  if(READ_LEX_HEAD == '0') {
    if(*(source_ptr + 1) == 'x') {
      ic_len+=2;
      ptr = ic_start+2;
    }
  }

  while(is_valid_integer_constant(*ptr)) {
    ptr++;
    ic_len++;
  }
  char *ic_str = malloc(ic_len);
  memcpy(ic_str, ic_start, ic_len);
  CONSUME_CHAR(ic_len);
  ic_str[ic_len] = '\0';

  return ic_str;
}

char *lex_string(void) {
  size_t len = 0;
  char *start = source_ptr;
  char *ptr = start;
	
  for(int i = 0; i < MAX_STRING_LITERAL_LEN; i++) {
    if(*ptr == '"' || *ptr == '\0') {
      break;
    }
    len++;
    ptr++;
  }

  char *str = malloc(len);
  memcpy(str, start, len);
  CONSUME_CHAR(len);

  return str;
}

/* Returns DIVIDE if not a comment or UNKNOWN if is a comment */
token_type lex_possible_comment(void) {
  if(*(source_ptr + 1) != '/' && *(source_ptr + 1) != '*') {
    //CONSUME_CHAR(1);
	  if(*(source_ptr + 1) == '=') {
		  CONSUME_CHAR(2);
		  return DIV_ASSIGN;
	  } else {
	  	return DIVIDE;
	  }
  } else {
    if(*(source_ptr + 1) == '/') {
      while(*source_ptr != '\n') {
        source_ptr++;
      }
    } else {
      while(*source_ptr != '^') {
        if(*source_ptr == '\n') {
          inc_line();
        }
        if(*source_ptr == '*') {
          source_ptr++;
          if(*source_ptr == '/') {
            source_ptr++;
            break;
          } else {
            source_ptr--;
          }
        }
        source_ptr++;
      }
    }
    return UNKNOWN;
  }
}

/*
 * C99 Section 6.4
 * If the input stream has been parsed into preprocessing tokens up to a given character, the next preprocessing token is the longest sequence of characters that could constitute a preprocessing token.
 * The maximal munch principle.
 */
token *lex_token(void) {
  token *t = NEW_TOKEN;
  lex_next_token:
  switch(READ_LEX_HEAD) { 
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z': case '_':
	  t->attr = (void *)lex_identifier();
      t->type = match_keyword((char *)t->attr);
	  break;

    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
      t->type = INTEGER_CONST;
      t->attr = (void *)lex_integer_constant();
      break;
	
    case '"':
      t->type = STRING_LITERAL;
      CONSUME_CHAR(1);
      t->attr = (void *)lex_string();
      break;

    case '\'':
      t->type = CHAR_CONST;
      CONSUME_CHAR(1);
	
      t->attr = malloc(1);
      memcpy(t->attr, source_ptr, 1);

      CONSUME_CHAR(1);
      if(READ_LEX_HEAD != '\'') {
        // Still continues after this but the code does not compile
        error("Expected ' at the end of character constant.");
      } else {
        CONSUME_CHAR(1);
      }
      break;

	case '[':
      t->type = LBRACK;
      CONSUME_CHAR(1);
      break;

    case ']':
      t->type = RBRACK;
      CONSUME_CHAR(1);
      break;


    case '{':
      t->type = LBRACE;
      CONSUME_CHAR(1);
      break;

    case '}':
      t->type = RBRACE;
      CONSUME_CHAR(1);
      break;

    case '(':
      t->type = LPAREN;
      CONSUME_CHAR(1);
      break;

    case ')':
      t->type = RPAREN;
      CONSUME_CHAR(1);
      break;

    case '*':
   	  if(source_ptr[1] == '=') {
		  t->type = MUL_ASSIGN;
		  CONSUME_CHAR(2);
	  } else {
		 t->type = ASTERISK;
		 CONSUME_CHAR(1);
	  }
      break;

	case '~':
	  t->type = TILDE;
	  CONSUME_CHAR(1);
	  break;


    case '/':
      t->type = lex_possible_comment();
      if(t->type == UNKNOWN) {
        goto lex_next_token;
      }
      break;

    case '+':
	  switch(source_ptr[1]) {
		  case '+':
			  t->type = INCREMENT;
			  CONSUME_CHAR(2);
			  break;
		  
		  default:
			  t->type = ADD;
			  CONSUME_CHAR(1);
			  break;
	  }
	  break;

    case '-':
	  switch(source_ptr[1]) {
		  case '-':
			  t->type = DECREMENT;
			  CONSUME_CHAR(2);
			  break;

		  case '=':
			  t->type = SUB_ASSIGN;
			  CONSUME_CHAR(2);
			  break;

		  case '>':
			  t->type = ARROW;
			  CONSUME_CHAR(2);
			  break;
			  
		  default:
			  t->type = SUB;
			  CONSUME_CHAR(1);
			  break;
	  }
	  break;

	case '^':
	  if(source_ptr[1] == '=') {
		  t->type = CARET_ASSIGN;
		  CONSUME_CHAR(2);
	  } else {
		 t->type = CARET;
		 CONSUME_CHAR(1);
	  }
	  break;

    case '>':
      switch(source_ptr[1]) {
        case '>':
			if(source_ptr[2] == '=') {
				t->type = LSHIFT_ASSIGN;
				CONSUME_CHAR(3);
			} else {
				t->type = LSHIFT;
				CONSUME_CHAR(2);
			}
          break;

        case '=':
          t->type = GTEQ;
          CONSUME_CHAR(2);
          break;

        default:
          t->type = GREATER;
          CONSUME_CHAR(1);
          break;
      }
      break;

    case '<':
      switch(source_ptr[1]) {
        case '<':
			if(source_ptr[2] == '=') {
				t->type = RSHIFT_ASSIGN;
				CONSUME_CHAR(3);
			} else {
				t->type = RSHIFT;
          		CONSUME_CHAR(2);
			}
		  break;

        case '=':
          t->type = LTEQ;
          CONSUME_CHAR(2);
          break;

        default:
          t->type = LESS;
          CONSUME_CHAR(1);
          break;
      }
      break;

    case '=':
      if(source_ptr[1] == '=') {
        t->type = EQUAL;
        CONSUME_CHAR(2);
      } else {
        t->type = ASSIGN;
        CONSUME_CHAR(1);
      }
      break;

    case '!':
      if(source_ptr[1] == '=') {
        t->type = NOTEQ;
        CONSUME_CHAR(2);
      } else {
        t->type = NOT;
        CONSUME_CHAR(1);
      }
      break;

    case '&':
	  switch(source_ptr[1]) {
		  case '&':
		  	t->type = LOGAND;
			CONSUME_CHAR(2);
			break;

		  case '=':
			t->type = AMPER_ASSIGN;
			CONSUME_CHAR(2);
			break;

		  default:
			t->type = AMPER;
			CONSUME_CHAR(1);
	  }
      break;

    case '|':
   	  switch(source_ptr[1]) {
		  case '|':
		  	t->type = LOGOR;
			CONSUME_CHAR(2);
			break;

		  case '=':
			t->type = PIPE_ASSIGN;
			CONSUME_CHAR(2);
			break;

		  default:
			t->type = PIPE;
			CONSUME_CHAR(1);
	  }
     break;

    case ',':
      t->type = COMMA;
      CONSUME_CHAR(1);
      break;

    case ':':
      t->type = COLON;
      CONSUME_CHAR(1);
      break;

    case ';':
      t->type = SEMI_COLON;
      CONSUME_CHAR(1);
      break;

    case ' ':
      /* skip whitespace */
      while(READ_LEX_HEAD == ' ') {
        CONSUME_CHAR(1);
      }
      goto lex_next_token;
      break;

    case '\n':
      t->type = NEWLINE;
      inc_line();
      CONSUME_CHAR(1);
      break;

    case '\0':
      t->type = END;
      break;

    default:
      t->type = UNKNOWN;
	  CONSUME_CHAR(1);
	  break;
  }
  return t;
}

token *current_token;

void init_lex(char *input_file) {
	FILE *ifp = open_input_file(input_file);
	
	if(ifp != NULL) {
		source = read_input_file(ifp);
		source_ptr = source;
	}
}

token *head;
token *tail;

token *lex_translation_unit(void) {
    token *t = lex_token();
    head = t;
    tail = t;

    while(t->type != END) {
        tail->next = t;
        tail = t;
        t = lex_token();
    }
    tail->next = t;
    tail = t;
	current_token = head;
    return head;
}

void consume_token(void) {
	current_token = current_token->next;
}

token *get_current_token(void) {
	return current_token;
}

token *peek_next_token(void) {
	return current_token->next;
}

token *peek_2nd_token(void) {
	return current_token->next->next;
}


/*
int main(void) {

  token *t = lex_token();
  while(t->type != END) {
    if(t->attr != NULL) {
      printf("%s\n", (char *)t->attr);
    }
    free(t);
    t = lex_token();
  }



  fclose(ifp);
}
 */

void print_token_type(token_type t) {
    switch(t) {
      case INT:
        printf("INT\n");
        break;
      case BREAK:
        printf("BREAK\n");
        break;
      case ELSE:
        printf("ELSE\n");
        break;
      case SWITCH:
        printf("SWITCH\n");
        break;
      case CASE:
        printf("CASE\n");
        break;
      case CHAR:
        printf("CHAR\n");
        break;
      case RETURN:
        printf("RETURN\n");
        break;
      case FOR:
        printf("FOR\n");
        break;
      case VOID:
        printf("VOID\n");
        break;
      case DEFAULT:
        printf("DEFAULT\n");
        break;
      case GOTO:
        printf("GOTO\n");
        break;
      case IF:
        printf("IF\n");
        break;
      case WHILE:
        printf("WHILE\n");
        break;

      case LBRACE:
        printf("LBRACE\n");
        break;
      case RBRACE:
        printf("RBRACE\n");
        break;
      case LPAREN:
        printf("LPAREN\n");
        break;
      case RPAREN:
        printf("RPAREN\n");
        break;

      case ASTERISK:
        printf("ASTERISK\n");
        break;
      case DIVIDE:
        printf("DIVIDE\n");
        break;

      case ADD:
        printf("ADD\n");
        break;
      case SUB:
        printf("SUB\n");
        break;

      case LSHIFT:
        printf("LSHIFT\n");
        break;
      case RSHIFT:
        printf("RSHIFT\n");
        break;

      case GREATER:
        printf("GREATER\n");
        break;
      case GTEQ:
        printf("GTEQ\n");
        break;
      case LESS:
        printf("LESS\n");
        break;
      case LTEQ:
        printf("LTEQ\n");
        break;

      case EQUAL:
        printf("EQUAL\n");
        break;
      case NOTEQ:
        printf("NOTEQ\n");
        break;

      case AMPER:
        printf("AMPER\n");
        break;
      case CARET:
        printf("CARET\n");
        break;
      case PIPE:
        printf("PIPE\n");
        break;
      case LOGAND:
        printf("LOGAND\n");
        break;
      case LOGOR:
        printf("LOGOR\n");
        break;

      case ASSIGN:
        printf("ASSIGN\n");
        break;


      case IDENTIFIER:
        printf("IDENTIFIER\n");
        break;
      case INTEGER_CONST:
        printf("INTEGER_CONST\n");
        break;
      case CHAR_CONST:
        printf("CHAR_CONST\n");
        break;
      case NEWLINE:
        printf("NEWLINE\n");
        break;
      case STRING_LITERAL:
        printf("STRING_LITERAL\n");
        break;

	  case STRING:
		printf("STRING\n");
		break;

      case INCREMENT:
        printf("INCREMENT\n");
        break;
      case DECREMENT:
        printf("DECREMENT\n");
        break;



      case SEMI_COLON:
        printf("SEMI_COLON\n");
        break;
      case COLON:
        printf("COLON\n");
        break;
      case COMMA:
        printf("COMMA\n");
        break;
      case END:
        printf("END\n");
        break;
      case UNKNOWN:
        printf("UNKNOWN\n");
        break;
    }
}
