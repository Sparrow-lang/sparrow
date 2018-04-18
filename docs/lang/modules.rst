.. highlight:: sparrow

Modules
=======

Modules represent Sparrow source code files to be compiled; to some degree, they resemble packages. Besides representing the content of a source file, a module has a name. Modules can be grouped together in hierarchies, just like packages do.

To indicate the module name, the source code would start with a module declaration:
::

    module myModule
    // actual source code content


If we were to image the source code without modules, this would be equivalent with:
::

    package myModule {
        // actual source code content
    }


The name of the module might be qualified, indicating the hierarchy in which the module lies:
::

    module company.sysA.subSysA1.myModule2


This would be equivalent with:
::

    package company { package sysA { package subSysA1 { package myModule2 {
        // actual source code content
    } } } }


If a source code does not contain module declaration, the name of the module is computed from the name of the file, removing the extension and any path.

In practice we often construct programs with more than one modules, i.e., split the code into several files. Therefore, the declarations of the program are split across different modules. One can make the declarations of one module available to other modules by using import declarations:
::

    package myOtherModule
    import myModule
    import company.sysA.subSysA1.myModule2


Syntax
------

.. code-block:: ebnf

    SourceFile      = [Module] {TopLevel} ;
    Module          = 'module' QualifiedId ';' ;
    TopLevel        = ImportList (* | ...*) ;
    ImportLine      = [AccessSpec] 'import' ImportName {',' ImportName} ';' ;
    ImportName      = [[Id] '=' ] (QualifiedId | String) ['(' {Id} ')'] ;
    QualifiedId     = Id {'.' Id} ;


Modules semantics
-----------------

A module is always created for each source-code. The name of the module can be specified with the ``module`` syntax, at the beginning of the source file.

The module name is used only to organize the code inside the module; it has no significance for any other module that may import that module. Having multiple modules with the same module name can lead to name clashing for the symbols defined in those modules; these clashes can be linker errors.

The module name can be completely independent of the filename or its position on disk.

The code generation for a module is independent of other modules that the program might have.


Import semantics
----------------

As part of the body of a module, a Sparrow source code might contain a series of import lines, each containing one or multiple import names. Having all import names in only one import line, or having them spread our across multiple lines doesn't make any difference. If a module is imported several times into the same module, in the same package, the compilation behaves as if the module was imported only once.

For most practical uses, the order of the import lines don't matter, if they are placed in the same package. When the imported modules have associated compile-time functionality that should run when the module is imported, then, of course the order of import lines matter.

An import name, must correspond to an existing module. The purpose of the import name is to tell the compiler how to find the file containing the module to be imported. If the compiler cannot find the corresponding file, an error is issued.

Depending on the import name specification, the compiler will perform different types of file/directory search:

- If the import name is a string, a file with that exact name is searched for; the file extension is assumed to be present in the string; relative or absolute paths might also be present.
- If the import name is a qualified id with only one name, then the compiler assumes this is the name of the file without extension, and, adding the default extension, will search for the file.
- If the import name is a qualified id with multiple names, then the compiler will assume that the first names will correspond to a directories path, while the last name corresponds to the file name (again without extension)

After the import line, the actual import name is not used anymore.

If we have three modules like the following:
::

    module he;
    fun hello = "Hello, "


::

    module wo
    fun world = "world!"

::

    module greet
    import he, wo
    fun greet
        cout << hello << world << endl

The effect of compiling the `greet` module would be similar to:
::

    package toplevel_anon_1
        package he
            fun hello  // not implemented in greet module
    package toplevel_anon_2
        package wo
            fun world  // not implemented in greet module
    package toplevel_anon_3
        package greet
            using toplevel_anon_1.he.*
            using toplevel_anon_2.wo.*
            fun greet
                cout << hello << world << endl

As shown in the above example, the compiler may create anonymous top-level packages to make sure that independent modules are not affecting one another. Also, the compiler will not redefine in the `greet` modules the functions that are already defined in the `he` and `wo` modules.




Named imports
-------------

By default, all the imported declarations from one module can be referred directly in the module that does the import. However, importing declarations from multiple modules can lead to name clashes. We can solve this by specifying a name at the import line, name that would be later be used for accessing the declarations.

Here is an example of doing a named import:
::

    module dbTest
    import db = storage.sql.database
    ...
    db.open(...)


In the above example, the import line corresponds the following expansion:
::

    package toplevel_anon_1
        package storage
            package sql
                package database
                    ...
    package toplevel_anon_2
        package dbTest
            using db = toplevel_anon_1.storage.db.database
            ...
            db.open(...)


The same effect can be achieved by placing the import line into a package:
::

    module dbTest;
    package db
        import storage.sql.database
    ...
    db.open(...)



Private and public imports
--------------------------

By default, if module `A` imports module `B`, which in turn imports module `C`, then `A` will not see the declarations from `C`. We say that the *imports are private* to that module.

There are however cases in which we want to make all the exports of a module to be public to the modules that import that module, i.e., making the *import public*. A good example is when we create a module as an interface for a component that spreads across different modules.

To make an import line public, one must add the `public` access specifier:
::

    module MyComponent
    [public] import Foo
    [public] import Bar



Selective imports
-----------------

It is often considered a best practice to import only the needed declarations from a module, and not all the declarations in that module. One can follow this practice and specify the names of declarations to be imported, as suggested by the following example:
::

    module foo
    fun f = 1
    fun g = 2
    fun h = 3


::

    module bar
    import foo(f, g)
    ...
    f()    // ok
    g()    // ok
    h()    // ERROR


The import line from the above example would be translated similar to:
::

    package toplevel_anon_1
        package foo
            fun f = 1
            fun g = 2
            fun h = 3
    package toplevel_anon_2
        package bar
            using f = toplevel_anon_1.foo.f
            using g = toplevel_anon_1.foo.g
            ...
            f()    // ok - resolves to toplevel_anon_1.foo.f
            g()    // ok - resolves to toplevel_anon_1.foo.g
            h()    // ERROR


.. note:: The compiler might compile all the declaration from `foo`, to make sure that everything functions properly; think of auxiliary types and using declarations. This syntax just makes sure that `bar` cannot access the non-specified declarations directly by name. Some compilers might also use this information to reduce processing.

