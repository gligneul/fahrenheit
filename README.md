# Fahrenheit :fire:

[![Build Status](https://travis-ci.org/gligneul/fahrenheit.svg?branch=master)](https://travis-ci.org/gligneul/fahrenheit)

# Getting started

Fahrenheit is a run-time code generator for JIT compilers.
Currently, Fahrenheit uses the LLVM toolchain to generate machine code during
run-time.
Fahrenheit offers a ANSI C89 compliant API that encapsulates the C++ mess of
LLVM.
The API is simple and consise, with a few lines of code you can write a simple
function that will be compiled during run-time:

```
/* Create a module */
f_init_module(M);

/* Add a function to the module */
fn_add = f_add_function(M, f_ftype(M, FInt32, 1, FInt32));

/* Create a basic block */
bb = f_add_bblock(M, fn_add);

/* Add instructions to the basic block */
b = f_builder(M, fn_add, bb);
v_x = f_getarg(b, 0);
v_y = f_binop(b, FAdd, v_x, f_consti(b, 1, FInt32));
f_ret(b, v_y);

/* Compile the module */
f_init_engine(E);
f_compile(E, M);

/* Obtain the function */
add = f_get_fpointer(E, 0, i32, (i32));

/* Run it */
printf("%d\n", add(n));
```

Checkout the full example at `examples/add.c`.

# Installation

Compiling Fahrenheit is easy, but first you need to download and install few
dependencies.

## Installing Dependencies

### C and C++ compiler

Fahrenheit provides a pure C interface, but it requires a C++ compiler during
its the build phase.
I recommend using g++:

```
sudo apt-get install gcc g++
```

### Cmake

Fahrenheit use cmake as a build system. The version from your package manager
should work:

```
sudo apt-get install cmake
```

If for some reason your cmake is outdated, you can download the source code and
compile it from: https://cmake.org/download/.

### LLVM

Fahrenheit uses LLVM to generate code at runtime. Download the version 3.9 from
your package manager:

```
sudo apt-get install llvm-3.9-dev
```

If your distro is outdated and doesn't have this version, you should download
LLVM directely from their release page: http://releases.llvm.org/.

And if you need to compile LLVM from source (it's not that hard), check out:
https://llvm.org/docs/GettingStarted.html.

## Compiling Fahrenheit

After installing all dependencies you will be able to compile fahrenheit.
You should clone the repository, download the submodules, create a build
directory, generate the makefiles with cmake and then compile it.
The following commands will do all those steps:

```
git clone https://github.com/gligneul/fahrenheit.git
cd fahrenheit
git submodule update --init --recursive
mkdir build
cd build
cmake ..
make -j4
sudo make install
```

### Useful CMake arguments:

- `-DLLVM_DIR="$HOME/llvm-3.9/lib/cmake/llvm/"`: use a LLVM from a custom folder
  (eg. your home).
- `-DCMAKE_INSTALL_PREFIX="$HOME/fahrenheit`: install Fahrenheit at a custom
  location.
- `-DCMAKE_BUILD_TYPE=Debug`: Useful if you want to debug Fahrenheit itself.

# Tests

You should use the cmake in the `tests` folder to build the tests:

```
cd tests 
mkdir build
cd build
cmake ..
make
make test
```

# Examples

Checkout the examples in the examples directory.
Run the following commands to build and run them:

```
cd examples
mkdir build
cd build
cmake ..
make
./hello
```

# Documentation

The full documentation for each module is available at:
https://gligneul.github.io/fahrenheit/modules.html.

