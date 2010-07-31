/**
 * @file   cdecl.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sun Jul 25 16:07:11 2010
 *
 * @brief  A program that descrypts C type declarations. Implemented as
 *         an exercise for "Expert C Programming" book.
 *
 * Not very intelligent (most of incorrect declarations are accepted with no
 * errors) program pronouncing C declarations.
 */


#include <assert.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>


/// Maximum length of the token that can be handled.
#define MAX_TOKEN_LEN 64


/// Maximum number of tokens that can be handled.
#define MAX_TOKENS 128


/// Defines possible tokens types.
enum token_type_t {
  TYPE,                         /**< type declaration */
  SPECIFIER,                    /**< type specifier */
  IDENTIFIER,                   /**< identifier  */
  ARRAY,                        /**< array */
  POINTER,                      /**< pointer */
  LBRACE,                       /**< left brace */
  RBRACE,                       /**< right brace */
  END,                          /**< declaration finishied */
};


/**
 * Transforms #token_type_t to its string representation.
 *
 * @param type type
 *
 * @return A string corresponding to a type.
 */
const char *
token_to_str(enum token_type_t type)
{
  static char *strs[] = { "TYPE",
                          "SPECIFIER",
                          "IDENTIFIER",
                          "ARRAY",
                          "POINTER",
                          "LBRACE",
                          "RBRACE",
                          "END" };

  return strs[type];
}


/// Token;
struct token_t {
  enum token_type_t type;       /**< type of the token */


  union {
    char name[MAX_TOKEN_LEN + 1];  /**< string representation of the token */
    int  size;                     /**< if ::type is #ARRAY then it's the size
                                    * of array (or -1 in case size has not been
                                    * specified) */
  } info;
};


static int line     = 1;        /**< current line being processed */
static int position = 0;        /**< current position in the line */
static struct token_t token;    /**< current token */


static struct token_t stack[MAX_TOKENS]; /**< stack of tokens to the left from
                                          * curent one*/
static int head = 0;                     /**< head of the #stack */


/**
 * Prints a message about fatal condition and exits with #EXIT_FAILURE exit
 * code.
 *
 * @param format Printf-like format string.
 */
void
fatal(const char *format, ...)
{
  va_list args;
  va_start(args, format);

  vfprintf(stderr, format, args);

  va_end(args);

  exit(EXIT_FAILURE);
}


/**
 * A variant of fatal() function that additionaly prints current line and
 * position.
 *
 * @param format Printf-like format string.
 */
#define fatal_pos(format, ...) \
  fatal("Line: %d, Position: %d: " format, line, position, #__VA_ARGS__)


/**
 * Pushes a token to the #stack.
 *
 * @param token token
 */
static void
stack_push(const struct token_t *token)
{
  if (head >= MAX_TOKENS) {
    fatal("Too long declaration. Can't proceed.\n");
  }

  stack[head++] = *token;
}


/**
 * Pops a token from the #stack.
 *
 *
 * @return token
 */
static struct token_t
stack_pop(void)
{
  /* can be a programming error actually */
  if (head == 0) {
    fatal("Stack underflow. Invalid declaration.\n");
  }

  return stack[--head];
}


/**
 * Determines whether #stack is empty.
 *
 *
 * @retval true  Stack is empty.
 * @retval false Stack is not empty.
 */
static bool
stack_is_empty(void)
{
  return head == 0;
}


/**
 * Wrapper for getchar() function which keeps #line and #position consistent.
 *
 *
 * @retval EOF  No input left.
 * @retval char A character that had been read.
 */
int
_getchar()
{
  int c = getchar();

  if (c != EOF) {
    ++position;

    if (c == '\n') {
      ++line;
      position = 0;
    }
  }

  return c;
}


/**
 * Wrapper for #ungetc function which keeps #position and #line consistent.
 *
 * @param c Character to return to stdin. It's not a good idea for it to be
 *          '\n' as then #line and #position would contain incorrect values.
 */
void
_ungetchar(int c)
{
  if (ungetc(c, stdin) != c) {
    fatal("Unrecoverable IO error occurred.\n");
  }
  --position;
}


/**
 * Skip all the space characters in stdin.
 *
 */
void
skip_spaces(void)
{
  int c;

  while ((c = getchar()) != EOF) {
    if (!isspace(c)) {
      _ungetchar(c);
      return;
    }
  }

  fatal_pos("Unexpected end of file\n");
}


/**
 * Reads identifier from @e stdin.
 *
 * @param[out] id A place to store the result. At most #MAX_TOKEN_LEN + 1 bytes
 *                will be used.
 *
 * @return A pointer to #id parameter.
 */
char *
get_id(char *id)
{
  int i = 0;
  char *p = id;
  /* it's guaranteed by the caller that the first character in stdin is a
   * letter */

  while (1) {
    int c = _getchar();
    if (c == EOF) {
      fatal_pos("Unexpected end of file\n");
    } else if (isalnum(c) || c == '_') {
      if (i++ >= MAX_TOKEN_LEN) {
        fatal_pos("Too long token occured. Can't proceed.\n");
      }

      *p++ = c;
    } else {
      _ungetchar(c);
      *p = '\0';
      return id;
    }
  }
}


/**
 * Skips all the input until specifies character is occured.
 *
 * @param c A character to wait for.
 */
void
skip_to_char(char end)
{
  while (1) {
    int c = _getchar();
    if (c == EOF) {
      fatal_pos("Unexpected end of file\n");
    } else if (c == end) {
      return;
    }
  }
}


/**
 * Reads token from @e stdin and stores it in #token.
 *
 */
void
get_token()
{
  skip_spaces();

  int c = getchar();
  ++position;

  if (c == EOF) {
    fatal_pos("Unexpected end of file\n");
  } else if (c == ';') {
    token.type   = END;
  } else if (isalpha(c)) {
    /* alias */
    char *name = token.info.name;

    /* some type, specifier or declarator starts here */
    _ungetchar(c);

    get_id(name);

    if (strcmp(name, "int")      == 0 ||
        strcmp(name, "char")     == 0 ||
        strcmp(name, "void")     == 0 ||
        strcmp(name, "signed")   == 0 ||
        strcmp(name, "unsigned") == 0 ||
        strcmp(name, "short")    == 0 ||
        strcmp(name, "long")     == 0 ||
        strcmp(name, "float")    == 0 ||
        strcmp(name, "double")   == 0)
    {
      token.type = TYPE;
    } else if (strcmp(name, "const")    == 0 ||
               strcmp(name, "volatile") == 0)
    {
      token.type = SPECIFIER;
    } else {
      token.type = IDENTIFIER;
    }
  } else if (c == '[') {
    skip_to_char(']');

    token.type      = ARRAY;
    /* for now all arrays are without size */
    token.info.size = -1;
  } else if (c == '(') {
    token.type = LBRACE;
  } else if (c == ')') {
    token.type = RBRACE;
  } else if (c == '*') {
    token.type = POINTER;
  }
}


/**
 * Pronounces single token.
 *
 * @param token A token to pronounce.
 */
static void
pronounce_token(const struct token_t *token)
{
  switch (token->type) {
  case TYPE:
    printf("%s ", token->info.name);
    break;
  case SPECIFIER:
    if (strcmp(token->info.name, "const") == 0) {
      printf("read-only ");
    } else {
      printf("%s ", token->info.name);
    }
    break;
  case ARRAY:
    printf("array of ");
    break;
  case POINTER:
    printf("pointer to ");
    break;
  case LBRACE:
    printf("function returning ");
    break;
  case RBRACE:
  case END:
    /* just keeping silence */
    break;
  default:
    assert(0);
  }
}


/**
 * Tries to "pronounce" @e C declaration read from @e stdin in
 * human-understandable form.
 *
 */
static void
pronounce(void)
{
  do {
    get_token();
    stack_push(&token);
  } while (token.type != IDENTIFIER);

  /* popping identifier */
  stack_pop();

  printf("%s is ", token.info.name);

  /* whether input to the right of identifier is finished */
  bool right_finished = false;
  bool left_finished  = false;

  while (true) {
    /* right pass */
    if (!right_finished) {
      do {
        get_token();
        pronounce_token(&token);

        /* skiping function argument */
        if (token.type == LBRACE) {
          do {
            get_token();
          } while (token.type != RBRACE);

          /* faking token type to proceed to the right */
          token.type = LBRACE;
        }
      } while (token.type != END &&
               token.type != RBRACE);

      /* we reached the end of input */
      if (token.type == END) {
        right_finished = true;
      }
    }

    /* left pass */
    if (!left_finished) {
      struct token_t left_token;
      do {
        left_token = stack_pop();

        /* left brace should not be pronounced as "function" */
        if (left_token.type != LBRACE) {
          pronounce_token(&left_token);
        }
      } while (!stack_is_empty() &&
               left_token.type != LBRACE);

      if (stack_is_empty()) {
        left_finished = true;
      }
    }

    if (left_finished && right_finished) {
      break;
    }
  }

  printf("\n");
}


int
main(void)
{
  pronounce();

  return EXIT_SUCCESS;
}
