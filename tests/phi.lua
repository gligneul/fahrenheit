-- MIT License
-- 
-- Copyright (c) 2017 Gabriel de Quadros Ligneul
-- 
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to
-- deal in the Software without restriction, including without limitation the
-- rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
-- sell copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
-- 
-- The above copyright notice and this permission notice shall be included in
-- all copies or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
-- FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
-- IN THE SOFTWARE.

-- Test phi instruction

local test = require 'test'

test.preamble()

-- Phi in the first block
test.case {
    success = false,
    functions = {{
        type = {'FVoid'},
        code = [[
            v[0] = f_phi(b, FInt32);
        ]]
    }}
}

-- Phi after instruction
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32'},
        code = [[
            bb[1] = f_add_bblock(&module, f[0]);

            v[0] = f_getarg(b, 0);
            f_jmp(b, bb[1]);

            b = f_builder(&module, f[0], bb[1]);
            v[1] = f_binop(b, FAdd, v[0], v[0]);
            v[2] = f_phi(b, FInt32);
        ]]
    }}
}

-- Phi of different type
test.case {
    success = false,
    functions = {{
        type = {'FInt32', 'FInt32'},
        code = [[
            bb[1] = f_add_bblock(&module, f[0]);

            v[0] = f_getarg(b, 0);
            f_jmp(b, bb[1]);

            b = f_builder(&module, f[0], bb[1]);
            v[1] = f_phi(b, FInt64);
            f_add_incoming(b, v[1], bb[0], v[0]);
            f_ret(b, v[1]);
        ]]
    }}
}

-- Simple phi
test.case {
    success = true,
    functions = {{
        type = {'FInt32', 'FInt32'},
        args = {'123'},
        ret = '123',
        code = [[
            bb[1] = f_add_bblock(&module, f[0]);

            v[0] = f_getarg(b, 0);
            f_jmp(b, bb[1]);

            b = f_builder(&module, f[0], bb[1]);
            v[1] = f_phi(b, FInt32);
            f_add_incoming(b, v[1], bb[0], v[0]);
            f_ret(b, v[1]);
        ]]
    }}
}

-- Multiple phi values
test.case {
    success = true,
    functions = {{
        type = {'FInt32', 'FInt32', 'FInt32'},
        args = {'3', '5'},
        ret = '8',
        code = [[
            bb[1] = f_add_bblock(&module, f[0]);

            v[0] = f_getarg(b, 0);
            v[1] = f_getarg(b, 1);
            f_jmp(b, bb[1]);

            b = f_builder(&module, f[0], bb[1]);
            v[2] = f_phi(b, FInt32);
            f_add_incoming(b, v[2], bb[0], v[0]);
            v[3] = f_phi(b, FInt32);
            f_add_incoming(b, v[3], bb[0], v[1]);
            v[4] = f_binop(b, FAdd, v[2], v[3]);
            f_ret(b, v[4]);
        ]]
    }}
}

-- Phi if
test.case {
    success = true,
    functions = {{
        type = {'FInt32', 'FBool'},
        args = {'123'},
        ret = 0,
        code = [[
            bb[1] = f_add_bblock(&module, f[0]);
            bb[2] = f_add_bblock(&module, f[0]);
            bb[3] = f_add_bblock(&module, f[0]);

            v[0] = f_getarg(b, 0);
            f_jmpif(b, v[0], bb[1], bb[2]);

            b = f_builder(&module, f[0], bb[1]);
            v[1] = f_consti(b, 0, FInt32);
            f_jmp(b, bb[3]);

            b = f_builder(&module, f[0], bb[2]);
            v[2] = f_consti(b, 1, FInt32);
            f_jmp(b, bb[3]);

            b = f_builder(&module, f[0], bb[3]);
            v[3] = f_phi(b, FInt32);
            f_add_incoming(b, v[3], bb[1], v[1]);
            f_add_incoming(b, v[3], bb[2], v[2]);
            f_ret(b, v[3]);
        ]]
    }}
}

-- Phi loop
test.case {
    success = true,
    functions = {{
        type = {'FInt32', 'FInt32'},
        args = {'10'},
        ret = 10,
        code = [[
            bb[1] = f_add_bblock(&module, f[0]);
            bb[2] = f_add_bblock(&module, f[0]);
            bb[3] = f_add_bblock(&module, f[0]);

            v[10] = f_getarg(b, 0);
            v[20] = f_consti(b, 0, FInt32);
            f_jmp(b, bb[1]);

            /* loop header */
            b = f_builder(&module, f[0], bb[1]);
            v[11] = f_phi(b, FInt32);
            v[21] = f_phi(b, FInt32);
            v[0] = f_intcmp(b, FIntSGt, v[11], f_consti(b, 0, FInt32));
            f_jmpif(b, v[0], bb[2], bb[3]);

            /* loop body */
            b = f_builder(&module, f[0], bb[2]);
            v[12] = f_binop(b, FSub, v[11], f_consti(b, 1, FInt32));
            v[22] = f_binop(b, FAdd, v[21], f_consti(b, 1, FInt32));
            f_jmp(b, bb[1]);

            /* return */
            b = f_builder(&module, f[0], bb[3]);
            f_ret(b, v[21]);

            f_add_incoming(b, v[11], bb[0], v[10]);
            f_add_incoming(b, v[11], bb[2], v[12]);

            f_add_incoming(b, v[21], bb[0], v[20]);
            f_add_incoming(b, v[21], bb[2], v[22]);
        ]]
    }}
}

test.epilog()

