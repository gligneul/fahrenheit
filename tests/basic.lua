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

-- Test basic stuff, constants and return

local test = require 'test'

test.preamble()

-- Module with no functions
test.case {
    success = false,
    functions = {}
}

-- Empty function
test.case {
    success = false,
    functions = {{
        type = {'FVoid'},
        code = ''
    }}
}

-- Instruction after return
test.case {
    success = false,
    functions = {{
        type = {'FVoid'},
        code = [[
            f_ret_void(b);
            f_ret_void(b);]]
    }}
}

-- Return value from void function
test.case {
    success = false,
    functions = {{
        type = {'FVoid'},
        code = [[
            v[0] = f_consti(b, 123, FInt64);
            f_ret(b, v[0]);]]
    }}
}

-- No return in non void function
test.case {
    success = false,
    functions = {{
        type = {'FInt64'},
        code = [[
            f_ret(b, FNullValue);]]
    }}
}

-- Return void
test.case {
    success = true,
    functions = {{
        args = {},
        type = {'FVoid'},
        code = [[
            f_ret_void(b);]]
    }}
}

-- Create a IR constant
local function create_const(v, ktype)
    if ktype == 'FBool' then
        return ('v[0] = f_constb(b, %s);'):format(v)
    elseif test.is_int(ktype) then
        return ('v[0] = f_consti(b, %s, %s);'):format(v, ktype)
    elseif test.is_float(ktype) then
        return ('v[0] = f_constf(b, %s, %s);'):format(v, ktype)
    else
        return ('v[0] = f_constp(b, %s);'):format(v)
    end
end

-- Return a constant
for i = 1, #test.types - 1 do
    for j = 1, #test.types - 1 do
        local ktype = test.types[j]
        local v = test.default_value(ktype)
        local call = create_const(v, ktype)
        test.case {
            success = (i == j),
            functions = {{
                args = {},
                ret = '(' .. test.convert_type(ktype) .. ')' .. v,
                type = {test.types[i]},
                code = call .. [[
                   f_ret(b, v[0]);]]
            }}
        }
    end
end

test.epilog()

