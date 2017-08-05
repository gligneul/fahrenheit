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

/*
 * Create a function that print hello, world
 */

#include <stdio.h>
#include <stdlib.h>

#include <fahrenheit/fahrenheit.h>

int main(void) {
    FModule module;
    FModule *M = &module;
    int t_printf, fn_printf, t_hello, fn_hello, bb;
    FValue v_str;
    FBuilder b;
    FEngine engine;
    FEngine *E = &engine;
    char err[FVerifyBufferSize] = {0};
    void (*hello)(void);

    f_init_module(M);

    /* Add printf to IR */
    t_printf = f_ftype(M, FVoid, 1, FPointer);
    f_set_vararg(M, t_printf);
    fn_printf = f_add_extfunction(M, t_printf, (FFunctionPtr)printf);

    /* Create the function that will be JITed */
    t_hello = f_ftype(M, FVoid, 0);
    fn_hello = f_add_function(M, t_hello);

    /* Call printf from hello */
    bb = f_add_bblock(M, fn_hello);
    b = f_builder(M, fn_hello, bb);
    v_str = f_constp(b, "Hello world!\n");
    f_call(b, fn_printf, 1, v_str);
    f_ret_void(b);

    /* Build the module */
    if(f_verify_module(M, err)) {
      fprintf(stderr, "%s\n", err);
      exit(1);
    }
    f_init_engine(E);
    f_compile(E, M);
    f_close_module(M);

    /* Obtain the function and run it */
    hello = f_get_fpointer(E, 1, void, (void));
    hello();

    /* Clean up */
    f_close_engine(E);

    return 0;
}

