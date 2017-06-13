# fahrenheit [![Build Status](https://travis-ci.org/gligneul/fahrenheit.svg?branch=master)](https://travis-ci.org/gligneul/fahrenheit)
Intermediate representation and backend for JIT compilers

## Sample

TODO

## Compilation

You must download the submodules before compiling.

```
git submodule update --init --recursive
mkdir build; cd build
cmake -DCMAKE_INSTALL_PREFIX=custom/install/dir ..
make
make install
```

## Running the tests

Just add the ENABLE_FAHRENHEIT_TESTS to the cmake build.

```
mkdir build; cd build
cmake -DENABLE_FAHRENHEIT_TESTS=1 ..
make
make test
```

