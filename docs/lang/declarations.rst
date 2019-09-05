.. highlight:: sparrow

Declarations
============

UNDER CONSTRUCTION

Syntax
------

.. code-block:: ebnf

    Declaration     = UsingDecl
                    | PackageDecl
                    | DatatypeDecl
                    | ConceptDecl
                    | VarDecl
                    | FunDecl
                    ;

    UsingDecl       = 'using' [Mods] QualifiedIdStar ';'
                    | 'using' [Mods] Id '=' Expr ';'
                    ;
    PackageDecl     = 'package' [Mods] Id [Params] '{' {Stmt} '}' ;
    DatatypeDecl    = 'class' [Mods] Id [Params] IfClause '{' Stmts '}'
                    | 'datatype'  [Mods] Id [Params] IfClause '{' {DatatypeItem} '}'
                    | 'datatype'  [Mods] Id [Params] '=' Expr IfClause ';'
                    ;
    ConceptDecl     = 'concept' [Mods] Id '(' Id [Type] ')' IfClause ';' ;
    VarDecl         = 'var' [Mods] IdList [Type] ['=' Expr] ';'
                    | 'var' [Mods] IdList '=' Expr ';'
                    ;
    FunDecl         = 'fun' [Mods] FunName [Params] [Type] IfClause FunBody
                    | 'fun' [Mods] FunName [Params] [Type] '=' Expr IfClause ';'
                    ;


    Mods            = '[' Expr {',' Expr} ']' ;
    IdList          = Id {',' Id} ;
    Type            = ':' ExprNE ;
    DatatypeItem    = UsingDecl | FieldsLine ;
    FieldsLine      = IdList Type ['=' Expr] ';' ;
    FunName         = IdOrOperator | '(' ')' ;
    FunBody         = '{' {Stmt} '}' | ';' ;
    Params          = '(' [Formal {',' Formal}] ')'
                    | IdList
                    ;
    Formal          = IdList Type ['=' Expr] ;
    IfClause        = ['if' Expr] ;

    Operator        = Oper | '=' ;
    OperatorNE      = Oper ;
    IdOrOperator    = Id | Operator ;
    IdOrOperatorNE  = Id | OperatorNE ;

Please note that tokens like ``{``, ``}`` or ``;`` can be introduced automatically by the lexer, depending on the layout of the program.

Using declarations
------------------

Using declarations introduce declaration names in a new context (package).

There are three forms of the using declaration:

- ``using QualifiedId`` This will just add a new symbol in the current context, referring to the declaration indicated by ``QualifiedId``
- ``using QualifiedId.*`` This assumes that QualifiedId is a package or module name, and will add all the declarations from that package to the current context
- ``using NewName = QualifiedId`` This creates an alias of the declaration indicated by ``QualifiedId`` and adds it with a new name to the current context.

Example:
::

    package Foo
        package Bar
            fun f = 1
            fun g = 2
    fun test1
        using Foo.Bar.f
        cout << f() << endl
    fun test2
        using Foo.Bar.*
        cout << f() << endl
        cout << g() << endl
    fun test3
        using ff = Foo.Bar.f
        using FB = Foo.Bar
        cout << ff() << endl
        cout << FB.g() << endl

Package declarations
--------------------

Package declarations are used to group code.

Example:
::

    package Grp
        datatype MyType = Int

        fun f(this: @MyType) = this.data
        fun print(this: @MyType) {...}

    fun caller
        var x: Grp.MyType
        cout << Grp.f(x) << endl
        Grp.print(x)
        x print             // OK, we are searching near the class first
        print(x)            // ERROR, cannot find 'print'

Packages can be generics. Please see :doc:`generics` for more details.

Datatype declarations
---------------------

As its name suggests, a datatype declaration introduces a new data type. In Sparrow, a datatype can only contain fields (variables) or other using declarations.

There are two main forms of declaring datatypes: an explicit one, and a simple one. The explicit form is exemplified by the following:
::

    datatype MyType
        x: Int
        y: Float
        name: String
        using BaseType = Int

In this example, we introduced a new composite type ``MyType`` that contains three fields: ``x``, ``y`` and ``name``. It also contains an using name ``BaseType`` that expands to ``Int``; there is no memory reserved for any using declarations; they are used for type introspection, especially in the context of generics.

The simple form of declaring datatypes is illustrated by the following example:
::

    datatype Type1 = Int
    datatype Type2 = Int*Float // Pair of Int & Float

This is a shortcut for the following code:
::

    datatype Type1
        data: Int
    datatype Type2
        data: Int*Float

In addition to this, the compiler will also generate a constructor that can covert the type given after ``=`` to the new type.

For any datatype declared, the compiler will also attempt to auto-generate several constructors, a destructor, a ``=`` and a ``==`` operator. For more details see :doc:`generatedAssocFun`.

Datatypes can be generics. Please see :doc:`generics` for more details.


Concept declarations
--------------------

Formally, a concept is a predicate on types, or from a different point of view, a set of types. We use it in generic programming to be able to operate on set of types.

Example:
::

    concept Swappable(x) if isValid(x.swap(x))

The above line can be read as: a type is *Swappable* if for a value ``x`` of that type, the expression after ``if`` (the *if-clause*) is fulfilled -- that is, ``x.swap(x)`` is a semantically valid construct.

As an if-clause, there can be any compile-time expression that evaluates to ``Bool``. If, for a type, the given if-clause will result in errors, the type will not model the concept.

Such a concept can be then use in generic programming, in the following way:
::

    fun doSwap(x: @Swappable, y: typeOf(x))
        x swap y

Please see :doc:`generics` for more details on generics.

TODO: base concepts


Variable declarations
---------------------

TODO

Function declarations
---------------------

TODO

Access modes
------------

Declarations in Sparrow can have three access modes: ``public``, ``protected`` and ``private``. By default, the ``public`` mode is assumed. If the declaration starts with ``_``, then ``private`` is assumed instead. If the declaration is a ctor, dtor or one of the ``=``, ``==`` operators, by default the access mode is ``protected``.

The user can force an access mode by using the ``public``, ``protected`` and ``private`` modifiers. This is especially useful for public imports, where we cannot control the name of the symbol imported.

A public declaration means that everybody can access and use that declaration.

A private declarations means that only code from the **same module** can use that declarations. Code that imports the module cannot use the declaration.

Example:
::

    module A
    fun f = 1
    fun _g = f() + 1
    fun h = g() + 1

::

    module B
    ...
    A.f()   // ok, 1
    A.g()   // ERROR
    A.h()   // ok, 3

.. note:: Private declarations of one package can be seen from another package in the same module.

This rule simplifies *friendship* relations between entities that are closely related. See the following example:
::

    module goodFamily
    package mom
        fun publicAttitude {...}
        fun _secrets {...}
    package dad
        fun publicAttitude {...}
        fun _secrets {...}
    package child
        fun schoolBehaviour {...}
        fun _secrets {...}

    fun discussion
        // No secrets between the members of the module
        mom._secrets
        dad._secrets
        child._secrets

A protected declaration is accessible by everybody. The difference from a public declaration is that protected declarations are not considered in ``using`` clauses without explicit names given. This way, one can hide the declaration from general name lookups. This hiding occurs across different modules as well as within the same module. Protected declarations can be still be accessed whenever we access them indirectly by searching near a class at the operator call.

Example:
::

    package ShySpace
        datatype Foo = Int
        [protected] fun print(this: Foo) { cout << this.data }

    using ShySpace.*
    var x: Foo = 0
    x print     // ok print searched indirectly through Foo
    x.print     // also ok
    print(x)    // ERROR: print is not visible here
