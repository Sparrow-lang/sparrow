.. highlight:: bash

Quickstart
==========

Compiling the compiler
----------------------

External dependencies:
^^^^^^^^^^^^^^^^^^^^^^

- CMake (3.9.0 or later)
- Boost (1.54 or later)
- LLVM 5.0.0 or 5.0.1

Building on Mac or Unix-like platforms
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Ensure you have all the dependencies installed. The typical way is to install them from a package manager (brew, apt, etc.).

Create a "build" directory under the Sparrow trunk folder and go there ($sprDir/build). Then run the following commands
::

    cmake ..
    cmake --build .

Depending on your system, CMake may not find a proper version of LLVM, and it will fail the first step; you'll see this appear in the CMake output. It may be required for you to pass the path to the CMake files of the LLVM installation. Here is an example of passing that directory:
::

    cmake .. -DLLVM_DIR=/usr/local/Cellar/llvm/5.0.1/lib/cmake/llvm

After these steps, optionally, you can also install it:
::

    cmake --build . -- install


Compiling Sparrow compiler on Windows 64-bit
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Not tried since a long while. Expect errors.

Running the tests
^^^^^^^^^^^^^^^^^

Go into "test" folder of Sparrow distribution and run:
::

    ./test-all.pl

the tests should run successfully (at least the vast majority of them)

Using docker
^^^^^^^^^^^^

Get the ``sparrowlang/sparrow`` docker image. This contains a pre-compiled Sparrow installation inside it.
Try running:
::

    SparrowCompiler --help

Then, to run tests you can run:
::

    ./test-all.pl


