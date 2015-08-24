/*********************************************************************************************************************
 * C++ Declarations
 *********************************************************************************************************************/

// Code to be put in the header before the union type
%code requires
{
    #include <StdInc.h>
    #include <Nest/Frontend/Location.h>

    #include <Nodes/Decls/AccessType.h>

    typedef struct Nest_Node Node;

    namespace SprFrontend
    {
        class Scanner;
    }

    // Define the macro that is used to advance the current location to the end of the N-th location in the stack
    #define YYLLOC_DEFAULT(curLoc, rhs, N) \
        if ( N > 0 ) \
        { \
            (curLoc).sourceCode = YYRHSLOC(rhs, 1).sourceCode; \
            (curLoc).start = YYRHSLOC(rhs, 1).start; \
            (curLoc).end = YYRHSLOC(rhs, N).end; \
        } \
        else \
        { \
            (curLoc).sourceCode = YYRHSLOC(rhs, 0).sourceCode; \
            (curLoc).start = YYRHSLOC(rhs, 0).end; \
            (curLoc).end = YYRHSLOC(rhs, 0).end; \
        }
}


// Code to be put at the top of the cpp file
%code top
{
#include <StdInc.h>

#include <Nodes/Builder.h>
#include <Nodes/SparrowNodes.h>
#include <Feather/Nodes/FeatherNodes.h>

#include <Nest/Common/Diagnostic.h>
#include <Nest/Intermediate/Node.h>

#include <Grammar/Scanner.h>

using namespace SprFrontend;
using namespace Nest;

using namespace std;

#ifdef _WIN32
    #pragma warning(disable:4127 4065 4150 4702)
#endif

    // Used for empty rules to move the location to the start of the next token
    #define NEXT_LOC \
        { yylhs.location.sourceCode = yyla.location.sourceCode; yylhs.location.start = yyla.location.start; yylhs.location.end = yyla.location.start; }
}

/*********************************************************************************************************************
 * Bison Declarations
 *********************************************************************************************************************/

%require "3.0"                          // Require bison 3.0 or later
%no-lines                               // Don't use #line directives
//%debug                                  // add debug output code to generated parser. To be dsabled for release versions.
%error-verbose                          // verbose error messages
%expect 1                               // Expect that many shift-reduce warnings
%locations                              // keep track of the current position within the input
%defines                                // write out a header file containing the token defines
%skeleton "lalr1.cc"                    // use newer C++ skeleton file
%name-prefix "SprFrontend"              // namespace to enclose parser in
%define "parser_class_name" {Parser}    // set the parser's class identifier
%define api.location.type {Location}    // the type that holds a location

%start Start                            // the start rule

// The parameters to be passed to the grammar - also stored in the Parser class
%parse-param { SprFrontend::Scanner& scanner }
%parse-param { Location startLocation }
%parse-param { Node** resultNode }

%initial-action         // Actions to be performed when start parsing
{
    // Set the initial location
    @$ = startLocation;
};

/*********************************************************************************************************************
 * Grammar tokens
 *********************************************************************************************************************/

%union
{
    long long           integerVal;
    double              floatingVal;
    char                charVal;
    string*             stringVal;
    vector<string>*     stringList;
    Node*         node;
    AccessType          accessType;
}

%token                  END          0  "end of file"

%token START_PROGRAM START_EXPRESSION

// Keywords
%token BREAK
%token CATCH CLASS CONCEPT CONTINUE
%nonassoc THEN_CLAUSE
%nonassoc ELSE
%token FALSE FINALLY FOR FRIEND FUN
%token IF IMPORT
%token NULLCT
%token PACKAGE PRIVATE PROTECTED PUBLIC
%token RETURN
%token THIS THROW TRUE TRY
%token USING
%token VAR
%token WHILE

// Parentheses, delimiters, special operators
%token LCURLY RCURLY LBRACKET RBRACKET LPAREN RPAREN
%token COLON SEMICOLON COMMA DOT BACKSQUOTE
%token EQUAL

// Identifiers
%token<stringVal> IDENTIFIER    "identifier"
%token<stringVal> OPERATOR      "operator"
%destructor { delete $$; } IDENTIFIER OPERATOR

// Literals
%token CHAR_LITERAL     "Char literal"
%token STRING_LITERAL   "String literal"
%token INT_LITERAL      "Int literal"
%token LONG_LITERAL     "Long literal"
%token UINT_LITERAL     "UInt literal"
%token ULONG_LITERAL    "ULong literal"
%token FLOAT_LITERAL    "Float literal"
%token DOUBLE_LITERAL   "Double literal"


%type <stringVal>   Operator OperatorNoEq IdentifierOrOperator IdentifierOrOperatorNoEq
%type <node>        QualifiedName QualifiedNameStar
%type <stringList>  IdentifierList
%type <node>       IdentifierListNode Modifiers ModifierSpec

%destructor { delete $$; } Operator OperatorNoEq IdentifierOrOperator IdentifierOrOperatorNoEq

%type <node>        Start ProgramFile PackageTopDeclaration ImportDeclaration
%type <node>        ImportDeclarationsOpt ImportDeclarations
%type <node>        DeclarationsOpt Declarations FormalsOpt Formals Formal
%type <node>        Declaration InFunctionDeclaration PackageDeclaration ClassDeclaration ConceptDeclaration VarDeclaration FunDeclaration UsingDeclaration
//FriendDeclaration
%type <node>        IfClause FunRetType FunctionBody
%type <accessType>  AccessSpec
%type <stringVal>   FunOrOperName

%destructor { delete $$; } FunOrOperName

%type <node>        Expr PostfixExpr InfixExpr PrefixExpr SimpleExpr
%type <node>        ExprNoEq PostfixExprNoEq InfixExprNoEq PrefixExprNoEq SimpleExprNoEq
%type <node>        LambdaExpr
%type <node>        ExprList ExprListOpt LambdaClosureParams

%type <node>        Statement EmptyStmt ExpressionStmt BlockStmt BreakStmt ContinueStmt IfStmt ForStmt ReturnStmt WhileStmt
//ThrowStmt TryStmt CatchBlock FinallyBlock
%type <node>        Statements
//Catches


%{
// Define the lexer function used to do the lexing of the source code
#undef yylex
#define yylex scanner.lex
%}

/*********************************************************************************************************************
 * Grammar rules
 *********************************************************************************************************************/
%%

Start
    : START_PROGRAM ProgramFile
        { $$ = *resultNode = $2; }
    | START_EXPRESSION Expr
        { $$ = *resultNode = $2; }
    ;

///////////////////////////////////////////////////////////////////////////////
// Common rules
//

Operator
    : OPERATOR
        { $$ = $1; }
    | EQUAL
        { $$ = new string("="); }
    ;

OperatorNoEq
    : OPERATOR
        { $$ = $1; }
    ;

IdentifierOrOperator
    : IDENTIFIER
        { $$ = $1; }
    | Operator
        { $$ = $1; }
    ;

IdentifierOrOperatorNoEq
    : IDENTIFIER
        { $$ = $1; }
    | OperatorNoEq
        { $$ = $1; }
    ;

QualifiedName
	: QualifiedName DOT IDENTIFIER
        { $$ = mkCompoundExp(@$, $1, *$3); }
	| IDENTIFIER
        { $$ = mkIdentifier(@$, *$1); }
	;

QualifiedNameStar
	: QualifiedNameStar DOT IDENTIFIER
        { $$ = mkCompoundExp(@$, $1, *$3); }
	| QualifiedNameStar DOT OPERATOR    // Assume STAR here
        { $$ = mkStarExp(@$, $1, *$3); }
	| IDENTIFIER
        { $$ = mkIdentifier(@$, *$1); }
	;

IdentifierList
    : IdentifierList COMMA IDENTIFIER
        { $$ = buildStringList($1, *$3); }
    | IDENTIFIER
        { $$ = buildStringList(NULL, *$1); }
    ;

IdentifierListNode
    : IdentifierListNode COMMA IDENTIFIER
        { $$ = Feather::addToNodeList($1, mkIdentifier(@3, *$3)); }
    | IDENTIFIER
        { $$ = Feather::mkNodeList(@$, { mkIdentifier(@$, *$1) }); }
    ;

ModifierSpec
	: LBRACKET Modifiers RBRACKET
        { $$ = $2; }
    | /*nothing*/
        { $$ = NULL; }
	;

Modifiers
	: Modifiers COMMA Expr
        { $$ = Feather::addToNodeList($1, $3); }
	| Expr
        { $$ = Feather::addToNodeList(NULL, $1); }
	;


///////////////////////////////////////////////////////////////////////////////
// Program and declarations
//

ProgramFile
	: PackageTopDeclaration ImportDeclarationsOpt DeclarationsOpt END
        { $$ = mkSprCompilationUnit(@$, $1, $2, $3); }
	;

PackageTopDeclaration
	: PACKAGE QualifiedName SEMICOLON       // WARNING: Shift-reduce conflict here
        { $$ = $2; }
    | /*nothing*/
        { $$ = NULL; }
	;

ImportDeclarationsOpt
    : ImportDeclarations
        { $$ = $1; }
    | /*nothing*/
        { $$ = NULL; }
    ;

ImportDeclarations
	: ImportDeclarations ImportDeclaration
        { $$ = Feather::addToNodeList($1, $2); }
	| ImportDeclaration
        { $$ = Feather::addToNodeList(NULL, $1); }
	;

ImportDeclaration
	: IMPORT QualifiedNameStar SEMICOLON
        { $$ = $2; }
	| IMPORT STRING_LITERAL SEMICOLON
        { $$ = buildStringLiteral(@$, static_cast<string&>(*$<stringVal>2)); }
	;

DeclarationsOpt
    : Declarations
        { $$ = $1; }
    | /*nothing*/
        { $$ = NULL; }

Declarations
	: Declarations Declaration
        { $$ = Feather::addToNodeList($1, $2); }
	| Declaration
        { $$ = Feather::addToNodeList(NULL, $1); }
	;

Declaration
    : InFunctionDeclaration
        { $$ = $1; }
//	| FriendDeclaration
//        { $$ = $1; }
	| IfStmt
		{ $$ = $1; }
	| Expr SEMICOLON
		{ $$ = $1; }
 	;

InFunctionDeclaration
	: UsingDeclaration
        { $$ = $1; }
	| PackageDeclaration
        { $$ = $1; }
	| ClassDeclaration
        { $$ = $1; }
	| ConceptDeclaration
        { $$ = $1; }
	| VarDeclaration
        { $$ = $1; }
	| FunDeclaration
        { $$ = $1; }
 	;

AccessSpec
	: PRIVATE
        { $$ = privateAccess; }
	| PROTECTED
        { $$ = protectedAccess; }
	| PUBLIC
        { $$ = publicAccess; }
	| /*nothing*/
        { $$ = publicAccess; NEXT_LOC; }
	;

UsingDeclaration
	: AccessSpec USING ModifierSpec QualifiedNameStar SEMICOLON
        { $$ = mkModifiers(@$, mkSprUsing(@$, "", $4, $1), $3); }
	| AccessSpec USING ModifierSpec IDENTIFIER EQUAL Expr SEMICOLON
        { $$ = mkModifiers(@$, mkSprUsing(@$, *$4, $6, $1), $3); }
	;

//FriendDeclaration
//	: FRIEND ModifierSpec QualifiedName SEMICOLON
//        { $$ = mkModifiers(@$, mkFriend(@$, $3), $2); }
//	;

PackageDeclaration
	: AccessSpec PACKAGE ModifierSpec IDENTIFIER LCURLY DeclarationsOpt RCURLY
        { $$ = mkModifiers(@$, mkSprPackage(@$, *$4, $6, $1), $3); }
	;

VarDeclaration                        
	: AccessSpec VAR ModifierSpec IdentifierList COLON ExprNoEq EQUAL Expr SEMICOLON    // var[...] a,b,c : Int = 3;
        { $$ = buildVariables(@$, *$4, $6, $8, $3, $1); delete $4; }
	| AccessSpec VAR ModifierSpec IdentifierList COLON ExprNoEq SEMICOLON               // var[...] a,b,c : Int;
        { $$ = buildVariables(@$, *$4, $6, NULL, $3, $1); delete $4; }
	| AccessSpec VAR ModifierSpec IdentifierList EQUAL Expr SEMICOLON                   // var[...] a,b,c = 3;
        { $$ = buildVariables(@$, *$4, NULL, $6, $3, $1); delete $4; }
	;

ClassDeclaration
	: AccessSpec CLASS ModifierSpec IDENTIFIER FormalsOpt COLON ExprListOpt IfClause LCURLY DeclarationsOpt RCURLY
        { $$ = mkModifiers(@$, mkSprClass(@$, *$4, $5, $7, $8, $10, $1), $3); }
	| AccessSpec CLASS ModifierSpec IDENTIFIER FormalsOpt IfClause LCURLY DeclarationsOpt RCURLY
        { $$ = mkModifiers(@$, mkSprClass(@$, *$4, $5, NULL, $6, $8, $1), $3); }
	;

ConceptDeclaration
	: AccessSpec CONCEPT ModifierSpec IDENTIFIER LPAREN IDENTIFIER FunRetType RPAREN IfClause SEMICOLON
        { $$ = mkModifiers(@$, mkSprConcept(@$, *$4, *$6, $7, $9, $1), $3); }
	;

FunDeclaration
	: AccessSpec FUN ModifierSpec FunOrOperName FormalsOpt FunRetType IfClause FunctionBody
        { $$ = mkModifiers(@$, mkSprFunction(@$, *$4, $5, $6, $8, $7, $1), $3); }
	| AccessSpec FUN ModifierSpec FunOrOperName FormalsOpt FunRetType EQUAL Expr IfClause SEMICOLON
        { $$ = mkModifiers(@$, buildSprFunctionExp(@$, *$4, $5, $6, $8, $9, $1), $3); }
	;

FunRetType
    : COLON ExprNoEq
        { $$ = $2; }
    | /*nothing*/
        { $$ = NULL; }
    ;

FunOrOperName
	: IdentifierOrOperator  { $$ = $1; }
	| LPAREN RPAREN         { $$ = new string("()"); }
	;

FunctionBody
	: BlockStmt
        { $$ = $1; }
    | SEMICOLON
        { $$ = NULL; }
	;

FormalsOpt
    : LPAREN Formals RPAREN
        { $$ = $2; }
    | IdentifierList
        { $$ = buildParameters(@$, *$1, mkIdentifier(@$, "AnyType"), NULL, NULL); delete $1; }
    | LPAREN RPAREN
        { $$ = NULL; }
    | /*nothing*/
        { $$ = NULL; }
    ;

Formals
	: Formals COMMA Formal
        { $$ = Feather::appendNodeList($1, $3); }
	| Formal
        { $$ = $1; }
	;

Formal
	: IdentifierList COLON ExprNoEq EQUAL Expr
        { $$ = buildParameters(@$, *$1, $3, $5, NULL); delete $1; }
	| IdentifierList COLON ExprNoEq
        { $$ = buildParameters(@$, *$1, $3, NULL, NULL); delete $1; }
	;

IfClause
    : IF Expr
        { $$ = $2; }
    | /*nothing*/
        { $$ = NULL; }
    ;


///////////////////////////////////////////////////////////////////////////////
// Expressions
//


ExprListOpt
    : ExprList
        { $$ = $1; }
    | /*nothing*/
        { $$ = NULL; }
    ;

ExprList
	: ExprList COMMA Expr
        { $$ = Feather::addToNodeList($1, $3); }
	| Expr
        { $$ = Feather::addToNodeList(NULL, $1); }
	;

Expr
    : PostfixExpr
        { $$ = $1; }
    ;

// Same as expression, but don't allow EQUAL sign; used for types, where '=' can appear after them
ExprNoEq
    : PostfixExprNoEq
        { $$ = $1; }
    ;

PostfixExpr
    : InfixExpr
        { $$ = $1; }
    | InfixExpr IdentifierOrOperator
        { $$ = buildPostfixOp(@$, *$2, $1); }
    ;

PostfixExprNoEq
    : InfixExprNoEq
        { $$ = $1; }
    | InfixExprNoEq IdentifierOrOperatorNoEq
        { $$ = buildPostfixOp(@$, *$2, $1); }
    ;

InfixExpr
    : PrefixExpr
        { $$ = $1; }
    | InfixExpr IdentifierOrOperator PrefixExpr
        { $$ = mkInfixOp(@$, *$2, $1, $3); }
    ;

InfixExprNoEq
    : PrefixExprNoEq
        { $$ = $1; }
    | InfixExprNoEq IdentifierOrOperatorNoEq PrefixExprNoEq
        { $$ = mkInfixOp(@$, *$2, $1, $3); }
    ;

PrefixExpr
    : SimpleExpr
        { $$ = $1; }
    | Operator PrefixExpr
        { $$ = buildPrefixOp(@$, *$1, $2); }
    | BACKSQUOTE IDENTIFIER BACKSQUOTE PrefixExpr
        { $$ = buildPrefixOp(@$, *$2, $4); }
    ;

PrefixExprNoEq
    : SimpleExprNoEq
        { $$ = $1; }
    | OperatorNoEq PrefixExpr
        { $$ = buildPrefixOp(@$, *$1, $2); }
    | BACKSQUOTE IDENTIFIER BACKSQUOTE PrefixExprNoEq
        { $$ = buildPrefixOp(@$, *$2, $4); }
    ;

SimpleExpr
	: SimpleExpr LPAREN ExprListOpt RPAREN
        { $$ = mkInfixOp(@$, "__fapp__", $1, $3); }
    | SimpleExpr DOT IdentifierOrOperator
        { $$ = mkInfixOp(@$, "__dot__", $1, mkIdentifier(@3, *$3)); }
    | SimpleExpr DOT LPAREN RPAREN
        { $$ = mkInfixOp(@$, "__dot__", $1, mkIdentifier(@3, "()")); }
    | SimpleExpr LBRACKET Modifiers RBRACKET
        { $$ = mkModifiers(@$, $1, $3); }
    | LambdaExpr
        { $$ = $1; }
	| LPAREN Expr RPAREN
        { $$ = buildParenthesisExp(@$, $2); }
	| IDENTIFIER
        { $$ = mkIdentifier(@$, *$1); }
	| THIS
        { $$ = mkThisExp(@$); }
	| NULLCT
        { $$ = buildNullLiteral(@$); }
	| TRUE
        { $$ = buildBoolLiteral(@$, true); }
	| FALSE
        { $$ = buildBoolLiteral(@$, false); }
	| INT_LITERAL
        { $$ = buildIntLiteral(@$, static_cast<int>($<integerVal>1)); }
	| LONG_LITERAL
        { $$ = buildLongLiteral(@$, static_cast<long>($<integerVal>1)); }
	| UINT_LITERAL
        { $$ = buildUIntLiteral(@$, static_cast<unsigned>($<integerVal>1)); }
	| ULONG_LITERAL
        { $$ = buildULongLiteral(@$, static_cast<unsigned long>($<integerVal>1)); }
	| FLOAT_LITERAL
        { $$ = buildFloatLiteral(@$, static_cast<float>($<floatingVal>1)); }
	| DOUBLE_LITERAL
        { $$ = buildDoubleLiteral(@$, static_cast<double>($<floatingVal>1)); }
	| CHAR_LITERAL
        { $$ = buildCharLiteral(@$, static_cast<char>($<charVal>1)); }
	| STRING_LITERAL
        { $$ = buildStringLiteral(@$, static_cast<string&>(*$<stringVal>1)); }
	;

SimpleExprNoEq
	: SimpleExprNoEq LPAREN ExprListOpt RPAREN
        { $$ = mkInfixOp(@$, "__fapp__", $1, $3); }
    | SimpleExprNoEq DOT IdentifierOrOperatorNoEq
        { $$ = mkInfixOp(@$, "__dot__", $1, mkIdentifier(@3, *$3)); }
    | SimpleExprNoEq LBRACKET Modifiers RBRACKET
        { $$ = mkModifiers(@$, $1, $3); }
    | LambdaExpr
        { $$ = $1; }
	| LPAREN Expr RPAREN
        { $$ = buildParenthesisExp(@$, $2); }
	| IDENTIFIER
        { $$ = mkIdentifier(@$, *$1); }
	| THIS
        { $$ = mkThisExp(@$); }
	| NULLCT
        { $$ = buildNullLiteral(@$); }
	| TRUE
        { $$ = buildBoolLiteral(@$, true); }
	| FALSE
        { $$ = buildBoolLiteral(@$, false); }
	| INT_LITERAL
        { $$ = buildIntLiteral(@$, static_cast<int>($<integerVal>1)); }
	| LONG_LITERAL
        { $$ = buildLongLiteral(@$, static_cast<long>($<integerVal>1)); }
	| UINT_LITERAL
        { $$ = buildUIntLiteral(@$, static_cast<unsigned>($<integerVal>1)); }
	| ULONG_LITERAL
        { $$ = buildULongLiteral(@$, static_cast<unsigned long>($<integerVal>1)); }
	| FLOAT_LITERAL
        { $$ = buildFloatLiteral(@$, static_cast<float>($<floatingVal>1)); }
	| DOUBLE_LITERAL
        { $$ = buildDoubleLiteral(@$, static_cast<double>($<floatingVal>1)); }
	| CHAR_LITERAL
        { $$ = buildCharLiteral(@$, static_cast<char>($<charVal>1)); }
	| STRING_LITERAL
        { $$ = buildStringLiteral(@$, static_cast<string&>(*$<stringVal>1)); }
	;

LambdaExpr
    :   LPAREN FUN LambdaClosureParams FormalsOpt FunRetType FunctionBody RPAREN
        { $$ = mkLambdaExp(@$, $4, $5, $6, NULL, $3); }
    |   LPAREN FUN LambdaClosureParams FormalsOpt FunRetType EQUAL Expr RPAREN
        { $$ = mkLambdaExp(@$, $4, $5, NULL, $7, $3); }
    ;

LambdaClosureParams
    :   DOT LCURLY IdentifierListNode RCURLY
        { $$ = $3; }
    |   DOT LCURLY RCURLY
        { $$ = NULL; }
    |   /*nothing*/
        { $$ = NULL; }
    ;


///////////////////////////////////////////////////////////////////////////////
// Statements
//

Statements
    : Statements Statement
        { $$ = Feather::addToNodeList($1, $2); }
    | /*nothing*/
        { $$ = NULL; }
    ;

Statement
	: EmptyStmt
        { $$ = $1; }
	| ExpressionStmt
        { $$ = $1; }
	| BlockStmt
        { $$ = $1; }
	| IfStmt
        { $$ = $1; }
    | ForStmt
        { $$ = $1; }
	| WhileStmt
        { $$ = $1; }
	| BreakStmt
        { $$ = $1; }
	| ContinueStmt
        { $$ = $1; }
	| ReturnStmt
        { $$ = $1; }
//	| ThrowStmt
//        { $$ = $1; }
//	| TryStmt
//        { $$ = $1; }
    | InFunctionDeclaration
        { $$ = $1; }
    ;

EmptyStmt
	: SEMICOLON
        { $$ = NULL; }
	;

ExpressionStmt
	: Expr SEMICOLON
        { $$ = buildExpressionStmt(@$, $1); }
	;

BlockStmt
    : LCURLY Statements RCURLY
        { $$ = buildBlockStmt(@$, $2); }
    ;

IfStmt
	: IF ModifierSpec LPAREN Expr RPAREN Statement ELSE Statement
        { $$ = mkModifiers(@$, buildIfStmt(@$, $4, $6, $8), $2); }
	| IF ModifierSpec LPAREN Expr RPAREN Statement %prec THEN_CLAUSE
        { $$ = mkModifiers(@$, buildIfStmt(@$, $4, $6, NULL), $2); }
	;

ForStmt
	: FOR ModifierSpec LPAREN IDENTIFIER COLON ExprNoEq EQUAL Expr RPAREN Statement
        { $$ = mkModifiers(@$, mkForStmt(@$, *$4, $6, $8, $10), $2); }
	| FOR ModifierSpec LPAREN IDENTIFIER EQUAL Expr RPAREN Statement
        { $$ = mkModifiers(@$, mkForStmt(@$, *$4, NULL, $6, $8), $2); }
	;

WhileStmt
	: WHILE ModifierSpec LPAREN Expr RPAREN Statement
        { $$ = mkModifiers(@$, buildWhileStmt(@$, $4, NULL, $6), $2); }
	| WHILE ModifierSpec LPAREN Expr SEMICOLON Statement RPAREN Statement
        { $$ = mkModifiers(@$, buildWhileStmt(@$, $4, $6, $8), $2); }
	| WHILE ModifierSpec LPAREN Expr SEMICOLON Expr RPAREN Statement
        { $$ = mkModifiers(@$, buildWhileStmt(@$, $4, $6, $8), $2); }
	;

BreakStmt
	: BREAK SEMICOLON
        { $$ = buildBreakStmt(@$); }
	;

ContinueStmt
	: CONTINUE SEMICOLON
        { $$ = buildContinueStmt(@$); }
	;

ReturnStmt
	: RETURN SEMICOLON
        { $$ = mkReturnStmt(@$, NULL); }
	| RETURN Expr SEMICOLON
        { $$ = mkReturnStmt(@$, $2); }
	;

//ThrowStmt
//	: THROW Expr SEMICOLON
//        { $$ = mkThrowStmt(@$, $2); }
//	;
//
//TryStmt
//	: TRY BlockStmt Catches FinallyBlock
//        { $$ = mkTryStmt(@$, $2, $3, $4); }
//	;
//
//Catches
//	: Catches CatchBlock
//        { $$ = Feather::addToNodeList($1, $2); }
//    | /*nothing*/
//        { $$ = NULL; }
//	;
//
//CatchBlock
//	: CATCH Formal BlockStmt
//        { $$ = mkCatchBlock(@$, $2, $3); }
//	;
//
//FinallyBlock
//	: FINALLY BlockStmt
//        { $$ = mkFinallyBlock(@$, $2); }
//    | /*nothing*/
//        { $$ = NULL; }
//	;


/*********************************************************************************************************************
 * Additional C++ code
 *********************************************************************************************************************/
%%

void Parser::error(const Parser::location_type& location, const string& m)
{
    REP_ERROR(location, "%1%") % m;
}
