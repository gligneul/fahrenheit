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

-- Test the cast instruction

local test = require 'test'

test.preamble()

-- Cast null value
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_cast(b, FUIntCast, FNullValue, FInt32);]]
    }}
}

-- Cast from void
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FPointer'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_store(b, v[0], v[0]);
            v[2] = f_cast(b, FUIntCast, v[1], FInt32);]]
    }}
}

-- Cast from pointer
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FPointer'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_cast(b, FUIntCast, v[0], FInt32);]]
    }}
}

-- Cast to void
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_cast(b, FUIntCast, v[0], FVoid);]]
    }}
}

-- Cast to pointer
test.case {
    success = false,
    functions = {{
        type = {'FVoid', 'FInt32'},
        code = [[
            v[0] = f_getarg(b, 0);
            v[1] = f_cast(b, FUIntCast, v[0], FPointer);]]
    }}
}

-- Test successful a cast
local function test_cast(cast_type, from_type, to_type, force_int_signed)
    local from_value = test.default_value(from_type)
    local from_ctype = test.convert_type(from_type)
    if force_int_signed then
        from_ctype = string.sub(from_ctype, 2)
    end
    local to_value = '(' .. from_ctype .. ')' .. from_value
    if test.is_float(from_type) and test.is_int(to_type) then
        to_value = '(ui64)' .. to_value
    end
    test.case {
        success = true,
        functions = {{
            args = {from_value},
            ret = to_value,
            type = {to_type, from_type},
            code = [[
                v[0] = f_getarg(b, 0);
                v[1] = f_cast(b, ]].. cast_type ..[[, v[0], ]].. to_type ..[[);
                       f_ret(b, v[1]);]]
        }}
    }
end

-- Integer casts
for i = 1, #test.int_types do
    for j = 1, #test.int_types do
        local from_type = test.int_types[i]
        local to_type = test.int_types[j]
        -- unsigned int casts
        test_cast('FUIntCast', from_type, to_type)
        -- signed int casts
        test_cast('FSIntCast', from_type, to_type, true)
    end
end

-- Float casts
for i = 1, #test.float_types do
    for j = 1, #test.float_types do
        local from_type = test.float_types[i]
        local to_type = test.float_types[j]
        test_cast('FFloatCast', from_type, to_type)
    end
end

-- Int/float casts
for i = 1, #test.int_types do
    for j = 1, #test.float_types do
        local int_type = test.int_types[i]
        local flt_type = test.float_types[j]
        test_cast('FFloatToUInt', flt_type, int_type)
        test_cast('FFloatToSInt', flt_type, int_type)
        test_cast('FUIntToFloat', int_type, flt_type)
        test_cast('FSIntToFloat', int_type, flt_type, true)
    end
end

test.epilog()

