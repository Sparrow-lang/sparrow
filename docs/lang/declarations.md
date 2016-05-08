# Declarations

UNDER CONSTRUCTION

## Syntax
```
Declaration ::= [AccessSpec] UsingDecl
              | [AccessSpec] PackageDecl
              | [AccessSpec] ClassDecl
              | [AccessSpec] ConceptDecl
              | [AccessSpec] VarDecl
              | [AccessSpec] FunDecl
AccessSpec  ::= 'private' | 'public'

UsingDecl   ::= 'using' [Mods] QualifiedIdStar ';'
              | 'using' [Mods] Id '=' Expr ';'
PackageDecl ::= 'package' [Mods] Id '{' {TopLevel} '}'
ClassDecl   ::= 'class' [Mods] Id [Params] IfClause '{' {TopLevel} '}'
              | 'datatype'  [Mods] Id [Params] IfClause '{' [Formal {',' Formal}] '}'
              | 'datatype'  [Mods] Id [Params] '=' Expr IfClause ';'
ConceptDecl ::= 'concept' [Mods] Id '(' Id [Type] ')' IfClause ';'
VarDecl     ::= 'var' [Mods] IdList [Type] ['=' Expr] ';'
              | 'var' [Mods] IdList '=' Expr ';'
FunDecl     ::= 'fun' [Mods] FunName [Params] [Type] IfClause FunBody
              | 'fun' [Mods] FunName [Params] [Type] '=' Expr IfClause ';'

Mods        ::= '[' Expr {',' Expr} ']'
IdList      ::= Id {',' Id}
Type        ::= ':' ExprNE
FunName     ::= IdOrOperator | '(' ')'
FunBody     ::= '{' {Stmt} '}' | ';'
Params      ::= '(' [Formal {',' Formal}] ')'
              | IdList
Formal      ::= IdList Type ['=' Expr]
IfClause    ::= ['if' Expr]

Operator       ::= Oper | '='
OperatorNE     ::= Oper
IdOrOperator   ::= Id | Operator
IdOrOperatorNE ::= Id | OperatorNE
```

## Using declarations

Using declarations introduce declaration names in a new context (package).

There are three forms of the using declaration:
- `using QualifiedId` This will just add a new symbol in the current context, referring to the declaration indicated by `QualifiedId`
- `using QualifiedId.*` This assumes that QualifiedId is a package or module name, and will add all the declarations from that package to the current context
- `using NewName = QualifiedId` This creates an alias of the declaration indicated by `QualifiedId` and adds it with a new name to the current context.

Example:
```
package Foo {
    package Bar {
        fun f = 1;
        fun g = 2;
    }
}
fun test1 {
    using Foo.Bar.f;
    cout << f() << endl;
}
fun test2 {
    using Foo.Bar.*;
    cout << f() << endl;
    cout << g() << endl;
}
fun test3 {
    using ff = Foo.Bar.f;
    using FB = Foo.Bar;
    cout << ff() << endl;
    cout << FB.g() << endl;
}
```

## Package declarations

TODO

## Class declarations

TODO

## Concept declarations

TODO

## Variable declarations

TODO

## Function declarations

TODO

## Access specifiers

Declarations in Sparrow can have only two access specifiers: `public` and `private`. If an access specifier is not explicitly mentioned, the `private` specifier is assumed.

A public declaration means that everybody can access and use that declaration.

A private declarations means that only code from the same module can use that declarations. Code that imports the module cannot use the declaration.

Example:
```
module A;
fun f = 1;
private fun g = f() + 1;
public fun h = g() + 1;
```
```
module B;
...
A.f()   // ERROR
A.g()   // ERROR
A.h()   // ok, 3
```