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

-- Test call instruction

local test = require 'test'

test.preamble()

-- Call non existent function
test.case {
    success = false,
    functions = {{
        type = {'FVoid'},
        code = [[
            f_call(b, 1, 0);
        ]]
    }}
}

-- Call non existent function with an arg
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            f_call(b, 1, 2, v[0], v[0]);
        ]]
    }}
}

-- Call function with the wrong number of args
test.case {
    success = false,
    functions = {
    {
        type = {'FVoid', 'FInt32'},
        code = [[
            f_ret_void(b);
        ]]
    },
    {
        type = {'FVoid', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            f_call(b, f[0], 2, v[0], v[0]);
        ]]
    },
    }
}

-- Call function missing args
test.case {
    success = false,
    functions = {
    {
        type = {'FVoid', 'FInt32'},
        code = [[
            f_ret_void(b);
        ]]
    },
    {
        type = {'FVoid', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            f_call(b, f[0], 0);
        ]]
    },
    }
}

-- Call function with args of wrong type
test.case {
    success = false,
    functions = {
    {
        type = {'FVoid', 'FInt32'},
        code = [[
            f_ret_void(b);
        ]]
    },
    {
        type = {'FVoid', 'FInt64'},
        code = [[
            v[0] = f_getarg(b, 0);
            f_call(b, f[0], 1, v[0]);
        ]]
    },
    }
}

-- Call function with second arg with wrong type
test.case {
    success = false,
    functions = {
    {
        type = {'FVoid', 'FInt64', 'FInt32'},
        code = [[
            f_ret_void(b);
        ]]
    },
    {
        type = {'FVoid', 'FInt64'},
        code = [[
            v[0] = f_getarg(b, 0);
            f_call(b, f[0], 2, v[0], v[0]);
        ]]
    },
    }
}

-- Call function with no args
test.case {
    success = true,
    functions = {
    {
        type = {'FVoid'},
        code = [[
            f_ret_void(b);
        ]]
    },
    {
        args = {''},
        type = {'FVoid'},
        code = [[
            f_call(b, f[0], 0);
            f_ret_void(b);
        ]]
    },
    }
}

-- Call function with 1 arg
test.case {
    success = true,
    functions = {
    {
        type = {'FInt32', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            f_ret(b, v[0]);
        ]]
    },
    {
        args = {'1234'},
        ret = '1234',
        type = {'FInt32', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_call(b, f[0], 1, v[0]);
            f_ret(b, v[1]);
        ]]
    },
    }
}

-- Call function with args of different types
test.case {
    success = true,
    functions = {
    {
        type = {'FInt32', 'FInt64', 'FBool', 'FDouble', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 3);
            f_ret(b, v[0]);
        ]]
    },
    {
        args = {'0xFFFFFFEF', '1', '123.0123', '1234'},
        ret = '1234',
        type = {'FInt32', 'FInt64', 'FBool', 'FDouble', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_getarg(b, 1);
            v[2] = f_getarg(b, 2);
            v[3] = f_getarg(b, 3);
            v[4] = f_callv(b, f[0], 4, v);
            f_ret(b, v[4]);
        ]]
    },
    }
}

-- Call function with multiple args
test.case {
    success = true,
    functions = {
    {
        type = {'FInt32', 'FInt32', 'FInt32', 'FInt32', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_getarg(b, 1);
            v[2] = f_getarg(b, 2);
            v[3] = f_getarg(b, 3);
            v[4] = f_consti(b, 0, FInt32);
            v[4] = f_binop(b, FAdd, v[4], v[0]);
            v[4] = f_binop(b, FAdd, v[4], v[1]);
            v[4] = f_binop(b, FAdd, v[4], v[2]);
            v[4] = f_binop(b, FAdd, v[4], v[3]);
            f_ret(b, v[4]);
        ]]
    },
    {
        args = {'10'},
        ret = '40',
        type = {'FInt32', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_call(b, f[0], 4, v[0], v[0], v[0], v[0]);
            f_ret(b, v[1]);
        ]]
    },
    }
}

-- Recursive call
test.case {
    success = true,
    functions = {{
        args = {'10'},
        type = {'FVoid', 'FInt32'},
        code = [[
            bb[1] = f_add_bblock(&module, f[0]);
            bb[2] = f_add_bblock(&module, f[0]);

            v[0] = f_getarg(b, 0);
            v[1] = f_intcmp(b, FIntSLe, v[0], f_consti(b, 0, FInt32));
            f_jmpif(b, v[1], bb[1], bb[2]);

            b = f_builder(&module, f[0], bb[1]);
            f_ret_void(b);

            b = f_builder(&module, f[0], bb[2]);
            v[2] = f_binop(b, FSub, v[0], f_consti(b, 1, FInt32));
            f_call(b, f[0], 1, v[2]);
            f_ret_void(b);
        ]]
    }}
}

-- Call external function

-- Call external variadic function

test.epilog()

