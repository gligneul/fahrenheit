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

-- Test the binary operations

local test = require 'test'

test.preamble()

-- left value null
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_binop(b, FAdd, FNullValue, v[0]);]]
    }}
}

-- right value null
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_binop(b, FAdd, v[0], FNullValue);]]
    }}
}

-- incompatible types
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32', FInt64},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_getarg(b, 1);
            v[2] = f_binop(b, FAdd, v[0], v[1]);]]
    }}
}

-- pointer arithmetic
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FPointer'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_binop(b, FAdd, v[0], v[0]);]]
    }}
}

-- pointer arithmetic 2
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FPointer', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_getarg(b, 1);
            v[2] = f_binop(b, FAdd, v[0], v[1]);]]
    }}
}

-- invalid operations for floats
local logical_ops = {'FRem', 'FShl', 'FShr', 'FAnd', 'FOr', 'FXor'}
for _, op in ipairs(logical_ops) do
    test.case {
        success = false,
        functions = {{
            type = {'FVoid', 'FFloat', 'FFloat'},
            code = [[
                v[0] = f_getarg(b, 0);
                v[1] = f_getarg(b, 1);
                v[2] = f_binop(b, ]].. op ..[[, v[0], v[1]);]]
        }}
    }
end

-- create a string that performs the operation in C code
local function perform_op(t, op, l, r)
    local wrap = function(v)
        local ctype = test.convert_type(t)
        return '(' .. ctype .. ')(' .. v .. ')'
    end
    local lhs = wrap(l)
    local rhs = wrap(r)
    if op == 'FAdd' then
        return lhs .. '+' .. rhs
    elseif op == 'FSub' then
        return lhs .. '-' .. rhs
    elseif op == 'FMul' then
        return lhs .. '*' .. rhs
    elseif op == 'FDiv' then
        return lhs .. '/' .. rhs
    elseif op == 'FRem' then
        return lhs .. '%' .. rhs
    elseif op == 'FShl' then
        return lhs .. '<<' .. rhs
    elseif op == 'FShr' then
        return lhs .. '>>' .. rhs
    elseif op == 'FAnd' then
        return lhs .. '&' .. rhs
    elseif op == 'FOr' then
        return lhs .. '|' .. rhs
    elseif op == 'FXor' then
        return lhs .. '^' .. rhs
    else
        error('invalid op: ' .. tostring(op))
    end
end

-- test a binary op
local function test_binary(t, op, r, l)
    test.case {
        success = true,
        functions = {{
            args =  {r, l},
            ret = perform_op(t, op, r, l),
            type = {t, t, t},
            code = [[
                v[0] = f_getarg(b, 0);
                v[1] = f_getarg(b, 1);
                v[2] = f_binop(b, ]].. op ..[[, v[0], v[1]);
                       f_ret(b, v[2]);]]
        }}
    }
end

-- valid operations for floats
local arith_ops = {'FAdd', 'FSub', 'FMul', 'FDiv'}
for _, op in ipairs(arith_ops) do
    for _, t in ipairs(test.float_types) do
        local v = test.default_value(t)
        test_binary(t, op, v, v)
    end
end

-- arithmetic operations for integers
for _, op in ipairs(arith_ops) do
    for _, t in ipairs(test.int_types) do
        local v = test.default_value(t)
        test_binary(t, op, v, v)
    end
end

-- logical operations for integers
for _, op in ipairs(logical_ops) do
    for _, t in ipairs(test.int_types) do
        local r = test.default_value(t)
        local l = '4'
        test_binary(t, op, r, l)
    end
end

test.epilog()

