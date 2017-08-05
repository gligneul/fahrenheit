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
 * Create a function that add 1 to an integer
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <fahrenheit/fahrenheit.h>

int main(int argc, char *argv[]) {
    FModule module;
    FModule *M = &module;
    int n, fn_add, bb;
    FValue v_x, v_y;
    FBuilder b;
    FEngine engine;
    FEngine *E = &engine;
    char err[FVerifyBufferSize] = {0};
    i32 (*add)(i32);

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

    /* Verify the generated IR */
    if(f_verify_module(M, err)) {
      fprintf(stderr, "%s\n", err);
      exit(1);
    }

    /* Compile the module */
    f_init_engine(E);
    f_compile(E, M);

    /* Obtain the function */
    add = f_get_fpointer(E, 0, i32, (i32));

    /* You can already delete the IR */
    f_close_module(M);

    /* Obtain the number from argv and call add*/
    if (argc != 2 || sscanf(argv[1], "%d", &n) != 1) {
        printf("this program require an integer argument\n");
        exit(1);
    }

    /* Run it */
    printf("%d\n", add(n));

    /* Engine clean up */
    f_close_engine(E);

    return 0;
}

