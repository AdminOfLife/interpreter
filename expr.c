#include "expr.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////
// Context

/** Representation for a variable anme and its value. */
typedef struct {
  char name[ MAX_IDENT_LEN + 1 ];
  char *val;
} VarRec;

/** Hidden implementation of the context.  Really just a wrapper
    around a resizable array of VarRec structs. */
struct ContextTag {
  // List of all varname/value pairs.
  VarRec *vlist;

  // Number of name/value pairs.
  int len;

  // Capacity of the name/value list.
  int capacity;
};

Context *makeContext()
{
  // You need to implement this.
  Context *c = malloc(sizeof(Context));
  c->len = 0;
  c->capacity = 1;
  c->vlist = malloc(sizeof(VarRec) * c->capacity);
  return c;
}

char const *getVariable( Context *ctxt, char const *name )
{
  for (int i = 0; i < ctxt->len; i++) {
    if (strcmp(ctxt->vlist[i].name, name) == 0) {
      return ctxt->vlist[i].val;
    }
  }
  return "";
}

void setVariable( Context *ctxt, char const *name, char *value )
{
  for (int i = 0; i < ctxt->len; i++) {
    if (strcmp(ctxt->vlist[i].name, name) == 0) {
      free(ctxt->vlist[i].val);
      ctxt->vlist[i].val = malloc(strlen(value)+1);
      strcpy(ctxt->vlist[i].val, value);
      return;
    }
  }
  //printf("after search\n");
  if (ctxt->len == ctxt->capacity) {
    VarRec *newlist = malloc(sizeof(VarRec) * ctxt->capacity * 2);
    for (int i = 0; i < ctxt->len; i++) {
      strcpy(newlist[i].name, ctxt->vlist[i].name);
      newlist[i].val = ctxt->vlist[i].val;
    }
    free(ctxt->vlist);
    ctxt->vlist = newlist;
    ctxt->capacity = ctxt->capacity*2;
  }
  //printf("after resize\n");

  // if (ctxt->len == ctxt->capacity) {
  //   Context *c = malloc(sizeof(Context));
  //   c->len = ctxt->len;
  //   c->capacity = ctxt->capacity * 2;
  //   c->vlist = malloc(sizeof(VarRec) * c->capacity);

  //   for (int i = 0; i < ctxt->len; i++) {
  //     strcpy(c->vlist[i].name, ctxt->vlist[i].name);
  //     c->vlist[i].val = malloc(sizeof(ctxt->vlist[i].val)+1);
  //     strcpy(c->vlist[i].val, ctxt->vlist[i].val);
  //   }
  //   freeContext(ctxt);
  //   ctxt = c;
  // }


  if (ctxt->len < ctxt->capacity) {
    strcpy(ctxt->vlist[ctxt->len].name, name);
    ctxt->vlist[ctxt->len].val = malloc(strlen(value)+1);
    strcpy(ctxt->vlist[ctxt->len].val, value);
    ctxt->len++;
  }
  //printf("after insert\n");
}

void freeContext( Context *ctxt )
{
  for (int i = 0; i < ctxt->len; i++) {
    free(ctxt->vlist[i].val);
  }
  free(ctxt->vlist);
  free(ctxt);
}

//////////////////////////////////////////////////////////////////////
// Literal

// Representation for a Literal expression, derived from Expr.
typedef struct {
  char *(*eval)( Expr *oper, Context *ctxt );
  void (*destroy)( Expr *oper );

  /** Literal value of this expression. */
  char *val;
} LiteralExpr;

// Function to evaluate a literal expression.
static char *evalLiteral( Expr *expr, Context *ctxt )
{
  // Cast the this pointer to a more specific type.
  LiteralExpr *this = (LiteralExpr *)expr;

  // Make and return a copy of the value we contain.
  char *result = (char *) malloc( strlen( this->val ) + 1 );
  strcpy( result, this->val );
  return result;
}

// Function to free a literal expression.
static void destroyLiteral( Expr *expr )
{

  //printf("in destroy lit\n");

  // Cast the this pointer to a more specific type.
  LiteralExpr *this = (LiteralExpr *)expr;

  // Free the value we contain and the literal object itself.
  //printf("before free\n");
  free( this->val );
  //printf("after free1\n");
  free( this );
  //printf("after free2\n");
}

Expr *makeLiteral( char *val )
{
  // Allocate space for the LiteralExpr object
  LiteralExpr *this = (LiteralExpr *) malloc( sizeof( LiteralExpr ) );

  // Remember our virutal functions.
  this->eval = evalLiteral;
  this->destroy = destroyLiteral;

  // Remember the literal string we contain.
  this->val = val;

  // Return the result, as an instance of the base.
  return (Expr *) this;
}

// For double values, this should be the longest representation that could
// get printed with %f, a large positive exponent and some fractional digits.
#define MAX_NUMBER 400

//////////////////////////////////////////////////////////////////////
// Sum expressions

/** Representation for a sum expression.  This struct could probably
    be used to represent lots of different binary expressions. */
typedef struct {
  char *(*eval)( Expr *oper, Context *ctxt );
  void (*destroy)( Expr *oper );

  // Two sub-expressions.
  Expr *leftExpr, *rightExpr;
} SumExpr;

// Destroy function for sum expression.
static void destroySum( Expr *expr )
{
  SumExpr *this = (SumExpr *)expr;

  // Free our operand subexpressions.
  this->leftExpr->destroy( this->leftExpr );
  this->rightExpr->destroy( this->rightExpr );

  // Then the struct itself.
  free( this );
}


// Eval function for a sum expression.
static char *evalSum( Expr *expr, Context *ctxt )
{
  // Get a pointer to the more specific type this function works with.
  SumExpr *this = (SumExpr *)expr;

  // Evaluate our two operands
  char *left = this->leftExpr->eval( this->leftExpr, ctxt );
  char *right = this->rightExpr->eval( this->rightExpr, ctxt );

  // Parse the left and right operands as doubles.  Set them
  // to zero if they don't parse correctly.
  double a, b;
  if ( sscanf( left, "%lf", &a ) != 1 )
    a = 0.0;

  if ( sscanf( right, "%lf", &b ) != 1 )
    b = 0.0;

  // We're done with the values returned by our two subexpressions,
  // We just needed to get them as doubles
  free( left );
  free( right );

  // Compute the result, store it in a dynamically allocated string
  // and return it to the caller.
  char *result = (char *)malloc( MAX_NUMBER + 1 );
  sprintf( result, "%f", a + b );
  return result;
}

Expr *makeSum( Expr *leftExpr, Expr *rightExpr )
{
  // Make an instance of SumExpr
  SumExpr *this = (SumExpr *) malloc( sizeof( SumExpr ) );
  this->destroy = destroySum;
  this->eval = evalSum;

  // Remember the two sub-expressions.
  this->leftExpr = leftExpr;
  this->rightExpr = rightExpr;

  // Return the instance as if it's an Expr (which it sort of is)
  return (Expr *) this;
}

static char *evalDiff( Expr *expr, Context *ctxt )
{
  // Get a pointer to the more specific type this function works with.
  SumExpr *this = (SumExpr *)expr;

  // Evaluate our two operands
  char *left = this->leftExpr->eval( this->leftExpr, ctxt );
  char *right = this->rightExpr->eval( this->rightExpr, ctxt );

  // Parse the left and right operands as doubles.  Set them
  // to zero if they don't parse correctly.
  double a, b;
  if ( sscanf( left, "%lf", &a ) != 1 )
    a = 0.0;

  if ( sscanf( right, "%lf", &b ) != 1 )
    b = 0.0;

  // We're done with the values returned by our two subexpressions,
  // We just needed to get them as doubles
  free( left );
  free( right );

  // Compute the result, store it in a dynamically allocated string
  // and return it to the caller.
  char *result = (char *)malloc( MAX_NUMBER + 1 );
  sprintf( result, "%f", a - b );
  return result;
}

Expr *makeDifference( Expr *leftExpr, Expr *rightExpr )
{
  // Make an instance of SumExpr
  SumExpr *this = (SumExpr *) malloc( sizeof( SumExpr ) );
  this->destroy = destroySum;
  this->eval = evalDiff;

  // Remember the two sub-expressions.
  this->leftExpr = leftExpr;
  this->rightExpr = rightExpr;

  // Return the instance as if it's an Expr (which it sort of is)
  return (Expr *) this;
}

static char *evalProd( Expr *expr, Context *ctxt )
{
  // Get a pointer to the more specific type this function works with.
  SumExpr *this = (SumExpr *)expr;

  // Evaluate our two operands
  char *left = this->leftExpr->eval( this->leftExpr, ctxt );
  char *right = this->rightExpr->eval( this->rightExpr, ctxt );

  // Parse the left and right operands as doubles.  Set them
  // to zero if they don't parse correctly.
  double a, b;
  if ( sscanf( left, "%lf", &a ) != 1 )
    a = 0.0;

  if ( sscanf( right, "%lf", &b ) != 1 )
    b = 0.0;

  // We're done with the values returned by our two subexpressions,
  // We just needed to get them as doubles
  free( left );
  free( right );

  // Compute the result, store it in a dynamically allocated string
  // and return it to the caller.
  char *result = (char *)malloc( MAX_NUMBER + 1 );
  sprintf( result, "%f", a * b );
  return result;
}

Expr *makeProduct( Expr *leftExpr, Expr *rightExpr )
{
  // Make an instance of SumExpr
  SumExpr *this = (SumExpr *) malloc( sizeof( SumExpr ) );
  this->destroy = destroySum;
  this->eval = evalProd;

  // Remember the two sub-expressions.
  this->leftExpr = leftExpr;
  this->rightExpr = rightExpr;

  // Return the instance as if it's an Expr (which it sort of is)
  return (Expr *) this;
}

static char *evalQuot( Expr *expr, Context *ctxt )
{
  // Get a pointer to the more specific type this function works with.
  SumExpr *this = (SumExpr *)expr;

  // Evaluate our two operands
  char *left = this->leftExpr->eval( this->leftExpr, ctxt );
  char *right = this->rightExpr->eval( this->rightExpr, ctxt );

  // Parse the left and right operands as doubles.  Set them
  // to zero if they don't parse correctly.
  double a, b;
  if ( sscanf( left, "%lf", &a ) != 1 )
    a = 0.0;

  if ( sscanf( right, "%lf", &b ) != 1 )
    b = 0.0;

  // We're done with the values returned by our two subexpressions,
  // We just needed to get them as doubles
  free( left );
  free( right );

  // Compute the result, store it in a dynamically allocated string
  // and return it to the caller.
  char *result = (char *)malloc( MAX_NUMBER + 1 );
  sprintf( result, "%f", a / b );
  return result;
}

Expr *makeQuotient( Expr *leftExpr, Expr *rightExpr )
{
  // Make an instance of SumExpr
  SumExpr *this = (SumExpr *) malloc( sizeof( SumExpr ) );
  this->destroy = destroySum;
  this->eval = evalQuot;

  // Remember the two sub-expressions.
  this->leftExpr = leftExpr;
  this->rightExpr = rightExpr;

  // Return the instance as if it's an Expr (which it sort of is)
  return (Expr *) this;
}

static char *evalLess( Expr *expr, Context *ctxt )
{
  // Get a pointer to the more specific type this function works with.
  SumExpr *this = (SumExpr *)expr;

  // Evaluate our two operands
  char *left = this->leftExpr->eval( this->leftExpr, ctxt );
  char *right = this->rightExpr->eval( this->rightExpr, ctxt );

  // Parse the left and right operands as doubles.  Set them
  // to zero if they don't parse correctly.
  double a, b;
  if ( sscanf( left, "%lf", &a ) != 1 )
    a = 0.0;

  if ( sscanf( right, "%lf", &b ) != 1 )
    b = 0.0;

  // We're done with the values returned by our two subexpressions,
  // We just needed to get them as doubles
  free( left );
  free( right );

  // Compute the result, store it in a dynamically allocated string
  // and return it to the caller.
  char *result = (char *)malloc( MAX_NUMBER + 1 );
  if (a < b) {
    strcpy(result, "t");
  } else {
    strcpy(result, "");
  }
  return result;
}

Expr *makeLess( Expr *leftExpr, Expr *rightExpr )
{
  // Make an instance of SumExpr
  SumExpr *this = (SumExpr *) malloc( sizeof( SumExpr ) );
  this->destroy = destroySum;
  this->eval = evalLess;

  // Remember the two sub-expressions.
  this->leftExpr = leftExpr;
  this->rightExpr = rightExpr;

  // Return the instance as if it's an Expr (which it sort of is)
  return (Expr *) this;
}

static char *evalEqu( Expr *expr, Context *ctxt )
{
  // Get a pointer to the more specific type this function works with.
  SumExpr *this = (SumExpr *)expr;

  // Evaluate our two operands
  char *left = this->leftExpr->eval( this->leftExpr, ctxt );
  char *right = this->rightExpr->eval( this->rightExpr, ctxt );

  // Parse the left and right operands as doubles.  Set them
  // to zero if they don't parse correctly.
  // We're done with the values returned by our two subexpressions,
  // We just needed to get them as doubles
  // Compute the result, store it in a dynamically allocated string
  // and return it to the caller.
  char *result = (char *)malloc( MAX_NUMBER + 1 );
  if (strcmp(left, right)==0) {
    strcpy(result, "t");
  } else {
    strcpy(result, "");
  }
  free( left );
  free( right );

  return result;
}

Expr *makeEquals( Expr *leftExpr, Expr *rightExpr )
{
  // Make an instance of SumExpr
  SumExpr *this = (SumExpr *) malloc( sizeof( SumExpr ) );
  this->destroy = destroySum;
  this->eval = evalEqu;

  // Remember the two sub-expressions.
  this->leftExpr = leftExpr;
  this->rightExpr = rightExpr;

  // Return the instance as if it's an Expr (which it sort of is)
  return (Expr *) this;
}

static char *evalOr( Expr *expr, Context *ctxt )
{
  // Get a pointer to the more specific type this function works with.
  SumExpr *this = (SumExpr *)expr;

  // Evaluate our two operands
  char *left = this->leftExpr->eval( this->leftExpr, ctxt );
  char *right = this->rightExpr->eval( this->rightExpr, ctxt );

  // Parse the left and right operands as doubles.  Set them
  // to zero if they don't parse correctly.
  int a, b;
  if (strcmp(left, "") == 0)
    a = 0;
  else
    a = 1;
  if (strcmp(right, "") == 0)
    b = 0;
  else
    b = 1;

  // We're done with the values returned by our two subexpressions,
  // We just needed to get them as doubles
  free( left );
  free( right );

  // Compute the result, store it in a dynamically allocated string
  // and return it to the caller.
  char *result = (char *)malloc( MAX_NUMBER + 1 );
  if (a || b) {
    strcpy(result, "t");
  } else {
    strcpy(result, "");
  }
  return result;
}

Expr *makeOr( Expr *leftExpr, Expr *rightExpr )
{
  // Make an instance of SumExpr
  SumExpr *this = (SumExpr *) malloc( sizeof( SumExpr ) );
  this->destroy = destroySum;
  this->eval = evalOr;

  // Remember the two sub-expressions.
  this->leftExpr = leftExpr;
  this->rightExpr = rightExpr;

  // Return the instance as if it's an Expr (which it sort of is)
  return (Expr *) this;
}

static char *evalAnd( Expr *expr, Context *ctxt )
{
  // Get a pointer to the more specific type this function works with.
  SumExpr *this = (SumExpr *)expr;

  // Evaluate our two operands
  char *left = this->leftExpr->eval( this->leftExpr, ctxt );
  char *right = this->rightExpr->eval( this->rightExpr, ctxt );

  // Parse the left and right operands as doubles.  Set them
  // to zero if they don't parse correctly.
  int a, b;
  if (strcmp(left, "") == 0)
    a = 0;
  else
    a = 1;
  if (strcmp(right, "") == 0)
    b = 0;
  else
    b = 1;

  // We're done with the values returned by our two subexpressions,
  // We just needed to get them as doubles
  free( left );
  free( right );

  // Compute the result, store it in a dynamically allocated string
  // and return it to the caller.
  char *result = (char *)malloc( MAX_NUMBER + 1 );
  if (a && b) {
    strcpy(result, "t");
  } else {
    strcpy(result, "");
  }
  return result;
}

Expr *makeAnd( Expr *leftExpr, Expr *rightExpr )
{
  // Make an instance of SumExpr
  SumExpr *this = (SumExpr *) malloc( sizeof( SumExpr ) );
  this->destroy = destroySum;
  this->eval = evalAnd;

  // Remember the two sub-expressions.
  this->leftExpr = leftExpr;
  this->rightExpr = rightExpr;

  // Return the instance as if it's an Expr (which it sort of is)
  return (Expr *) this;
}

typedef struct {
  char *(*eval)( Expr *oper, Context *ctxt );
  void (*destroy)( Expr *oper );

  // Two sub-expressions.
  char name[MAX_IDENT_LEN+1];
} VarExpr;

static char *evalVar( Expr *expr, Context *ctxt ) {
  VarExpr *this = (VarExpr *)expr;
  char *result = (char *)malloc(MAX_NUMBER +1);
  strcpy(result, getVariable(ctxt, this->name));
  return result;
}

static void destroyVariable( Expr *expr )
{

  // Cast the this pointer to a more specific type.
  VarExpr *this = (VarExpr *)expr;

  free( this );
}

Expr *makeVariable (char const *vname) {

  VarExpr *this = (VarExpr *) malloc(sizeof(VarExpr));
  this->destroy = destroyVariable;
  this->eval = evalVar;

  strcpy(this->name, vname);
  return (Expr *) this;
}
