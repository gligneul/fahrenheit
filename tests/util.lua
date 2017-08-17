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

-- Test utility functions

local test = require 'test'

-- declare a structure to be used in the tests
decls = [[
typedef struct Data {
    ui32 a;
    float b;
    ui8 c;
    void *d;
} Data;

int dummy = 0;
const ui32 _a = 0xFFFFFFEF;
const float _b = 999.999;
const ui8 _c = 0xFE;
const void *_d = &dummy;
const Data _data = {0xFFFFFFEF, 999.999, 0xFE, &dummy};
]]

test.preamble(decls)

-- Test array offset
test.case {
    success = true,
    decls = 'i32 arr[] = {1, 2, 3};',
    functions = {{
        type = {'FPointer', 'FPointer'},
        args = {'arr'},
        ret = 'arr + 2',
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_arr_offset(b, i32, v[0], f_consti(b, 2, FInt32));
                   f_ret(b, v[1]);]]
    }}
}

-- Test array set
test.case {
    success = true,
    decls = 'i32 arr[] = {1, 2, 3};',
    after = 'test(arr[2] == 20);',
    functions = {{
        type = {'FVoid', 'FPointer'},
        args = {'arr'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_arr_set(b, i32, v[0], f_consti(b, 2, FInt32), 
                             f_consti(b, 20, FInt32));
                   f_ret_void(b);]]
    }}
}

-- Test array get
test.case {
    success = true,
    decls = 'i32 arr[] = {1, 2, 3};',
    functions = {{
        type = {'FInt32', 'FPointer'},
        args = {'arr'},
        ret = '3',
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_arr_get(b, i32, v[0], f_consti(b, 2, FInt32), FInt32);
                   f_ret(b, v[1]);]]
    }}
}

-- Test field offset
test.case {
    success = true,
    decls = 'Data data = _data;',
    functions = {{
        type = {'FPointer', 'FPointer'},
        args = {'&data'},
        ret = '&(data.d)',
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_field_offset(b, Data, v[0], d);
                   f_ret(b, v[1]);]]
    }}
}

-- Test field get
test.case {
    success = true,
    decls = 'Data data = _data;',
    functions = {{
        type = {'FPointer', 'FPointer'},
        args = {'&data'},
        ret = '&(data.d)',
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_field_offset(b, Data, v[0], d);
                   f_ret(b, v[1]);]]
    }}
}

-- Test field set

test.epilog()

