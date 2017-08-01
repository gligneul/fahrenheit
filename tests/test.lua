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

-- functional
function map(f, x)
    local y = {}
    for i = 1, #x do
        y[i] = f(x[i])
    end
    return y
end

local test = {}

-- Fahrenheit types
test.types = {
    'FBool',
    'FInt8',
    'FInt16',
    'FInt32',
    'FInt64',
    'FFloat',
    'FDouble',
    'FPointer',
    'FVoid',
}

-- Integral types
test.int_types = {
    'FInt8',
    'FInt16',
    'FInt32',
    'FInt64',
}

-- Float types
test.float_types = {
    'FFloat',
    'FDouble',
}

-- Verify if a type is integral
function test.is_int(type)
    return type == 'FInt8' or type == 'FInt16' or
           type == 'FInt32' or type == 'FInt64'
end

-- Verify if a type is float point
function test.is_float(type)
    return type == 'FFloat' or type == 'FDouble'
end

-- Convert a fahrenheit type to a C type
function test.convert_type(type)
    if type == 'FBool' then
        return 'ui8'
    elseif type == 'FInt8' then
        return 'ui8'
    elseif type == 'FInt16' then
        return 'ui16'
    elseif type == 'FInt32' then
        return 'ui32'
    elseif type == 'FInt64' then
        return 'ui64'
    elseif type == 'FFloat' then
        return 'float'
    elseif type == 'FDouble' then
        return 'double'
    elseif type == 'FPointer' then
        return 'void *'
    elseif type == 'FVoid' then
        return 'void'
    else
        error('invalid type: ' .. tostring(type))
    end
end

-- Obtain a default value to test given the type
function test.default_value(type)
    if type == 'FBool' then
        return '1'
    elseif type == 'FInt8' then
        return '255'
    elseif type == 'FInt16' then
        return '1234'
    elseif type == 'FInt32' then
        return '12345'
    elseif type == 'FInt64' then
        return '123456'
    elseif type == 'FFloat' then
        return '123.45'
    elseif type == 'FDouble' then
        return '12345.56789'
    elseif type == 'FPointer' then
        return '&module'
    elseif type == 'FVoid' then
        return ''
    else
        error('invalid type: ' .. tostring(type))
    end
end

-- Initialize the C source file
-- Should be the first function called in a test generator
function test.preamble(decls)
    decls = decls or ''
    print([[
#include <stdio.h>
#include <stdlib.h>
#include <stdplus/stdplus.h>
#include <fahrenheit/fahrenheit.h>

]].. decls ..[[

static int usedmem = 0;
static char err[FVerifyBufferSize];

static void *checkmem(void *addr, size_t oldsize, size_t newsize) {
  usedmem = usedmem - oldsize + newsize;
  return mem_alloc_(addr, oldsize, newsize);
}

#define test(cond) \
  if (!(cond)) { \
    fprintf(stderr, "condition failed at line %d: %s\n", __LINE__, #cond); \
    exit(1); \
  }

int main(void) {
  int test_cases = 0;
  mem_alloc = checkmem;
  (void)err;
]])
end

-- Should be the last function called in a test generator
function test.epilog()
    print([[
  printf("Number of tests cases: %d\n", test_cases);
  return 0;
}]])
end

-- Common setup for fahrenheit tests
-- Declare the variables module, engine, f[n], bb[n], 
-- v[n] (values) and b (builder)
-- Receive a string with any other necessary declarations
function test.setup(decls)
    decls = decls or ''
    print([[
  {
    FModule module;
    FEngine engine;
    int f[100] = {0};
    int bb[100] = {0};
    FValue v[100] = {{0}};
    FBuilder b;
    ]].. decls ..[[
    f_init_module(&module);
    f_init_engine(&engine);
    (void)f;
    (void)bb;
    (void)v;
    (void)b;
]])
end

-- End a test case (must be called after setup)
function test.teardown()
    print([[
    f_printer(&module, stdout);
    f_close_module(&module);
    f_close_engine(&engine);
    test(usedmem == 0);
    test_cases++;
  }
]])
end

-- Create a function type
function test.make_ftype(ret, ...)
    return { ret = ret, args = {...}  }
end

-- Add a function with the given type
function test.add_function(f, ftype)
    local args_str = table.concat(ftype.args, ', ')
    if args_str == '' then args_str = '0' end
    print(([[
    f[%d] = f_add_function(&module, f_ftype(&module, %s, %d, %s));
]]):format(f, ftype.ret, #ftype.args, args_str))
end

function test.start_function(f, bb)
    print(([[
    bb[%d] = f_add_bblock(&module, f[%d]);
    b = f_builder(&module, f[%d], bb[%d]);
]]):format(bb, f, f, bb))
end

-- Call the verify function and expect success
function test.verify_sucess()
    print([[
    if(f_verify_module(&module, err)) {
      fprintf(stderr, "error: %s\n", err);
      exit(1);
    }
]])
end

-- Call the verify function and expect an error
function test.verify_fail()
    print([[
    if(!f_verify_module(&module, err)) {
      fprintf(stderr, "expected an error\n");
      exit(1);
    }
]])
end

-- Compile the module
function test.compile()
    print([[
    test(f_compile(&engine, &module) == 0);
]])
end

-- Run a function
function test.run_function(f, ftype, args, ret)
    local fret = test.convert_type(ftype.ret)
    local fargs = map(test.convert_type, ftype.args)
    local fargs_str = table.concat(fargs, ', ')
    if fargs_str == '' then fargs_str = 'void' end
    if ret then
        print(([[
    test(f_get_fpointer(engine, %d, %s, (%s))(%s) == (%s)(%s));
]]):format(f, fret, fargs_str, args, fret, ret))
    else
        print(([[
    f_get_fpointer(engine, %d, %s, (%s))(%s);
]]):format(f, fret, fargs_str, args))
    end
end

-- Print the module in stdout
function test.print()
    print([[
    f_printer(&module, stdout);
]])
end

-- Test case that should fail
function test.case_fail(ret, args, code)
    test.setup()
    local ftype = test.make_ftype(ret, table.unpack(args))
    test.add_function(0, ftype)
    test.start_function(0, 0)
    print(code)
    test.verify_fail()
    test.teardown()
end

-- Test case that should succeed
function test.case_success(ret_type, args_type, code, args, expect_ret)
    test.setup()
    local ftype = test.make_ftype(ret_type, table.unpack(args_type))
    test.add_function(0, ftype)
    test.start_function(0, 0)
    print(code)
    test.verify_sucess()
    test.compile()
    test.run_function(0, ftype, args, expect_ret)
    test.teardown()
end

return test

