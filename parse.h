/**
  @file parse.h
  @author Arthur Vargas (ahvargas@ncsu.edu)

  Parser and tokenizer functions.
*/

#ifndef _PARSE_H_
#define _PARSE_H_

#include <stdio.h>
#include <stdbool.h>

#include "expr.h"
#include "stmt.h"

//////////////////////////////////////////////////////////////////////
// Input totkenization

// Maximum length of a token in the source file.
#define MAX_TOKEN 1023

/** Read the next token from the given file, a space-delimtied word, a
    double quoted string or closing parentheses, crly brackets or a
    semi-colon..
    @param tok storage for the token, with room for a string of up to
     MAX_TOKEN characters.
    @param fp file to read tokens from.
    @return true if the token is successfully read.
*/
bool parseToken( char *token, FILE *fp );

/** Parse with one token worth of look-ahead, return the Expr
    object representing the next legal expression from the input.
    @param tok next token from the input, already read before
    calling this function.
    @param fp file subsequent tokens are being read from.
    @return the Expr object constructed from the input.
*/
Expr *parseExpr( char *tok, FILE *fp );

/** Parse with one token worth of look-ahead, return the Stmt
    object representing the next legal statement from the input.
    @param tok next token from the input, already read before
    calling this function.
    @param fp file subsequent tokens are being read from.
    @return the Stmt object constructed from the input.
*/
Stmt *parseStmt( char *tok, FILE *fp );

#endif

