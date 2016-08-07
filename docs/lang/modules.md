# Modules

Modules represent Sparrow source code files to be compiled; to some degree, they resemble packages. Besides representing the content of a source file, a module has a name. Modules can be grouped together in hierarchies, just like packages do.

To indicate the module name, the source code would start with a module declaration:
```
module myModule;
// actual source code content
```

If we were to image the source code without modules, this would be equivalent with:
```
package myModule {
    // actual source code content
}
```

The name of the module might be qualified, indicating the hierarchy in which the module lies:
```
module company.sysA.subSysA1.myModule2;
```

This would be equivalent with:
```
package company { package sysA { package subSysA1 { package myModule2 {
    // actual source code content
} } } }
```

If a source code does not contain module declaration, the name of the module is computed from the name of the file, removing the extension and any path.

In practice we often construct programs with more than one modules, i.e., split the code into several files. Therefore, the declarations of the program are split across different modules. One can make the declarations of one module available to other modules by using import declarations:
```
package myOtherModule;
import myModule;
import company.sysA.subSysA1.myModule2;
```

## Syntax
```ebnf
SourceFile      ::= [Module] {ImportLine} {TopLevel}
Module          ::= 'module' QualifiedId ';'
ImportLine      ::= [AccessSpec] 'import' ImportName {',' ImportName} ';'
ImportName      ::= [[Id] '=' ] (QualifiedId | String) ['(' {ImportDecl} ')']
ImportDecl      ::= [Id '=' ] (Id)
QualifiedId     ::= Id {'.' Id}
```

## Modules semantics
A module is always created for each source-code. The name of the module can be specified with the "Module" syntax, at the beginning of the source file.

If the qualified id contains only one part, then this part is the name of the module. If it contains multiple parts, the last part corresponds to the name of the module, while the rest of the parts correspond to the package hierarchy above the module.

The module name can be completely independent of the filename or its position on disk.

The code generation for a module is independent of other modules that the program might have.


## Import semantics

After the optional module specification, a Sparrow source code might contain a series of import lines, each containing one or multiple import names. Having all import names in only one import line, or having them spread our across multiple lines doesn't make any difference. Also, the order in which the import occur has no significance. If a module is imported several times into the same module, the compilation behaves as if the module was imported only once.

An import name, must correspond to an existing module. The purpose of the import name is to tell the compiler how to find the file containing the module to be imported. If the compiler cannot find the corresponding file, an error is issued.

Depending on the import name specification, the compiler will perform different types of file/directory search:
- If the import name is a string, a file with that exact name is searched for; the file extension is assumed to be present in the string; relative or absolute paths might also be present.
- If the import name is a qualified id with only one name, then the compiler assumes this is the name of the file without extension, and, adding the default extension, will search for the file.
- If the import name is a qualified id with multiple names, then the compiler will assume that the first names will correspond to a directories path, while the last name corresponds to the file name (again without extension)

The qualified id for importing a module should match (at least partially) the imported module name.

After the import line, the actual import name is not used anymore. Any references to the fully qualified name of the imported declarations are made using the module name of the imported file.

If we have three modules like the following:
```
module he;
fun hello = "Hello, ";
```
```
module wo;
fun world = "world!";
```
```
module greet;
import he, wo;
fun greet { cout << he.hello << wo.world << endl; }
```
The effect of compiling the "greet" module would be similar to:
```
package toplevel_anon_1 {
    package he {
        fun hello = "Hello, ";
    }
}
package toplevel_anon_2 {
    package wo {
        fun world = "world!";
    }
}
package toplevel_anon_3 {
    package greet {
        using he = toplevel_anon_1.he;
        using wo = toplevel_anon_2.wo;
        fun greet { cout << he.hello << wo.world << endl; }
    }
}
```
As shown in the above example, the compiler may create anonymous top-level packages to make sure that independent modules are not affecting one another.

When using declarations from one module, by default the full qualified names must be used. In this example, we use `he.hello` and `wo.world` to refer to the declarations found in other modules. If the module to be imported has multiple names, the user will have to use all the names when qualifying imported declarations:


While importing a module, the compiler will try to recompile as little as possible from the imported module.
```
module dbTest;
import storage.sql.database;
...
storage.sql.database.open(...);
```

The compiler will treat this as following:
```
package toplevel_anon_1 {
    package storage {
        package sql {
            package database {
                ...
            }
        }
    }
}
package toplevel_anon_2 {
    package dbTest {
        package storage {
            package sql {
                using database = toplevel_anon_1.storage.db.database;
            }
        }
        ...
        storage.sql.database.open(...);
    }
}
```



## Renamed imports
Sometimes it is useful to change the name of the imported module, just as we can rename declarations in an using statement. Among the most popular reasons for renaming modules are: the module name is too long and giving a more significant name to the imported module in the context of the new module.

Here is an example of doing a renamed import:
```
module dbTest;
import db = storage.sql.database;
...
db.open(...);
```

In the above example, the import line corresponds the following expansion:
```
package toplevel_anon_1 {
    package storage {
        package sql {
            package database {
                ...
            }
        }
    }
}
package toplevel_anon_2 {
    package dbTest {
        using db = toplevel_anon_1.storage.db.database;
        ...
        db.open(...);
    }
}
```



## Non-qualified imports

Sometimes it's more convenient to lose the import qualifier completely when using declarations from an  imported module. Moreover, for operators, losing the qualifier is crucial.

To lose the qualifier, one use the `=` symbol in the import name, but don't write any identifier. For example:
```
module helloWorldMessages;
fun hello = "Hello, ";
fun world = "world!";
```
```
module greet;
import =helloWorldMessages;
fun greet { cout << hello << world << endl; }
```

A typical include of the form will allow the module that does the importing to use declarations from this module, without fully-qualifying them:
```
include MyLib.MyMod1;
...
funFromImportedMod();
```

If we add the `[nonUsing]` modifier to the include line, we force the client to use fully qualified names for referring the imported declarations:
```
include[nonUsing] MyLib.MyMod1;
...
funFromImportedMod();               // ERROR
MyLib.MyMod1.funFromImportedMod();  // ok
```

This is particularly useful when multiple modules use the same name for different declarations, and we want to avoid confusion on the caller side. Here is an example:
```
module A;
fun foo = 1;
```

```
module B;
fun foo = 2;
```

```
module C;
import A, B;
...
foo()       // ERROR
A.foo();    // ok
B.foo();    // ok
```


## Private and public imports
By default, if module `A` imports module `B`, which in turn imports module `C`, then `A` will not see the declarations from `C`. We say that the *imports are private* to that module.

There are however cases in which we want to make all the exports of a module to be public to the modules that import that module, i.e., making the *import public*. A good example is when we create a module as an interface for a component that spreads across different modules.

To make an import line public, one must add the `public` access specifier:
```
module MyComponent;
public import Foo;
public import Bar;
```


## Selective imports
Sometimes it is considered a best practice to import only the needed declarations from a module, and not all the declarations in that module. One can specify the names of declarations to be imported, as suggested by the following example:
```
module foo;
fun f = 1;
fun g = 2;
fun h = 3;
```

```
module bar;
import foo(f, g);
...
foo.f();    // ok
foo.g();    // ok
foo.h();    // ERROR
```

The import line from the above example would be translated similar to:
```
package toplevel_anon_1 {
    package foo {
        fun f = 1;
        fun g = 2;
        fun h = 3;
    }
}
package toplevel_anon_2 {
    package bar {
        fun world = "world!";
    }
}
package toplevel_anon_2 {
    package bar {
        package foo {
            using f = toplevel_anon_1.foo.f;
            using g = toplevel_anon_1.foo.g;
        }
        ...
        foo.f();    // ok
        foo.g();    // ok
        foo.h();    // ERROR
    }
}
```

> Note: the compiler might compile all the declaration from `foo`, to make sure that everything functions properly; think of auxiliary types and using declarations. This syntax just makes sure that `bar` cannot access the non-specified declarations. Some compilers might also use this information to reduce processing.

Selective imports can be combined with the renaming feature. We can have renaming both at the package name level, or at declaration level. For example:
```
module bar;
import f = foo(ff = f, gg = g);
...
f.ff();
f.gg();
```
