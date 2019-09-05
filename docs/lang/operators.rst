.. highlight:: sparrow

Operators
=========

There are 3 types of operators in Sparrow, based on their form:

- prefix operators: ``oper_name arg``; example: ``++a``
- postfix operators: ``arg oper_name``; example: ``a++``
- infix operators: ``arg1 oper_name arg2``; example: ``a + b``

All operators in Sparrow are user-defined. However, compilers may choose to implement some of them directly, without relying on user-code, for optimization purposes.

Sparrow does not have a fixed set of operators. Almost all combination of symbols are valid operator names. Moreover, alphanumeric identifiers can also be operator names.

Syntax
------

.. code-block:: ebnf

    Operator        = Oper | '=' ;
    OperatorNE      = Oper ;
    IdOrOperator    = Id | Operator ;
    IdOrOperatorNE  = Id | OperatorNE ;

    Letter          = 'a'-'z' | 'A'-'Z' | '_' ;
    Digit           = '0'-'9' ;
    OpChar          = '~' | '``' | '@' | '#' | '$' | '%' | '^' | '&'
                    | '-'| '+' | '=' | ' | ' | '\' | ':' | '<' | '>'
                    | '?' | '/'| '*' | '.' ;
    Oper            = (OpChar, {OpChar}) - '=' - '.' - ':' ;
    Id              = Letter, {Letter | Digit}, ['_', Oper] ;

    Expr            = PrefixExpr, {IdOrOperator, [PrefixExpr]} ;
    ExprNE          = PrefixExprNE, {IdOrOperatorNE, [PrefixExprNE]} ;
    PrefixExpr      = {'`', Id, '`' | Oper} SimpleExpr ;
    PrefixExprNE    = {'`', Id, '`' | Oper} SimpleExprNE ;
    SimpleExpr      = "..." ;
    SimpleExprNE    = "..." ;

According to the above grammar, ``.`` and ``:`` are never operator names, but can be part of operator names. In addition, ``=`` can be part of operator names and, but sometimes can be by itself an operator name (i.e., ``a = 10``), and sometimes it is not. The cases in which ``=`` is not an operator name, are the cases in which ``=`` cannot be part of an expression; this is the case with type expressions; i.e., ``a: Int = 3`` -- here ``=`` is a separator between a type expression and an initializer expression.

The following are valid examples of operators: ``+``, ``-``, ``++``, ``**``, ``#$``, ``=/=/=/=``. Valid operator names can be also: ``foo``, ``bar123``, ``_123``, ``oper_$#@``.

As can be seen from grammar, one can use identifiers as prefix operator names if they are enclosed into backticks.

Here are some more examples of operators, this time in context:
::

    a + b       // add a and b
    a + b * c   // add a to the product of b and c
    ++a++       // prefix ++ on a, then apply the postfix ++
    v1 dot v2   // dot product between v1 and v2
    `length` a  // calling length on variable a with prefix call notation
    1..100      // the numeric range between 1 and 100, exclusive
    @Int        // type representing a reference to Int

A nice property of the grammar, is the chaining of name operators. Here is an example of this technique:

::

    (1..) map \collatzSeq map \rangeSize takeWhile (fun s = s < 500) rootMeanSquare

In this example, ``..`` and ``rootMeanSquare`` are postfix operators. On the other hand, ``map``, ``takeWhile`` and ``<`` are infix operators. The above line has the following meaning: take the range of integers starting from 1 (pseudo-infinite range), map it with the ``collatzSeq`` function, then map the result to ``rangeSize`` function, then apply ``takeWhile`` operator with a lambda function, and finally apply ``rootMeanSquare`` as a postfix operator.

If we were not to have prefix operators, then reading an expression of the form ``a b c d e f`` is straight forward:

- all the elements on odd positions are operands (``a``, ``c`` and ``e``)
- all the elements on even positions are operator names (``b``, ``d`` and ``f``)
- if the expression has an odd number of elements, the last operator is a postfix operator; all the rest are infix operators

Prefix operators can be identified by observing pure operator names (symbols only) on odd positions -- symbols only identifiers cannot ever be operands. Whenever a prefix operator is encountered, the odd/even rule above is shifted with one position. For example, ``a b ++ c`` is interpreted as ``a b (++ c)``, and cannot be interpreted as ``(a b ++) c``, with ``++`` being an operand.

Defining operators
^^^^^^^^^^^^^^^^^^

Defining an operator is identical to defining a regular function. Example:
::

    datatype Complex {re, im: Float}
    fun + (x, y: Complex) = Complex(x.re+y.re, x.im+y.im)
    fun - (x: Complex) = Complex(-x.re, -x.im)

In this example, the ``+`` operator is a binary operator (can be used in infix operations) and ``-`` is an unary operator; this is inferred from the number of parameters. In this example, the ``-`` function can be used both for infix and prefix operator calls. To distinguish between prefix and postfix operations, user can add the ``pre_`` or ``post_`` prefix to the function name. Examples of prefix-only and postfix-only functions:
::

    fun pre_++ (x: Complex): @Complex { x+=1; return x; }
    fun post_++(x: Complex): Complex { var old=x; x+=1; return old; }


Please note that Sparrow does not impose a limit on the number of parameters to the functions with operator names. If such a function has too many parameters, it won't be able to be called with operator notation.

Operator lookup
---------------

A prefix operator ``oper_name arg`` can be reduced to an infix operator with a null operand: ``null oper_name arg``. Similarly, a postfix operator ``arg oper_name`` can be reduced to an infix operator with a null operand: ``arg oper_name null``.

We explain in this section the semantics of a simple infix operator call ``arg1 oper_name arg2``, possibly with a null argument.

To simplify the explanation of the operator lookup, it is convenient to separate out the processes involved in the operator lookup:

#. overall operator resolving
#. operator selection
#. overload procedure

Overall operator resolving may invoke operator selection once or multiple times; similarly, operator selection can invoke overload procedure once or multiple times.

Overall operator resolving
^^^^^^^^^^^^^^^^^^^^^^^^^^

This process will handle any operators that the compiler wants to handle directly, and it will provide the fallback cases for when the raw operator selection didn't succeed. The process is summarized by the following table:

.. csv-table:: overal operator resolving
   :header: "Condition", "Action"

   "op= ``===``, infix", "Handle reference equality directly"
   "op= ``!==``, infix", "Handle reference inequality directly"
   "op= ``:=``, infix", "Handle reference assignment directly"
   "op= ``\``, prefix", "Handle function pointer operator directly"
   "op= ``construct``, prefix", "Handle construct calls directly"
   "not handled yet", "**apply operator selection**"
   "not handled yet, ``a != b``", "attempt to transform into ``!(a == b)``"
   "not handled yet, ``a > b``", "attempt to transform into ``b < a``"
   "not handled yet, ``a <= b``", "attempt to transform into ``!(b < a)``"
   "not handled yet, ``a >= b``", "attempt to transform into ``!(a < b)``"
   "not handled yet, ``a <op>= b``", "attempt to transform into ``a = a <op> b``"
   "not handled yet, infix+postfix", "attempt to transform first arg into ``(a .)``"

The first entries in this table force the compiler to deal directly with the operators, rather than executing user-defined code; this is for speeding up certain common operations. Then, if these operations are not applied, the compiler will attempt to use the operator selection process to resolve the operator call; the vast majority of operator calls should be handled here. If this does not succeed, the compiler will attempt to handle some fall-back cases. It tries to infer inequality based on equality, it tries to infer other relational operators based on the definition of ``<``, and it tries to resolve oper-equals operators. The last rule is the application of the dot operator, in case nothing worked; this is only applied to the first argument of the operator.

If no action is successful, i.e., cannot find a proper operator or sequence of operators to call, an error is reported.

Operator selection
^^^^^^^^^^^^^^^^^^

Given two arguments (``<arg1>`` and ``<arg2>``), at least one non-null, and an operation name (``<oper>``), this process will attempt to find a way to call the operation with the given arguments.

We call the *base argument* the first non-null argument. This process will search around the datatype of the base arguments for matching functions.

If the operation is prefix, it will also consider searching with ``pre_<oper>``. If the operation is postfix, this will also consider searching with ``post_<oper>``. We call these search names ``<operWithPrefix>`` -- note, this may not be valid.

There are multiple contexts in which the selection process can search, and the compiler can search both with ``<oper>`` and ``<operWithPrefix>``. The compiler will attempt to perform the search, in order, according to the following list:

#. in the datatype of the base argument, using ``<operWithPrefix>`` (if valid)
#. in the datatype of the base argument, using ``<oper>``
#. in the package that contains the datatype of the base argument, using ``<operWithPrefix>`` (if valid)
#. in the package that contains the datatype of the base argument, using ``<oper>``
#. upward from the context of the operator call, using ``<operWithPrefix>`` (if valid)
#. upward from the context of the operator call, using ``<oper>``

At each step, the compiler will perform a name search in the appropriate context for the given name. If declarations are found, the overload procedure is invoked, trying to select the appropriate declaration that can be called.  If at one step a match is found, the compiler will not continue with the rest of the steps. If valid names are found, but no match is found, the compiler will continue with the next step. Please note that it may be ok for this process to fail; a follow-up step in the overall operator resolving process may succeed.

Example:
::

    package A
        datatype Foo
            a, b, c: Int

        fun f1(this: @Foo) {}
        fun f2(this: @Foo) {}

    fun oper(this: @A.Foo)
        cout << 'this will be selected' << endl

    package B
        fun g1(this: @A.Foo) {}
        fun g2(this: @A.Foo) {}

        fun test
            var x: A.Foo

            x oper  // searches 'post_oper' inside A.Foo (a,b,c)                -> FAIL
                    // searches 'oper' inside A.Foo (a,b,c)                     -> FAIL
                    // searches 'post_oper' inside A (f1,f2)                    -> FAIL
                    // searches 'oper' inside A (f1,f2)                         -> FAIL
                    // searches 'post_oper' up from current context (B, global) -> FAIL
                    // searches 'oper' up from current context (B, global)      -> SUCCESS


Overload procedure
^^^^^^^^^^^^^^^^^^

TODO: add more details


Precedence and associativity
----------------------------

For infix operators, we also need to consider precedence and associativity. Precedence determines the order in which different infix operators inside the same expression are called. Associativity determines whether for an expression containing only operators of the same type the order of applying the operator is from left to right, or from right to left.

For each infix operator we can associate a numeric value such that we can compare the precedence of two operators. Let us denote by :math:`p_1` the precedence of the operator ``op1`` and by :math:`p_2` the precedence of the operator ``op2``. Then, the expression ``A op1 B op2 C`` would be interpreted as ``(A op1 B) op2 C`` if :math:`p_1 \geq p_2`, and as ``A op1 (B op2 C)`` if :math:`p_1 < p_2`. For example, multiplication and division have higher precedence than addition and subtraction.

For an infix operator ``op``, an expression like ``A op B op C`` would be interpreted as ``(A op B) op C`` if ``op`` has left associativity, and ``A op (B op C)`` if ``op`` has right associativity. Most of the mathematical operators have left associativity, but an operation like assignment makes sense to have right associativity. Also, if one were to define an exponentiation operator, it should also have right associativity.

As Sparrow operators are defined in the library, precedence and associativity can also be defined in the library -- with ``using`` directives:
::

    using oper_precedence_default = 100
    using oper_precedence_+       = 500
    using oper_precedence_*       = 550
    using oper_assoc_=            = -1

Whenever the compiler needs to know the precedence of an operator ``<op>`` it will search for the using with the name ``oper_precedence_<op>`` and use its value. If this cannot be found, it will use the ``oper_precedence_default`` value.

Whenever the compiler needs to know the associativity of an operator ``<op>`` it will search for the using with the name ``oper_assoc_<op>``. If we found a value and it's negative, the compiler will use right associativity; otherwise it will use left associativity.

Example of using precedence and associativity for a new operator:
::

    fun **(x, y: Float)      = Math.pow(x, y)
    using oper_precedence_** = 1 + oper_precedence_* // higher precedence than multiplication
    using oper_assoc_**      = -1                    // right associativity

    cout << 4 * 3 ** 2 << endl      // 36 == 4 * (3**2)
    cout << 4 ** 3 ** 2 << endl     // 262144 == 4 ** (3**2)


The dot operator
----------------

The compiler uses a special ``.`` operator to ease the access for some datatypes. Consider the following example:
::

    package A
        [initCtor]
        datatype Ptr(type: Type)
            using ValueType = type

            _ptr: @ValueType
        fun get(this: Ptr): @ValueType = _ptr
        fun .(this: Ptr) = _ptr

    [initCtor]
    datatype MyObj
        x: Int
    fun print(this: @MyObj)
        cout << "MyObj.print: " << x << endl

    var p: A.Ptr(MyObj) = ...
    p.x = 10                    // OK
    p.print                     // OK
    p print                     // OK

If the dot operator would not be present, then the last 4 lines would not be valid anymore. The user would have been supposed to write:
::

    p.get().x = 30              // UGLIER
    p.get().print               // UGLIER
    p.get print                 // UGLIER

Defining a dot operator allows the overall operator resolving process to use the value returned by the dor operator to resolve the above expressions.

Formally, if ``X`` has a dot operator (i.e., ``(X .)`` would be valid), then if ``x`` is of type ``X`` (or derived), then ``x <op> y`` would fall back to ``(x .) <op> y``.

Please note that the syntax of Sparrow does not allow to write ``(x .)``. In general, ``.`` is not an operator name. However, the syntax of Sparrow was extended for dot operator to allow users to write ``fun .``.

In the above example we shown how dot operator works with operator calls, but also with compound expressions (things of the form ``a.b``). The main idea is the same, but the details are somehow different. In compound expressions we don't have overload resolution, so if find a name, the selection process stops successfully; this means the dot operator will not be tried.

This can generate some strange errors. In the above example, if we remove the package (making ``Ptr`` be contained in the same package as ``MyObj`` and ``print``), then the line ``p.print`` would result in an error. The ``print`` function is found near the ``Ptr`` class, the finding process stops, and then the result cannot be used, and thus an error is generated.



