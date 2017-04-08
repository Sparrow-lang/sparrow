# Declarations

UNDER CONSTRUCTION

## Syntax
```
Declaration ::= UsingDecl
              | PackageDecl
              | ClassDecl
              | ConceptDecl
              | VarDecl
              | FunDecl

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

## Access modes

Declarations in Sparrow can have three access modes: `public`, `protected` and `private`. By default, the `public` mode is assumed. If the declaration starts with `_`, then `private` is assumed instead. If the declaration is a ctor, dtor or the `=` operator, by default the access mode is `protected`.

The user can force an access mode by using the `public`, `protected` and `private` modifiers. This is especially useful for public imports, where we cannot control the name of the symbol imported.

A public declaration means that everybody can access and use that declaration.

A private declarations means that only code from the **same module** can use that declarations. Code that imports the module cannot use the declaration.

Example:
```
module A;
fun f = 1;
fun _g = f() + 1;
fun h = g() + 1;
```
```
module B;
...
A.f()   // ok, 1
A.g()   // ERROR
A.h()   // ok, 3
```

> Note: Private declarations of one package can be seen from another package in the same module. This rule simplifies *friendship* relations between entities that are closely related. See the following example:

```
module goodFamily;
package mom {
    fun publicAttitude {...}
    fun _secrets {...}
}
package dad {
    fun publicAttitude {...}
    fun _secrets {...}
}
package child {
    fun schoolBehaviour {...}
    fun _secrets {...}
}

fun discussion {
    // No secrets between the members of the module
    mom._secrets;
    dad._secrets;
    child._secrets;
}
```

A protected declaration is accessible by everybody. The difference from a public declaration is that protected declarations are not considered in `using` clauses without explicit names given. This way, one can hide the declaration from general name lookups. This hiding occurs across different modules as well as within the same module. Protected declarations can be still be accessed whenever we access them indirectly by searching near a class at the operator call.

Example:
```
package ShySpace {
    datatype Foo = Int;
    [protected] fun print(this: Foo) { cout << this.data; }
}
using ShySpace.*
var x: Foo = 0
x print     // ok; print searched indirectly through Foo
x.print     // also ok
print(x)    // ERROR: print is not visible here
```