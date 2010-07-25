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


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>


/// Maximum length of the token that can be handled.
#define MAX_TOKEN_LEN 64


/// Defines possible tokens types.
enum token_type_t {
  TYPE,                         /**< type declaration */
  SPECIFIER,                    /**< type specifier */
  IDENTIFIER,                   /**< identifier  */
  ARRAY,                        /**< array */
  POINTER,                      /**< pointer */
  FUNCTION,                     /**< function */
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
                          "FUNCTION",
                          "END" };

  return strs[type];
}


/// Token;
struct token_t {
  enum token_type_t type;       /**< type of the token */

  char str[MAX_TOKEN_LEN + 1];  /**< string representation of the token */
  union {
    int size;                   /**< if ::type is #ARRAY then it's the size of
                                 * array (or -1 in case size has not been
                                 * specified)*/
  } info;
};


static int line     = 1;        /**< current line being processed */
static int position = 0;        /**< current position in the line */
static struct token_t token;    /**< current token */


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
  int i;
  char *p = id;
  /* it's guaranteed by the caller that the first character in stdin is a
   * letter */

  while (1) {
    int c = _getchar();
    if (c == EOF) {
      fatal_pos("Unexpected end of file\n");
    } else if (isalnum(c) || c == '_') {
      ++i;
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
    token.str[0] = ';';
    token.str[1] = '\0';
  } else if (isalpha(c)) {
    /* some type, specifier or declarator starts here */
    _ungetchar(c);

    get_id(token.str);

    if (strcmp(token.str, "int")      == 0 ||
        strcmp(token.str, "char")     == 0 ||
        strcmp(token.str, "void")     == 0 ||
        strcmp(token.str, "signed")   == 0 ||
        strcmp(token.str, "unsigned") == 0 ||
        strcmp(token.str, "short")    == 0 ||
        strcmp(token.str, "long")     == 0 ||
        strcmp(token.str, "float")    == 0 ||
        strcmp(token.str, "double")   == 0)
    {
      token.type = TYPE;
    } else if (strcmp(token.str, "const")    == 0 ||
               strcmp(token.str, "volatile") == 0)
    {
      token.type = SPECIFIER;
    } else {
      token.type = IDENTIFIER;
    }
  } else if (c == '[') {
    skip_to_char(']');

    token.type      = ARRAY;
    *token.str      = '\0';
    /* for now all arrays are without size */
    token.info.size = -1;
  } else if (c == '(') {
    skip_to_char(')');
    token.type = FUNCTION;
    *token.str = '\0';
    /* similarly, all the function parameters are ignored for now */
  }
}


int
main(void)
{
  do {
    get_token();
    printf("Token type: %s, source: %s\n", token_to_str(token.type), token.str);

  } while (token.type != END);

  return EXIT_SUCCESS;
}
