#include "parse.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Initial capacity for the resizable array used to store
// statements in a compound statement.
#define INITIAL_CAPACITY 5

//////////////////////////////////////////////////////////////////////
// Input tokenization

// Current line we're parsing, starting from 1 like most editors.
static int lineCount = 1;

/** Print a syntax error message, with a line number and exit. */
static void syntaxError()
{
  fprintf( stderr, "line %d: syntax error\n", lineCount );
  exit( EXIT_FAILURE );
}

bool parseToken( char *token, FILE *fp )
{
  int ch;

  // Skip whitespace and comments.
  while ( isspace( ch = fgetc( fp ) ) || ch == '#' ) {
    // If we hit the comment characer, skip the whole line.
    if ( ch == '#' )
      while ( ( ch = fgetc( fp ) ) != EOF && ch != '\n' )
        ;

    if ( ch == '\n' )
      lineCount++;
  }
    
  if ( ch == EOF )
    return false;

  // Record the character we've read and keep up with the token length.
  int len = 0;
  token[ len++ ] = ch;

  // Handle punctuation.  These get special treatment so we can
  // unget as a single character, without having to read and unget
  // whitespace characters afterward.
  if ( ch == '}' || ch == ';' || ch == ')' ) {
    token[ len++ ] = '\0';
    return true;
  }

  // Handle non-quoted words.
  if ( ch != '"' ) {
    while ( ( ch = fgetc( fp ) ) != EOF && !isspace( ch ) &&
            ch != '{' && ch != '}' && ch != '"' && ch != '#' ) {
      // Complain if the token is too long.
      if ( len >= MAX_TOKEN ) {
        fprintf( stderr, "line %d: token too long\n", lineCount );
        exit( EXIT_FAILURE );
      }

      token[ len++ ] = ch;
    }

    // We had to read one character too far to find the end of the token.
    // put it back.
    if ( ch != EOF )
      ungetc( ch, fp );

    token[ len++ ] = '\0';
    return true;
  }

  // Most interesting case, handle strings.

  // Is the next character escaped.
  bool escape = false;

  // Keep reading until we hit the matching close quote.
  while ( ( ch = fgetc( fp ) ) != '"' || escape ) {
    // Error conditions
    if ( ch == EOF || ch == '\n' ) {
      fprintf( stderr, "line %d: %s while reading parsing string literal.\n",
               lineCount, ch == EOF ? "EOF" : "newline" );
      exit( EXIT_FAILURE );
    }
      
    // On a backslash, we just enable escape mode.
    if ( !escape && ch == '\\' ) {
      escape = true;
    } else {
      // Interpret escape sequences if we're in escape mode.
      if ( escape ) {
        switch ( ch ) {
        case 'n':
          ch = '\n';
          break;
        case 't':
          ch = '\t';
          break;
        case '"':
          ch = '"';
          break;
        case '\\':
          ch = '\\';
          break;
        default:
          fprintf( stderr, "line %d: Invalid escape sequence \"\\%c\"\n",
                   lineCount, ch );
          exit( EXIT_FAILURE );
        }
        escape = false;
      }

      // Complain if this string, with the eventual close quote, is too long.
      if ( len + 1 >= MAX_TOKEN ) {
        fprintf( stderr, "line %d: token too long\n", lineCount );
        exit( EXIT_FAILURE );
      }
      token[ len++ ] = ch;
    }
  }
    
  // Store the closing quote and the terminator and return.
  token[ len++ ] = '"';
  token[ len++ ] = '\0';
  return token;
}

/** Called when we expect another token on the input.  This function
    parses the token and exits with an error if there isn't one.
    @param storage for the next token, with capacity for at least
    MAX_TOKEN characters.  This buffer will be modified by the parse
    function as it reads additional tokens.
    @param fp file tokens should be read from.
    @return a copy of the pointer to the tok buffer, so this function
    can be used as a parameter to other parsing calls.
*/
static char *expectToken( char *tok, FILE *fp )
{
  if ( !parseToken( tok, fp ) )
    syntaxError();
  return tok;
}

/** Called when the next token, must be a particular value,
    target.  Prints an error message and exits if it's not.
    @param target string that the next token should match.
    @param fp file tokens should be read from.
*/
static void requireToken( char const *target, FILE *fp )
{
  char tok[ MAX_TOKEN + 1 ];
  if ( strcmp( expectToken( tok, fp ), target ) != 0 )
    syntaxError();
}

/** Return true if the given token is a legal number in our language (i.e.
    if it parses with sscanf using %lf
    @param tok token parsed from the input.
    @return true if the given token is a legal numeric value.
*/
static bool isNumber( char const *tok )
{
  double dummy;
  int pos;
  
  // See if the whole token parses as a double
  return sscanf( tok, "%lf%n", &dummy, &pos ) == 1 &&
    pos == strlen( tok );
}

/** Return true if the given string is a legal identifier name
    in the langauge.
    @param tok token parsed from the input.
    @return true if the given token is a legal identifier name.
*/
static bool isIdentifier( char const *tok )
{
  // Make sure the first character is legal.
  if ( !isalpha( tok[ 0 ] ) && tok[ 0 ] != '_' )
    return false;

  // Then, check the rest.
  for ( int i = 1; tok[ i ]; i++ )
    if ( !isalnum( tok[ i ] ) && tok[ i ] != '_' )
      return false;

  // Make sure it's not too long
  if ( strlen( tok ) > MAX_IDENT_LEN )
    return false;

  // And, make sure it doesn't match a reserved word.
  if ( strcmp( tok, "if" ) == 0 ||
       strcmp( tok, "while" ) == 0 ||
       strcmp( tok, "print" ) == 0 )
    return false;

  return true;
}

/** Return true if the given string is a binary operator in our language.
    @param tok token parsed from the input.
    @return true if the given token matches one of the binary operator.

static bool isBinaryOperator( char const *tok )
{
  return strcmp( tok, "+" ) == 0 ||
    strcmp( tok, "-" ) == 0 ||
    strcmp( tok, "*" ) == 0 ||
    strcmp( tok, "/" ) == 0 ||
    strcmp( tok, "&&" ) == 0 ||
    strcmp( tok, "||" ) == 0 ||
    strcmp( tok, "<" ) == 0 ||
    strcmp( tok, "==" ) == 0;
}
*/

/**
  Returns true if the token is an arithmetic operator
  of high precedence. 
  
  @param *tok a pointer to the token
  @return true if the token is an arithmetic operator of high precedence
*/
static bool isHiArithOperator( char const *tok )
{
  return 
    strcmp( tok, "*" ) == 0 ||
    strcmp( tok, "/" ) == 0;
}

/**
  Returns true if the token is an arithmetic operator
  of low precedence. 
  
  @param *tok a pointer to the token
  @return true if the token is an arithmetic operator of low precedence
*/
static bool isLowArithOperator( char const *tok )
{
  return 
    strcmp( tok, "+" ) == 0 ||
    strcmp( tok, "-" ) == 0;
}
/** Parse a building block for a larger expression, either a literal, a
    variable, or an expression inside parentheses.
    @param tok next token from the input.
    @param fp file subsequent tokens are being read from.
    @return the expression object constructed from the input.
*/
static Expr *parseTerm( char *tok, FILE *fp )
{
  // Create a literal token for a quoted string, without the quotes.
  if ( tok[ 0 ] == '"' ) {
    // Same as above, make a dynamically allocated copy of the string inside
    // the quotes and make a new literal expression containing it.
    int len = strlen( tok );
    char *str = (char *) malloc( len - 1 );
    strncpy( str, tok + 1, len - 2 );
    str[ len - 2 ] = '\0';
    return makeLiteral( str );
  } else if ( isNumber( tok ) ) {
    // Create a literal token for anything that looks like a number.
    char *str = (char *) malloc( strlen( tok ) + 1 );
    return makeLiteral( strcpy( str, tok ) );
  } else if ( strcmp(tok, "(") == 0) {
    Expr *paren = parseExpr(expectToken(tok, fp),fp);
    requireToken(")", fp);
    return paren;
  } else if (isIdentifier(tok)){
    char name[MAX_IDENT_LEN + 1];
    strcpy(name, tok);
    return makeVariable(name);
  } else
    syntaxError();
  

  // Not reached.
  return NULL;
}

/**
  Parse the expression with a high arithmetic operator.
  
  @param *tok a pointer to the token
  @param *fp a pointer to the file stream
*/
Expr *parseHiArith( char *tok, FILE *fp ) {
  Expr *left = parseTerm( tok, fp );
  
  // See if there's another oprator after this one.
  char op[ MAX_TOKEN + 1 ];
  while ( isHiArithOperator( expectToken( op, fp ) ) ) {
    // Parse the right-hand operand.
    Expr *right = parseTerm( expectToken( tok, fp ), fp );

    // Create the right type of expression, based on what binary
    // operator it is.
    if ( strcmp( op, "*" ) == 0 )
      left = makeProduct( left, right );
    if ( strcmp( op, "/" ) == 0 )
      left = makeQuotient( left, right );
  }

  // To end an expression, the next token must be ; or )

  // Code that called us is going to expect to see this token, so we
  // need to put it back on the input.
  if (op[0] == '&' || op[0] == '|' || op[0] == '=')
    ungetc(op[0], fp);
  ungetc( op[ 0 ], fp );
  return left;

}

/**
  Parse an expression with the low arithmetic operator.
  
  @param *tok a pointer to the token
  @param *fp a pointer to the file stream
*/
Expr *parseLowArith( char *tok, FILE *fp ) {
  Expr *left = parseHiArith( tok, fp );
  
  // See if there's another oprator after this one.
  char op[ MAX_TOKEN + 1 ];
  while ( isLowArithOperator( expectToken( op, fp ) ) ) {
    // Parse the right-hand operand.
    Expr *right = parseHiArith( expectToken( tok, fp ), fp );

    // Create the right type of expression, based on what binary
    // operator it is.
    if ( strcmp( op, "+" ) == 0 )
      left = makeSum( left, right );
    if ( strcmp( op, "-" ) == 0 )
      left = makeDifference( left, right );
  }

  // To end an expression, the next token must be ; or )

  // Code that called us is going to expect to see this token, so we
  // need to put it back on the input.
  if (op[0] == '&' || op[0] == '|' || op[0] == '=')
    ungetc(op[0], fp);
  ungetc( op[ 0 ], fp );
  return left;

}

/**
  Parse an expression with the comparison operator.

  @param *tok a pointer to the token
  @param *fp a pointer to the file stream
*/
Expr *parseComp( char *tok, FILE *fp ) {
  Expr *left = parseLowArith( tok, fp );
  
  // See if there's another oprator after this one.
  char op[ MAX_TOKEN + 1 ];
  while ( strcmp( expectToken( op, fp ), "<" ) == 0 ) {
    // Parse the right-hand operand.
    Expr *right = parseLowArith( expectToken( tok, fp ), fp );

    // Create the right type of expression, based on what binary
    // operator it is.
    if ( strcmp( op, "<" ) == 0 )
      left = makeLess( left, right );
  }

  // To end an expression, the next token must be ; or )

  // Code that called us is going to expect to see this token, so we
  // need to put it back on the input.
  if (op[0] == '&' || op[0] == '|' || op[0] == '=')
    ungetc(op[0], fp);
  ungetc( op[ 0 ], fp );
  return left;

}

/** 
  Parse an expression evaluating equivalency.

  @param *tok a pointer to the token
  @param *fp a pointer to the file stream
*/
Expr *parseEquals( char *tok, FILE *fp ) {
  Expr *left = parseComp( tok, fp );
  
  // See if there's another oprator after this one.
  char op[ MAX_TOKEN + 1 ];
  while ( strcmp( expectToken( op, fp ), "==" ) == 0 ) {
    // Parse the right-hand operand.
    Expr *right = parseComp( expectToken( tok, fp ), fp );

    // Create the right type of expression, based on what binary
    // operator it is.
    if ( strcmp( op, "==" ) == 0 )
      left = makeEquals( left, right );
  }

  // To end an expression, the next token must be ; or )

  // Code that called us is going to expect to see this token, so we
  // need to put it back on the input.
  if (op[0] == '&' || op[0] == '|' || op[0] == '=')
    ungetc(op[0], fp);
  ungetc( op[ 0 ], fp );
  return left;

}

/**
  Parse an expression with the AND operator.

  @param *tok a pointer to the token
  @param *fp a pointer to the file stream
*/
Expr *parseAnd( char *tok, FILE *fp ) {
  Expr *left = parseEquals( tok, fp );
  
  // See if there's another oprator after this one.
  char op[ MAX_TOKEN + 1 ];
  while ( strcmp( expectToken( op, fp ), "&&" ) == 0 ) {
    // Parse the right-hand operand.
    Expr *right = parseEquals( expectToken( tok, fp ), fp );

    // Create the right type of expression, based on what binary
    // operator it is.
    if ( strcmp( op, "&&" ) == 0 )
      left = makeAnd( left, right );
  }

  // To end an expression, the next token must be ; or )

  // Code that called us is going to expect to see this token, so we
  // need to put it back on the input.
  if (op[0] == '&' || op[0] == '|' || op[0] == '=')
    ungetc(op[0], fp);
  ungetc( op[ 0 ], fp );
  return left;

}

/**
  Parse the expression and call the appropriate method
  depending on operator precedence.

  @param *tok a pointer to the token
  @param *fp a pointer to the file stream
*/
Expr *parseExpr( char *tok, FILE *fp )
{
  // Parse the expression, or just the left-hand operatnd of a longer
  // expression.
  Expr *left = parseAnd( tok, fp );
  
  // See if there's another oprator after this one.
  char op[ MAX_TOKEN + 1 ];
  while ( strcmp( expectToken( op, fp ), "||" ) == 0 ) {
    // Parse the right-hand operand.
    Expr *right = parseAnd( expectToken( tok, fp ), fp );

    // Create the right type of expression, based on what binary
    // operator it is.
    if ( strcmp( op, "||" ) == 0 )
      left = makeOr( left, right );
  }

  // To end an expression, the next token must be ; or )
  if ( strcmp( op, ";" ) != 0 && strcmp( op, ")" ) != 0 )
    syntaxError();

  // Code that called us is going to expect to see this token, so we
  // need to put it back on the input.
  ungetc( op[ 0 ], fp );
  return left;
}

Stmt *parseStmt( char *tok, FILE *fp )
{
  // Handle compound statements
  if ( strcmp( tok, "{" ) == 0 ) {
    int len = 0;
    int cap = INITIAL_CAPACITY;
    Stmt **stmtList = (Stmt **) malloc( cap * sizeof( Stmt * ) );

    // Keep parsing statements until we hit the closing curly bracket.
    while ( strcmp( expectToken( tok, fp ), "}" ) != 0 ) {
      if ( len >= cap ) {
        cap *= 2;
        stmtList = (Stmt **) realloc( stmtList, cap * sizeof( Stmt * ) );
      }
      stmtList[ len++ ] = parseStmt( tok, fp );
    }

    return makeCompound( stmtList, len );
  }

  if (isIdentifier(tok)) {
    char name[ MAX_IDENT_LEN + 1 ];
    strcpy(name, tok);
    //printf("assinment\n");
    requireToken("=", fp);
    Expr *lval = parseExpr(expectToken(tok, fp), fp);
    requireToken(";", fp);
    return makeAssignment( name, lval );

  }

  if (strcmp(tok, "if") == 0) {
    requireToken("(", fp);
    Expr *cond = parseExpr(expectToken(tok, fp), fp);
    requireToken(")", fp);
    Stmt *body = parseStmt(expectToken(tok, fp), fp);
    return makeIf(cond, body);
  }

  if (strcmp(tok, "while") == 0) {
    requireToken("(", fp);
    Expr *cond = parseExpr(expectToken(tok, fp), fp);
    requireToken(")", fp);
    Stmt *body = parseStmt(expectToken(tok, fp), fp);
    return makeWhile(cond, body);
  }

  // Figure out what type of statement this is based on the next token.
  if ( strcmp( tok, "print" ) == 0 ) {
    // Parse the one argument to print, and create a print expression.
    Expr *arg = parseExpr( expectToken( tok, fp ), fp );
    requireToken( ";", fp );
    return makePrint( arg );
  } else
    syntaxError();

  // Never reached.
  return NULL;
}
