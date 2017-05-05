#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "expr.h"
#include "stmt.h"
#include "parse.h"

/** Print a usage message then exit unsuccessfully. */
void usage()
{
  fprintf( stderr, "usage: interpreter <program-file>\n" );
  exit( EXIT_FAILURE );
}

/**
  Uses the other components to parse and execute statements from the input program.
  
  @param argc the number of command line arguments
  @param *argv an array of arguments as strings
  @return EXIT_SUCCESS successful program execution
*/
int main( int argc, char *argv[] )
{
  // Open the program's source.
  if ( argc != 2 )
    usage();
  FILE *fp = fopen( argv[ 1 ], "r" );
  if ( !fp ) {
    fprintf( stderr, "Can't open file: %s\n", argv[ 1 ] );
    usage();
  }

  // Context, for storing variable values.
  Context *ctxt = makeContext();
  
  // Parse one statement at a time, then run the statement
  // using the same context.
  char tok[ MAX_TOKEN + 1 ];

  int counter = 0;
  while ( parseToken( tok, fp ) ) {
    // Parse the next input statement.
    Stmt *stmt = parseStmt( tok, fp );

    // Run it.
    stmt->execute( stmt, ctxt );
    // Delete it.
    stmt->destroy( stmt );

    counter++;
  }
  
  // We're done, close the input file and free the context.
  fclose( fp );
  freeContext( ctxt );

  return EXIT_SUCCESS;
}

