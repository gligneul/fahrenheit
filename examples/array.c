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
 * Obtain the sum of an array
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <fahrenheit/fahrenheit.h>

int main(void) {
    FModule module;
    FModule *M = &module;
    FEngine engine;
    FEngine *E = &engine;
    int fn_sum, bb_init, bb_header, bb_loop, bb_exit;
    FValue i_init, i_curr, i_next, s_init, s_curr, s_next, array, size, elem,
           cond;
    FBuilder b;
    char err[FVerifyBufferSize] = {0};
    i32 (*sum)(void *, i32);
    int c_array[] = {1, 2, 3, 4, 5, 6};
    int c_array_size = sizeof(c_array) / sizeof(*c_array);

    f_init_module(M);

    fn_sum = f_add_function(M, f_ftype(M, FInt32, 2, FPointer, FInt32));
    bb_init = f_add_bblock(M, fn_sum);
    bb_header = f_add_bblock(M, fn_sum);
    bb_loop = f_add_bblock(M, fn_sum);
    bb_exit = f_add_bblock(M, fn_sum);

    b = f_builder(M, fn_sum, bb_init);
    array = f_getarg(b, 0);
    size = f_getarg(b, 1);
    i_init = s_init = f_consti(b, 0, FInt32);
    f_jmp(b, bb_header);

    b = f_builder(M, fn_sum, bb_header);
    i_curr = f_phi(b, FInt32);
    s_curr = f_phi(b, FInt32);
    cond = f_intcmp(b, FIntSLt, i_curr, size);
    f_jmpif(b, cond, bb_loop, bb_exit);

    b = f_builder(M, fn_sum, bb_loop);
    elem = f_arr_get(b, i32, array, i_curr, FInt32);
    s_next = f_binop(b, FAdd, s_curr, elem);
    i_next = f_binop(b, FAdd, i_curr, f_consti(b, 1, FInt32));
    f_jmp(b, bb_header);

    b = f_builder(M, fn_sum, bb_exit);
    f_ret(b, s_curr);

    f_add_incoming(b, i_curr, bb_init, i_init);
    f_add_incoming(b, i_curr, bb_loop, i_next);

    f_add_incoming(b, s_curr, bb_init, s_init);
    f_add_incoming(b, s_curr, bb_loop, s_next);

    /* Verify the generated IR */
    if(f_verify_module(M, err)) {
      fprintf(stderr, "%s\n", err);
      exit(1);
    }
    f_init_engine(E);
    f_compile(E, M);
    sum = f_get_fpointer(E, fn_sum, i32, (void *, i32));
    f_close_module(M);

    /* Obtain the number from argv */
    printf("%d\n", sum(c_array, c_array_size));

    /* Clean up */
    f_close_engine(E);

    return 0;
}


