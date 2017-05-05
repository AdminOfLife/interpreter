#include "stmt.h"
#include "expr.h"
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////
// Print

// Representation for a print statement, derived from Stmt.
typedef struct {
  void (*execute)( Stmt *stmt, Context *ctxt );
  void (*destroy)( Stmt *stmt );

  /** Argument expression we're supposed to evaluate and print. */
  Expr *arg;
} PrintStmt;

typedef struct {
  void (*execute)( Stmt *stmt, Context *ctxt );
  void (*destroy)( Stmt *stmt );

  /** Argument expression we're supposed to evaluate and print. */
  char vname[MAX_IDENT_LEN + 1];
  Expr *lval;
} AssignStmt;

// Function to execute a print statemenchar const *vnamet.
static void executePrint( Stmt *stmt, Context *ctxt )
{
  // Cast the this pointer to a more specific type.
  PrintStmt *this = (PrintStmt *)stmt;

  // Evaluate our argument, print the result, then free it.
  char *result = this->arg->eval( this->arg, ctxt );
  printf( "%s", result );
  free( result );
}

// Function to free a print statement.
static void destroyPrint( Stmt *stmt )
{
  // Cast the this pointer to a more specific type.
  PrintStmt *this = (PrintStmt *)stmt;

  // Free our subexpression then the print statement itself.
  this->arg->destroy( this->arg );
  free( this );
}

Stmt *makePrint( Expr *arg )
{
  // Allocate space for the PrintStmt object
  PrintStmt *this = (PrintStmt *) malloc( sizeof( PrintStmt ) );

  // Remember our virutal functions.
  this->execute = executePrint;
  this->destroy = destroyPrint;

  // Remember our argument subexpression.
  this->arg = arg;

  // Return the result, as an instance of the Stmt interface.
  return (Stmt *) this;
}

// Function to execute an assignment statement 
static void executeAssign( Stmt *stmt, Context *ctxt )
{
  // Cast the this pointer to a more specific type.
  AssignStmt *this = (AssignStmt *)stmt;

  // Evaluate our argument, print the result, then free it.
  char *result = this->lval->eval( this->lval, ctxt );
  setVariable( ctxt, this->vname, result );
  free( result );
}

// Function to free an assignment statement.
static void destroyAssign( Stmt *stmt )
{

  //printf("in destroy assign\n");

  // Cast the this pointer to a more specific type.
  AssignStmt *this = (AssignStmt *)stmt;

  // Free our subexpression then the print statement itself.
  this->lval->destroy( this->lval );
  //printf("after free\n");
  free( this );
  //printf("out of destroy\n");
}

Stmt *makeAssignment( char const *vname, Expr *expr ) {
  AssignStmt *this = (AssignStmt *) malloc( sizeof ( AssignStmt ) );

  this->execute = executeAssign;
  this->destroy = destroyAssign;

  this->lval = expr;
  strcpy(this->vname,vname);
  return (Stmt *) this;
}
//////////////////////////////////////////////////////////////////////
// Compound

// Representation for a compound statement, derived from Stmt.
typedef struct {
  void (*execute)( Stmt *stmt, Context *ctxt );
  void (*destroy)( Stmt *stmt );

  /** List of statements in the compound. */
  Stmt **stmtList;

  /** Number of statements in the compound. */
  int len;
} CompoundStmt;

// Function to execute a compound statement.
static void executeCompound( Stmt *stmt, Context *ctxt )
{
  // Cast the this pointer to a more specific type.
  CompoundStmt *this = (CompoundStmt *)stmt;

  // Execute the sequence of statements in this compound
  for ( int i = 0; i < this->len; i++ )
    this->stmtList[ i ]->execute( this->stmtList[ i ], ctxt );
}

// Function to free a compound statement.
static void destroyCompound( Stmt *stmt )
{
  // Cast the this pointer to a more specific type.
  CompoundStmt *this = (CompoundStmt *)stmt;

  // Free the list of statements inside this compond.
  for ( int i = 0; i < this->len; i++ )
    this->stmtList[ i ]->destroy( this->stmtList[ i ] );

  // Then, free the compound statement itself.
  free( this->stmtList );
  free( this );
}

Stmt *makeCompound( Stmt **stmtList, int len )
{
  // Allocate space for the CompoundStmt object
  CompoundStmt *this = (CompoundStmt *) malloc( sizeof( CompoundStmt ) );

  // Remember our virutal functions.
  this->execute = executeCompound;
  this->destroy = destroyCompound;

  // Remember the list of statements in the compound.
  this->stmtList = stmtList;
  this->len = len;

  // Return the result, as an instance of the Stmt interface.
  return (Stmt *) this;
}

typedef struct {
  void (*execute)( Stmt *stmt, Context *ctxt );
  void (*destroy)( Stmt *stmt );

  /** List of statements in the compound. */
  Expr *cond;
  Stmt *body;
} IfStmt;

// function to execute an if statement
static void executeIf( Stmt *stmt, Context *ctxt )
{
  // Cast the this pointer to a more specific type.
  IfStmt *this = (IfStmt *)stmt;

  // Evaluate our argument, print the result, then free it.
  char *result = this->cond->eval( this->cond, ctxt );
  if (strcmp(result, "") != 0) {
    this->body->execute(this->body, ctxt);
  }

  free( result );
}

// function to free the if statement
static void destroyIf( Stmt *stmt )
{
  // Cast the this pointer to a more specific type.
  IfStmt *this = (IfStmt *)stmt;

  this->cond->destroy(this->cond);
  this->body->destroy(this->body);

  // Then, free the compound statement itself.
  free( this );
}

Stmt *makeIf( Expr *cond, Stmt *body ) {
  IfStmt * this = (IfStmt *) malloc (sizeof(IfStmt));

  this->execute = executeIf;
  this->destroy = destroyIf;

  this->cond = cond;
  this->body = body;

  return (Stmt *)this;
}

// function to execute a while statement
static void executeWhile( Stmt *stmt, Context *ctxt )
{
  // Cast the this pointer to a more specific type.
  IfStmt *this = (IfStmt *)stmt;

  // Evaluate our argument, print the result, then free it.
  char *result = this->cond->eval( this->cond, ctxt );
  while (strcmp(result, "") != 0) {
    this->body->execute(this->body, ctxt);
    free(result);
    result = this->cond->eval( this->cond, ctxt );
  }

  free( result );
}

Stmt *makeWhile( Expr *cond, Stmt *body ) {
  IfStmt * this = (IfStmt *) malloc (sizeof(IfStmt));

  this->execute = executeWhile;
  this->destroy = destroyIf;

  this->cond = cond;
  this->body = body;

  return (Stmt *)this;
}

