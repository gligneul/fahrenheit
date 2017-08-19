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

-- Test comparison and branching instructions

local test = require 'test'

test.preamble()

-- compare null left value
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_intcmp(b, FIntEq, FNullValue, v[0]);]]
    }}
}

-- compare null right value
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_intcmp(b, FIntEq, v[0], FNullValue);]]
    }}
}

-- compare values of different types
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32', 'FInt64'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_getarg(b, 1);
            v[2] = f_intcmp(b, FIntEq, v[0], v[1]);]]
    }}
}

-- float compare null left value
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_intcmp(b, FFpOEq, FNullValue, v[0]);]]
    }}
}

-- float compare null right value
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_intcmp(b, FFpOEq, v[0], FNullValue);]]
    }}
}

-- float compare values of different types
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32', 'FInt64'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_getarg(b, 1);
            v[2] = f_intcmp(b, FFpOEq, v[0], v[1]);]]
    }}
}

-- invalid pointer comparison
local invalid_ptr_cmp = {
    'FIntSLe', 'FIntSLt', 'FIntSGe', 'FIntSGt',
    'FIntULe', 'FIntULt', 'FIntUGe', 'FIntUGt'
}
for _, op in ipairs(invalid_ptr_cmp) do
    test.case {
        success = false,
        functions = {{
            type = {'FVoid', 'FPointer', 'FPointer'},
            code = [[
                v[0] = f_getarg(b, 0);
                v[1] = f_getarg(b, 1);
                v[2] = f_intcmp(b, ]].. op ..[[, v[0], v[1]);]]
        }}
    }
end

-- compare non int in intcmp
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FFloat'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_intcmp(b, FIntSLe, v[0], v[0]);]]
    }}
}

-- compare non float in fpcmp
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_fpcmp(b, FFpOLe, v[0], v[0]);]]
    }}
}

-- jmpif with null value
test.case {
    success = false,
    functions = {{
        type = {'FVoid'},
        code = [[
            bb[1] = f_add_bblock(&module, f[0]);

                   f_jmpif(b, FNullValue, bb[1], bb[1]);]]
    }}
}

-- jmpif with non boolean
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32'},
        code = [[
            bb[1] = f_add_bblock(&module, f[0]);

            v[0] = f_getarg(b, 0);
                   f_jmpif(b, v[0], bb[1], bb[1]);]]
    }}
}

-- jmp to invalid bb
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FBool'},
        code = [[
            f_jmp(b, bb[0]);]]
    }}
}

-- jmpif to invalid bb
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FBool'},
        code = [[
            bb[1] = f_add_bblock(&module, f[0]);

            v[0] = f_getarg(b, 0);
                   f_jmpif(b, v[0], bb[0], bb[1]);]]
    }}
}

-- jmp to valid branch
test.case {
    success = true,
    functions = {{
        type = {'FVoid'},
        args = {},
        code = [[
            bb[1] = f_add_bblock(&module, f[0]);
            bb[2] = f_add_bblock(&module, f[0]);

            f_jmp(b, bb[2]);

            b = f_builder(&module, f[0], bb[1]);
            f_ret_void(b);

            b = f_builder(&module, f[0], bb[2]);
            f_jmp(b, bb[1]);]]
    }}
}

-- jmpif to true branch
test.case {
    success = true,
    functions = {{
        type = {'FInt32', 'FBool'},
        args = {1},
        ret = 1,
        code = [[
            bb[1] = f_add_bblock(&module, f[0]);
            bb[2] = f_add_bblock(&module, f[0]);

            v[0] = f_getarg(b, 0);
            f_jmpif(b, v[0], bb[1], bb[2]);

            b = f_builder(&module, f[0], bb[1]);
            f_ret(b, f_consti(b, 1, FInt32));

            b = f_builder(&module, f[0], bb[2]);
            f_ret(b, f_consti(b, 0, FInt32));
        ]]
    }}
}

-- jmpif to false branch
test.case {
    success = true,
    functions = {{
        type = {'FInt32', 'FBool'},
        args = {0},
        ret = 0,
        code = [[
            bb[1] = f_add_bblock(&module, f[0]);
            bb[2] = f_add_bblock(&module, f[0]);

            v[0] = f_getarg(b, 0);
            f_jmpif(b, v[0], bb[1], bb[2]);

            b = f_builder(&module, f[0], bb[1]);
            f_ret(b, f_consti(b, 1, FInt32));

            b = f_builder(&module, f[0], bb[2]);
            f_ret(b, f_consti(b, 0, FInt32));
        ]]
    }}
}

-- create a test function for select instruction
local function test_select(cmd, args, ret)
    test.case {
        success = args ~= nil,
        functions = {{
            type = {'FInt32', 'FBool', 'FInt32', 'FInt32'},
            args = args,
            ret = ret,
            code = [[
                v[0] = f_getarg(b, 0);
                v[1] = f_getarg(b, 1);
                v[2] = f_getarg(b, 2);
                v[3] = ]].. cmd ..[[;
                       f_ret(b, v[3]);]]
        }}
    }
end

-- select with null cond
test_select('f_select(b, FNullValue, v[1], v[2])')

-- select with null left value
test_select('f_select(b, v[0], FNullValue, v[2])')

-- select with null right value
test_select('f_select(b, v[0], v[1], FNullValue)')

-- select with non boolean
test_select('f_select(b, v[1], v[1], v[2])')

-- select with different types
test_select('f_select(b, v[0], v[0], v[2])')

-- select true value
test_select('f_select(b, v[0], v[1], v[2])', {1, 123, 0}, 123)

-- select false value
test_select('f_select(b, v[0], v[1], v[2])', {0, 0, 123}, 123)

-- Create a test for comparisons
local function create_cmp_test(cmd, argtype, args)
    test.case {
        success = true,
        functions = {{
            type = {'FInt32', argtype, argtype},
            args = args,
            code = [[
                v[0] = f_getarg(b, 0);
                v[1] = f_getarg(b, 1);
                v[2] = ]].. cmd ..[[;
                v[3] = f_select(b, v[2], f_consti(b, 1, FInt32),
                                f_consti(b, 0, FInt32));
                       f_ret(b, v[3]);]]
        }}
    }
end

-- valid pointer comparisons
local pointer_cmp = {'FIntEq', 'FIntNe'}
for _, op in ipairs(pointer_cmp) do
    create_cmp_test(
        'f_intcmp(b, ' .. op .. ', v[0], v[1])',
        'FPointer', {'NULL', '&module'})
end

-- valid numeric comparisons
local int_cmp = {
    'FIntEq', 'FIntNe', 'FIntSLe', 'FIntSLt', 'FIntSGe', 'FIntSGt', 'FIntULe',
    'FIntULt', 'FIntUGe', 'FIntUGt'
}
for _, op in ipairs(int_cmp) do
    create_cmp_test(
        'f_intcmp(b, ' .. op .. ', v[0], v[1])',
        'FInt32', {0, 0})
end

-- valid 
local float_cmp = {
    'FFpOEq', 'FFpONe', 'FFpOLe', 'FFpOLt', 'FFpOGe', 'FFpOGt', 'FFpUEq',
    'FFpUNe', 'FFpULe', 'FFpULt', 'FFpUGe', 'FFpUGt'
}
for _, op in ipairs(float_cmp) do
    create_cmp_test(
        'f_fpcmp(b, ' .. op .. ', v[0], v[1])',
        'FFloat', {0, 0})
end

test.epilog()

