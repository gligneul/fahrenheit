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

-- Generate the basic test file

-- TODO tests
-- bblock without return
-- instruction after return
-- unreacheable bblock

local test = require 'test'

test.preamble()

-- basic test, does nothing
test.setup()
test.verify_sucess()
test.teardown()

-- empty function
test.setup()
test.add_function(0, test.make_ftype('FVoid'))
test.verify_fail()
test.teardown()

-- return non void in void function
test.setup()
test.add_function(0, test.make_ftype('FVoid'))
test.start_function(0, 0)
print([[
    v[0] = f_consti(b, 123, FInt64);
    f_ret(b, v[0]);
]])
test.verify_fail()
test.teardown()

-- return void in non void function
test.setup()
test.add_function(0, test.make_ftype('FInt64'))
test.start_function(0, 0)
print([[
    f_ret(b, FNullValue);
]])
test.verify_fail()
test.teardown()

-- return void
test.setup()
local ftype = test.make_ftype('FVoid')
test.add_function(0, ftype)
test.start_function(0, 0)
print([[
    f_ret(b, FNullValue);
]])
test.verify_sucess()
test.compile()
test.run_function(0, ftype, '')
test.teardown()

-- return a constant
for i = 1, #test.types - 1 do
    for j = 1, #test.types - 1 do
        test.setup()
        local ftype = test.make_ftype(test.types[i])
        test.add_function(0, ftype)
        test.start_function(0, 0)
        local ktype = test.types[j]
        local v = ''
        if test.is_int(ktype) then
            v = 1
            print(('    v[0] = f_consti(b, %s, %s);'):format(v, ktype))
        elseif test.is_float(ktype) then
            v = '12.34'
            print(('    v[0] = f_constf(b, %s, %s);'):format(v, ktype))
        else
            v = '&module'
            print(('    v[0] = f_constp(b, %s);'):format(v))
        end
        print('    f_ret(b, v[0]);')
        if i == j then
            local ret = '(' .. test.convert_type(ktype) .. ')' .. v
            test.verify_sucess()
            test.compile()
            test.run_function(0, ftype, '', ret)
        else
            test.verify_fail()
        end
        test.teardown()
    end
end

test.epilog()

