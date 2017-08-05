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

-- Test for memory related instructions: load, store, offset

local test = require 'test'

test.preamble()

-- Load from a non ptr
test.case {
    success = false,
    functions = {{
        type = {'FInt32', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_load(b, v[0], FInt32);
            f_ret(b, v[1]);]]
    }}
}

-- Load from a ptr
for i = 1, #test.types - 1 do
    local t = test.types[i]
    local ctype = test.convert_type(t)
    local v = test.default_value(t)
    test.case {
        success = true,
        decls = ctype .. ' cell = ' .. v .. ';\n',
        functions = {{
            args = {'&cell'},
            ret = v,
            type = {t, 'FPointer'},
            code = [[
                v[0] = f_getarg(b, 0);
                v[1] = f_load(b, v[0], ]].. t ..[[);
                f_ret(b, v[1]);]]
        }}
    }
end

-- Store in non ptr
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_getarg(b, 1);
                   f_store(b, v[0], v[1]);
                   f_ret_void(b);]]
    }}
}

-- Store void value
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FPointer', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_getarg(b, 1);
            v[2] = f_store(b, v[0], v[1]);
                   f_store(b, v[0], v[2]);
                   f_ret_void(b);]]
    }}
}

-- Store a value
for i = 1, #test.types - 1 do
    local t = test.types[i]
    local ctype = test.convert_type(t)
    local v = test.default_value(t)
    test.case {
        success = true,
        decls = ctype .. ' cell = 0;\n',
        functions = {{
            args = {'&cell', v},
            after = 'test(cell == ('.. ctype ..')'.. v ..');',
            type = {'FVoid', 'FPointer', t},
            code = [[
                v[0] = f_getarg(b, 0);
                v[1] = f_getarg(b, 1);
                       f_store(b, v[0], v[1]);
                       f_ret_void(b);]]
        }}
    }
end

-- Offset null address
local t = 'FInt32'
local ctype = test.convert_type(t)
test.case {
    success = false,
    functions = {{
        type = {t, 'FPointer'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_consti(b, sizeof(]].. ctype ..[[), FInt32);
            v[2] = f_offset(b, FNullValue, v[1], 0);
            v[3] = f_load(b, v[2], ]].. t ..[[);
            f_ret(b, v[3]);]]
    }}
}

-- Offset non ptr address
local t = 'FInt32'
local ctype = test.convert_type(t)
test.case {
    success = false,
    functions = {{
        type = {t, 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_consti(b, sizeof(]].. ctype ..[[), FInt32);
            v[2] = f_offset(b, v[0], v[1], 0);
            v[3] = f_load(b, v[2], ]].. t ..[[);
            f_ret(b, v[3]);]]
    }}
}

-- Offset nil value
local t = 'FInt32'
local ctype = test.convert_type(t)
test.case {
    success = false,
    functions = {{
        type = {t, 'FPointer'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_consti(b, sizeof(]].. ctype ..[[), FInt32);
            v[2] = f_offset(b, v[0], FNullValue, 0);
            v[3] = f_load(b, v[2], ]].. t ..[[);
            f_ret(b, v[3]);]]
    }}
}

-- Offset non integer value
local t = 'FInt32'
local ctype = test.convert_type(t)
test.case {
    success = false,
    functions = {{
        type = {t, 'FPointer'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_consti(b, 0, FFloat);
            v[2] = f_offset(b, v[0], v[1], 0);
            v[3] = f_load(b, v[2], ]].. t ..[[);
            f_ret(b, v[3]);]]
    }}
}

-- Correct offset
for i = 1, #test.types - 1 do
    local t = test.types[i]
    local ctype = test.convert_type(t)
    local v = test.default_value(t)
    test.case {
        success = true,
        decls = ctype .. ' cell[2] = {0};\n',
        functions = {{
            args = {'cell'},
            ret = v,
            type = {t, 'FPointer'},
            code = [[
                cell[1] = ]].. v ..[[;
                v[0] = f_getarg(b, 0);
                v[1] = f_consti(b, sizeof(]].. ctype ..[[), FInt32);
                v[2] = f_offset(b, v[0], v[1], 0);
                v[3] = f_load(b, v[2], ]].. t ..[[);
                f_ret(b, v[3]);]]
        }}
    }
end

-- Negative offset
local t = 'FInt32'
local ctype = test.convert_type(t)
local v = test.default_value(t)
test.case {
    success = true,
    decls = ctype .. ' cell = ' .. v .. ';\n',
    functions = {{
        args = {'&cell'},
        ret = v,
        type = {t, 'FPointer'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_consti(b, sizeof(]].. ctype ..[[), FInt32);
            v[2] = f_offset(b, v[0], v[1], 0);
            v[3] = f_offset(b, v[2], v[1], 1);
            v[4] = f_load(b, v[3], ]].. t ..[[);
            f_ret(b, v[4]);]]
    }}
}

test.epilog()

