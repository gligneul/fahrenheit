/*
 * MIT License
 * 
 * Copyright (c) 2017 Gabriel de Quadros Ligneul
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef fahrenheit_fahrenheit_h
#define fahrenheit_fahrenheit_h

/** @file fahrenheit.h
    @brief Include all Fahrenheit headers.

@mainpage

# Getting started

Fahrenheit is a run-time code generator for JIT compilers.
Currently, Fahrenheit uses the LLVM toolchain to generate machine code during
run-time.
Fahrenheit offers a ANSI C89 compliant API that encapsulates the
The API is simple and consise, with a few lines of code you can write a simple
program that will be compiled during run-time:

```
f_init_module(&m);
```

# Installation

Compiling Fahrenheit is easy, but first you need to download and install few
dependencies.

## Installing Dependencies

### C and C++ compiler

Fahrenheit is a pure C library, but it requires a C++ compiler during the build
phase.
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

If for some reason your cmake is out dated, you can download the source code and
compile it from: https://cmake.org/download/.

### LLVM

Fahrenheit uses LLVM to generate code at runtime.
You should download it directely from LLVM release page, instead of using the
available on your package manager.
The following commands will download and unpack LLVM-3.9 at your home folder.

```
cd ~
wget http://releases.llvm.org/3.9.1/clang+llvm-3.9.1-x86_64-linux-gnu-ubuntu-14.04.tar.xz
unxz clang+llvm-3.9.1-x86_64-linux-gnu-ubuntu-14.04.tar.xz
tar xfv clang+llvm-3.9.1-x86_64-linux-gnu-ubuntu-14.04.tar
mv clang+llvm-3.9.1-x86_64-linux-gnu-ubuntu-14.04 llvm-3.9
```

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
cmake -DLLVM_DIR="$HOME/llvm-3.9/lib/cmake/llvm/" ..
make -j4
sudo make install
```

## Installing Fahrenheit at a custom location

TODO

# Examples

TODO

# Documentation

TODO

# Develpment

By default, Fahrenheit build will generate only the library, no tests included.
If you want to generate the tests, you must include the FAHRENHEIT_DEV flag
during cmake generation.

```
cmake -DFAHRENHEIT_DEV=1 -DLLVM_DIR="$HOME/llvm-3.9/lib/cmake/llvm/" ..
```

*/

#include <fahrenheit/backend.h>
#include <fahrenheit/instructions.h>
#include <fahrenheit/ir.h>
#include <fahrenheit/printer.h>
#include <fahrenheit/util.h>
#include <fahrenheit/verify.h>
#include <fahrenheit/version.h>

#endif

