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

token_type prev_type;

static int line = 1;
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



bool is_valid_suffix(char s) {
	switch(s) {
		case 'u':
		case 'U':
		case 'l':
		case 'L':
			return true;
		default:
			return false;
	}
}

/*
 * Lex an integer constant in accordance with A2.5.1
 * Returns a pointer to the string including any suffixes, or hex prefixes
 */ 
char *lex_integer_constant(void) {
  size_t ic_len = 0;
  char *ic_start = source_ptr;
  char *ptr = ic_start;
 
  /* Handle hex prefix if present */
  if(*ptr == '0') {
	  if(ptr[1] == 'x') {
		  ptr += 2;
		  ic_len += 2;
	  }
  }

  /* 0-9, a-f, A-F */
  while(is_valid_integer_constant(*ptr)) {
	ptr++;
    ic_len++;
  }

  /* this will accept invalid suffixes such as uuullllll etc but the parser will catch this. */
  while(is_valid_suffix(*ptr)) {
	  ptr++;
	  ic_len++;
  }

  char *ic_str = malloc(ic_len + 1);
  memcpy(ic_str, ic_start, ic_len);
  CONSUME_CHAR(ic_len);
  ic_str[ic_len] = '\0';
 // printf("%s\n", ic_str);	
  return ic_str;
}
  /* This should be done by the parser to handle any suffixes */
  /*
   * Integer constants are considered as:
   * 	octal if they have a leading zero (01234)
   * 	hex if they begin with 0x (0x1234)
   * 	decimal if beginning with non-zero (1234)
 
  if(ic_str[0] == '0') {
	  if(ic_str[1] == 'x') {
	    val = hex_str_to_int(ic_str);
	  } else {
		val = oct_str_to_int(ic_str);
	  }
  } else {
	  val = dec_str_to_int(ic_str);
  }
  // Temp string no longer needed
  free(ic_str);
 
  // The attributes of a token are stored in a void pointer so allocate space for an int and copy in 
  int *val_ptr = malloc(sizeof(int));
  memcpy(val_ptr, &val, sizeof(int));


  return val_ptr;
}
*/

char *lex_string(void) {
  size_t len = 0;
  char *start = source_ptr;
  char *ptr = start;

  /* Handles L"..." string literal */
  if(*ptr == 'L') {
	  len++;
  }

  for(int i = 0; i < MAX_STRING_LITERAL_LEN; i++) {
    if(*ptr == '"' || *ptr == '\0') {
		break;
    }
    len++;
    ptr++;
  }

  char *str = malloc(len+1);
  memcpy(str, start, len);
  str[len] = '\0';
  CONSUME_CHAR(len);
  //printf("str = %s\n", str);
  return str;
}

bool is_character_constant(char c) {
	if(c > 31 && c < 127) {
		return true;
	} else {
		return false;
	}
}

char *lex_character_constant(void) {
	int len = 0;
	char *ptr = source_ptr;
	char *start = ptr;

	/* 
	 * Handles the case of L'x'
	 * Doesn't actually do anything with it, just accepts it with no errors
	 */
	if(*ptr == 'L') {
		len++;
	}

	if(is_character_constant(*ptr)) {
		if(*ptr == '\\') {
			while(*ptr != '\'') {
				len++;
				ptr++;
			}
			
			/* handle the case of '\'' */
			if(ptr[1] == '\'') {
				len++;
			}
		} else {
			len++;
		}
	}	

	char *str = malloc(len);
	memcpy(str, start, len);
	CONSUME_CHAR(len);
//	printf("str = %s\n", str);
	return str;
}

/* Returns DIVIDE if not a comment or UNKNOWN if is a comment */
token_type lex_possible_comment(void) {
	if(source_ptr[1] == '/') {
		while(*source_ptr != '\n') {
			source_ptr++;
		}
		return UNKNOWN;
	} else if(source_ptr[1] == '*') {
		source_ptr += 2;
		while(*source_ptr != '*') {
			if(*source_ptr == '\n') {
				line++;
			}
			if(source_ptr[1] == '*' && source_ptr[2] == '/') {
				source_ptr+=2;
				break;
			}
			source_ptr++;
		}
		return UNKNOWN;
	} else {
		return DIVIDE;
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
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
      t->type = INTEGER_CONST;
      t->attr = (void *)lex_integer_constant();
      break;
 
    case '"':
	  t->type = QUOTE;
	  CONSUME_CHAR(1);
	  break;

    case '\'':
	  t->type = APOSTROPHE;
	  CONSUME_CHAR(1);
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
	  } else {
		  if(source_ptr[1] == '=') {
			  t->type = DIV_ASSIGN;
			  CONSUME_CHAR(2);
		  } else {
			  CONSUME_CHAR(1);
		  }
	  }
      break;

	case '%':
	  if(source_ptr[1] == '=') {
		  t->type = MOD_ASSIGN;
		  CONSUME_CHAR(2);
	  } else {
		  t->type = MOD;
		  CONSUME_CHAR(1);
	  }
	break;

    case '+':
	  switch(source_ptr[1]) {
		  case '+':
			  t->type = INCREMENT;
			  CONSUME_CHAR(2);
		  break;
	      
	      case '=':
			  t->type = ADD_ASSIGN;
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

	case '\t':
	  while(READ_LEX_HEAD == '\t') {
		  CONSUME_CHAR(1);
	  }
	  goto lex_next_token;
	  break;

    case ' ':
      /* skip whitespace */
      while(READ_LEX_HEAD == ' ') {
        CONSUME_CHAR(1);
      }
      goto lex_next_token;
      break;

    case '\n':
      //t->type = NEWLINE;
	  while(READ_LEX_HEAD == '\n') {
	  	inc_line();
	  	CONSUME_CHAR(1);
	  }  
	  goto lex_next_token;
	  break;

    case '\0':
      t->type = END;
      break;

    default:
      /*
	   * if(previous == apostrophe) {
	   *	return valid char const stuff up to ' or ;
	   * } else if(previous == quote) {
	   * 	return valid string stuff up to " or ;
	   * } else if(is_valid_identifer){
	   * 	return identifier;
	   * } else {
	   * 	return unknwon
	   * }
	   */ 
	  if(prev_type == APOSTROPHE) {
		  t->type = CHAR_CONST;
		  t->attr = (void *)lex_character_constant();
	  } else if(prev_type == QUOTE) {
		  t->type = STRING_LITERAL;
		  t->attr = (void *)lex_string();
	  } else if(is_valid_identifier(*source_ptr)) {
		  /* handles the case of L'x' in A2.5.2 */
		  if(source_ptr[0] == 'L' && (source_ptr[1] == '\'' || source_ptr[1] == '\"')) {
			  if(source_ptr[1] == '\'') {
				  t->type = APOSTROPHE;
				  t->attr = (void *)lex_character_constant();
			  } else {
				  t->type = QUOTE;
				  t->attr = (void *)lex_string();
			  }
		  } else {
			  t->attr = (void *)lex_identifier();
			  t->type = match_keyword((char *)t->attr);
		  }
	  } else {
		  t->type = UNKNOWN;
		  CONSUME_CHAR(1);
	  }
	  break;
  }
  t->line = get_line();
//  print_token_type(t->type);
  prev_type = t->type;
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
	
	  case ENUM:
		printf("ENUM\n");
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
      case LBRACK:
        printf("LBRACK\n");
        break;
      case RBRACK:
        printf("RBRACK\n");
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
	  case QUOTE:
		printf("QUOTE\n");
		break;
	  case APOSTROPHE:
		printf("APOSTROPHE\n");
		break;
      case END:
        printf("END\n");
        break;
	  case UNKNOWN:
        printf("UNKNOWN\n");
        break;

	  default:
		printf("ADD VALUE %d TO print_token_type()!\n", t);
    }
}
