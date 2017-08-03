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

-- compare non float in fpcmp

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

-- jmpif to invalid bb

-- jmp to invalid

-- select with null cond

-- select with null left value

-- select with null right value

-- select with non boolean

-- select with different types

-- valid pointer comparisons
local invalid_ptr_cmp = {'FEq', 'FNe', 'FLe', 'FLt', 'FGe', 'FGt',}

-- valid numeric comparisons
local invalid_ptr_cmp = {'FEq', 'FNe', 'FLe', 'FLt', 'FGe', 'FGt',}

-- jmpif to true branch

-- jmpif to false branch

-- jmp to valid branch

-- select true value

-- select false value

test.epilog()

