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

-- Test the getarg instruction

local test = require 'test'

test.preamble()

-- return arg in void function
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            f_ret(b, v[0]);]]
    }}
}

-- get arg from function without args
test.case {
    success = false,
    functions = {{
        type = {'FVoid'},
        code = [[
            v[0] = f_getarg(b, 0);
            f_ret(b, v[0]);]]
    }}
}

-- return the arg (test for each type)
for i = 1, #test.types - 1 do
    local t = test.types[i]
    local v = test.default_value(t)
    test.case {
        success = true,
        args = {v},
        ret = v,
        functions = {{
            type = {t, t},
            code = [[
                v[0] = f_getarg(b, 0);
                f_ret(b, v[0]);]]
        }}
    }
end

-- return the second arg
test.case {
    success = true,
    args = {'0', '123'},
    ret = '123',
    functions = {{
        type = {'FInt32', 'FFloat', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 1);
            f_ret(b, v[0]);]]
    }}
}

-- return the 10th arg
local n = 10
local args_types = {}
local args = {}
for i = 1, n - 1 do
    table.insert(args_types, 'FFloat')
    table.insert(args, '0')
end
table.insert(args_types, 'FInt32')
table.insert(args, '123')
test.case {
    success = true,
    args = args,
    ret = '123',
    functions = {{
        type = {'FInt32', table.unpack(args_types)},
        code = [[
            v[0] = f_getarg(b, 9);
            f_ret(b, v[0]);]]
    }}
}

test.epilog()

