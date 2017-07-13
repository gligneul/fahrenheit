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
test.setup()
test.add_function(0, test.make_ftype('FVoid', 'FInt32'))
test.start_function(0, 0)
print([[
    v[0] = f_getarg(b, 0);
    f_ret(b, v[0]);
]])
test.verify_fail()
test.teardown()

-- get arg from function without args
test.setup()
test.add_function(0, test.make_ftype('FVoid'))
test.start_function(0, 0)
print([[
    v[0] = f_getarg(b, 0);
    f_ret(b, v[0]);
]])
test.verify_fail()
test.teardown()

-- return the arg (test for each type)
for i = 1, #test.types - 1 do
    local t = test.types[i]
    local ftype = test.make_ftype(t, t)
    local v = test.is_int(t) and '123' or
        test.is_float(t) and '12.34' or '&module'
    test.setup()
    test.add_function(0, ftype)
    test.start_function(0, 0)
    print([[
        v[0] = f_getarg(b, 0);
        f_ret(b, v[0]);
    ]])
    test.verify_sucess()
    test.compile()
    test.run_function(0, ftype, v, v)
    test.teardown()
end

-- return the second arg
local ftype = test.make_ftype('FInt32', 'FFloat', 'FInt32')
test.setup()
test.add_function(0, ftype)
test.start_function(0, 0)
print([[
    v[0] = f_getarg(b, 1);
    f_ret(b, v[0]);
]])
test.verify_sucess()
test.compile()
test.run_function(0, ftype, '0, 123', '123')
test.teardown()

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
local args_str = table.concat(args, ',')
local ftype = test.make_ftype('FInt32', table.unpack(args_types))
test.setup()
test.add_function(0, ftype)
test.start_function(0, 0)
print([[
    v[0] = f_getarg(b, 9);
    f_ret(b, v[0]);
]])
test.verify_sucess()
test.compile()
test.run_function(0, ftype, args_str, '123')
test.teardown()

test.epilog()

