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

#ifndef fahrenheit_backend_h
#define fahrenheit_backend_h

struct FModule;

/** Compiled function prototype */
typedef void (*FJitFunc)(void);

/** Store the functions from the module */
typedef struct FEngine {
  FJitFunc *funcs;
  int nfuncs;
  void *data;
} FEngine;

/** Initialize the engine */
void f_init_engine(FEngine *e);

/** Close the engine */
void f_close_engine(FEngine *e);

/** Compile the module and store the compiled functions into the engine
 *
 * The engine will not keep any references to the module.
 * Return a value different from 0 if there is an unexpected error. */
int f_compile(FEngine *e, struct FModule *m);

/** Obtain the function pointer given the type
 *
 * The parameter args should be inside a parenteses (eg. (void), (int, int)). */
#define f_get_fpointer(e, function, ret, args) \
  ((ret(*)args)(e.funcs[function]))

#endif

