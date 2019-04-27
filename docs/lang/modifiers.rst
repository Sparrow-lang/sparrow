.. highlight:: sparrow

Modifiers
=========

Modifiers allow changing the semantic of Sparrow constructs.

Example:
::

    [initCtor]
    datatype MyType
        a: Int
        b: Bool

    var x = MyType(13, true)

The ``initCtor`` is a modifier that makes the ``MyType`` datatype to have an initialization constructor for the two variables.

Syntax
------

.. code-block:: ebnf

    Mods            = '[' Expr { ',' Expr } ']' ;

    Stmts           = { Stmt } ;
    Stmt            = [Mods] ImportLine
                      | [Mods] UsingDecl
                      | [Mods] PackageDecl
                      | [Mods] DatatypeDecl
                      | [Mods] ConceptDecl
                      | [Mods] VarDecl
                      | [Mods] FunDecl
                      | [Mods] ExprStmt
                      | [Mods] BlockStmt
                      | [Mods] IfStmt
                      | [Mods] ForStmt
                      | [Mods] WhileStmt
                      | [Mods] BreakStmt
                      | [Mods] ContinueStmt
                      | [Mods] ReturnStmt
                      ;

Predefined modifiers
--------------------

``public`` modifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Applies to
    all declarations

Semantics
    Marks the declaration as being public. A public declaration can be accessed from any module.

See
    :ref:`Access modes`

``protected`` modifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Applies to
    all declarations

Semantics
    Marks the declaration as being protected. A protected declaration can be accessed from any module, but it will not be considered in `using` clauses without explicit names.

See
    :ref:`Access modes`


``private`` modifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Applies to
    all declarations

Semantics
    Marks the declaration as being private. A private declaration can be accessed only from the current module.

See
    :ref:`Access modes`


``ct`` modifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Applies to
    all declarations, `if` statements, `while` statements, `for` statements

Semantics
    Marks the declaration or statement to be executed at compile-time. A `ct` declaration will not have a runtime counterpart. A `ct` statement will be executed at compile-time; used in specializing code or code generation.

    Examples:
    ::

        [ct] fun addRef(t: Type): Type // function is available only at compile-time

        [ct]
        if sizeOf(t) <= sizeOf(Int)
            var storage: Int
        else
            var storage: Long
        // Depending on the size of 't', this will create different 'storage' variables

        [ct]
        for x = 1..5
            doStuff(ctEval(x))
        // Will generate the following code:
        //      doStuff(1)
        //      doStuff(2)
        //      doStuff(3)
        //      doStuff(4)


``rt`` modifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Applies to
    all declarations

Semantics
    Ensures the declaration has meaning both at run-time and at compile-time. The default behavior. Can be used in ``[ct]`` environments to go back to default ``[rt]`` environments


``autoCt`` modifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Applies to
    functions

Semantics
    If the function is called with only with compile-time arguments, the function will be compile-time; otherwise it will be run-time.

    Example:
    ::

        [autoCt] fun printVals(x, y: Int)
            cout << x << ' ' << y << '\n'

        var six = 6
        var seven = 7

        printVals(6, 7)         // prints '6 7' at compile-time
        printVals(six, 7)       // prints '6 7' at run-time
        printVals(six, seven)   // prints '6 7' at run-time

``ctGeneric`` modifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Applies to
    functions

Semantics
    Marks the function as being both a compile-time function and a generic function.

    A regular function (non-``[ct]``) that has compile-time parameters will be a generic; it can change the semantics of its body based on the compile-time parameters, and has run-time presence. On the other hand, a function marked as ``[ct]`` is not a generic; it doesn't have any run-time presence and it cannot change the semantics of its body based on the parameters.

    A ``[ctGeneric]`` function is a combination of both: it's a compile-time function (no run-time presence) but has a different code generated for each set of parameters.

``convert`` modifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Applies to
    ``ctor`` functions and datatypes

Semantics
    Used to indicate that a datatype can be implicitly converted from other types through constructors marked as ``[convert]``. To convert a value from a type ``A`` to a type ``B`` implicitly, the following conditions must be met:

    * the ``B`` datatype must be declared as ``[convert]``
    * ``B`` needs to have a constructor marked as ``[convert]`` that can construct a ``B`` from values of type ``A``


``noDefault`` modifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Applies to
    datatypes, ``ctor`` and ``dtor`` functions

Semantics
    When applied to datatypes, it tells the compiler not to generate default functions for the datatype. When applied to constructors and desructors, it tells the compiler not to try to inject constructor/destructor calls for the members of the datatype.

``initCtor`` modifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Applies to
    datatypes

Semantics
    It instructs the compiler to generate an extra constructor to initialize all the members of the datatype.

    When generating the initialized constructor, the following are applied:

    * the initialization constructor will have a parameter for each field of the datatype
    * the order of parameters in the initialization constructor will be the same as the oder in which the corresponding fields are declared in the datatype
    * if the field of the datatype has an initializer, the corresponding parameters will have the same initializer

``bitcopiable`` modifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Applies to
    datatypes

Semantics
    It instructs the compiler that the given datatype can be safely copied bitwise (i.e., with ``memcpy``). The copy constructor will be elided whenever possible.

    Also, functions returning *bitcopiable* will not take the location in which the resulting value needs to be placed by (hidden) reference.

    For example, a ``Vector`` applied to a *bitcopiable* type can avoid calling the copy constructor
    of the elements whenever resizing the data.

    The basic numeric types are all *bitcopiable*.

``autoBitcopiable`` modifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Applies to
    datatypes

Semantics
    Instructs the compiler to detect whether the datatype can be bitcopiable or not. A datatype with ``[autoBitcopiable]`` modifier applied will become *bitcopiable* if all the fields are also *bitcopiable*; if at least one of the fields is not *bitcopiable* then the datatype will not be *bitcopiable*.

    It is used for datatypes like ``Tuple`` to automatically become *bitcopiable* base on the field types.

``macro`` modifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Applies to
    functions

Semantics
    When making calls to the function, it will pass the raw AST (abstract syntax tree) nodes to the functions, without even compiling them. Used for manipulating the source code.

    Example: ``assert`` uses macros to extract the variables out of the given condition, to be able to print them whenever the assertion fails.

``noInline`` modifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Applies to
    functions

Semantics
    Prevents the compiler from inlining the function.

``native`` modifier
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Applies to
    functions, datatypes

Form
    ``native(`` *stringLiteral* ``)``

Semantics
    Assigns the specified name to the given function/datatype. It does not apply any name mangling rules when generating the assembly name.

    Native datatypes are used to map types defined in the standard library onto machine types. I.e., ``[native("i32")] datatype Int`` is mapping the type ``Int`` to a signed 32-bit machine type.

    Functions that are declared native would not change their return type (non-*bitcopiable* return types are typically transformed into hidden pointer parameters).

    Typically used when interacting with other libraries.


User-defined modifiers
----------------------

TODO: not yet implemented
