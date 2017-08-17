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
 * Create a for loop that print the index
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <fahrenheit/fahrenheit.h>

int main(int argc, char *argv[]) {
    FModule module;
    FModule *M = &module;
    FEngine engine;
    FEngine *E = &engine;
    int n, t_printf, fn_printf, fn_loop, bb_init, bb_header, bb_loop, bb_exit;
    FValue i_init, i_curr, i_next, cond;
    FBuilder b;
    char err[FVerifyBufferSize] = {0};
    void (*loop)(i32);

    f_init_module(M);

    t_printf = f_ftype(M, FVoid, 1, FPointer);
    f_set_vararg(M, t_printf);
    fn_printf = f_add_extfunction(M, t_printf, (FFunctionPtr)printf);

    fn_loop = f_add_function(M, f_ftype(M, FVoid, 1, FInt32));
    bb_init = f_add_bblock(M, fn_loop);
    bb_header = f_add_bblock(M, fn_loop);
    bb_loop = f_add_bblock(M, fn_loop);
    bb_exit = f_add_bblock(M, fn_loop);

    b = f_builder(M, fn_loop, bb_init);
    i_init = f_getarg(b, 0);
    f_jmp(b, bb_header);

    b = f_builder(M, fn_loop, bb_header);
    i_curr = f_phi(b, FInt32);
    cond = f_intcmp(b, FIntSGt, i_curr, f_consti(b, 0, FInt32));
    f_jmpif(b, cond, bb_loop, bb_exit);

    b = f_builder(M, fn_loop, bb_loop);
    f_call(b, fn_printf, 2, f_constp(b, "%d\n"), i_curr);
    i_next = f_binop(b, FSub, i_curr, f_consti(b, 1, FInt32));
    f_jmp(b, bb_header);

    b = f_builder(M, fn_loop, bb_exit);
    f_ret_void(b);

    f_add_incoming(b, i_curr, bb_init, i_init);
    f_add_incoming(b, i_curr, bb_loop, i_next);

    /* Verify the generated IR */
    if(f_verify_module(M, err)) {
      fprintf(stderr, "%s\n", err);
      exit(1);
    }
    f_init_engine(E);
    f_compile(E, M);
    loop = f_get_fpointer(E, fn_loop, void, (i32));
    f_close_module(M);

    /* Obtain the number from argv */
    if (argc != 2 || sscanf(argv[1], "%d", &n) != 1) {
        printf("this program require an integer argument\n");
        exit(1);
    }
    loop(n);

    /* Clean up */
    f_close_engine(E);

    return 0;
}

