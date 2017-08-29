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
local function setup_test(decls)
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
local function teardown_test()
    print([[
    f_close_module(&module);
    f_close_engine(&engine);
    test(usedmem == 0);
    test_cases++;
    puts("----------------------------------------");
  }
]])
end

-- Add a function with the given type
local function add_function(f, ftype)
    local ret = ftype[1]
    local args = table.pack(table.unpack(ftype, 2))
    local args_str = table.concat(args, ', ')
    if args_str == '' then args_str = '0' end
    print(([[
    f[%d] = f_add_function(&module, f_ftype(&module, %s, %d, %s));
    bb[0] = f_add_bblock(&module, f[%d]);
    b = f_builder(&module, f[%d], bb[0]);]]):
        format(f, ret, #args, args_str, f, f))
end

-- Add an external function to the module
local function add_ext_function(f, ftype, ptr, variadic)
    local ret = ftype[1]
    local args = table.pack(table.unpack(ftype, 2))
    local args_str = table.concat(args, ', ')
    if args_str == '' then args_str = '0' end
    print(('    f[%d] = f_ftype(&module, %s, %d, %s);\n'):
        format(f, ret, #args, args_str))
    if variadic then
        print(('    f_set_vararg(&module, f[%d]);\n'):format(f))
    end
    print(('    f[%d] = f_add_extfunction(&module, f[%d], %s);\n'):
        format(f, f, ptr))
end

-- Call the verify function and expect success
local function verify_sucess()
    print([[
    if(f_verify_module(&module, err)) {
      fprintf(stderr, "unexpected! %s\n", err);
      exit(1);
    }
    else {
      printf("ok\n");
    }
]])
end

-- Call the verify function and expect an error
local function verify_fail()
    print([[
    if(!f_verify_module(&module, err)) {
      fprintf(stderr, "unexpected ok!\n");
      exit(1);
    }
    else {
      printf("%s\n", err);
    }
]])
end

-- Compile the module
local function compile()
    print([[
    test(f_compile(&engine, &module) == 0);
]])
end

-- Obtain the print format for the type
local function print_format(type)
    return ({
        FBool = '%u',
        FInt8 = '%u',
        FInt16 = '%u',
        FInt32 = '%u',
        FInt64 = '%lu',
        FFloat = '%g',
        FDouble = '%g',
        FPointer = '%p',
    })[type]
end

-- Run a function
local function run_function(f, ftype, args)
    local fret = test.convert_type(ftype[1])
    local fargs = map(test.convert_type, table.pack(table.unpack(ftype, 2)))
    local fargs_str = table.concat(fargs, ', ')
    if fargs_str == '' then fargs_str = 'void' end
    print('printf("running function @'.. f + 1 ..' with '.. args ..'\\n");\n')
    if ftype[1] == 'FPointer' then
        print(('printf("%%d\\n", ' ..
               'f_get_fpointer(&engine, %d, %s, (%s))(%s) == &module);\n')
            :format(f, fret, fargs_str, args))
    elseif ftype[1] ~= 'FVoid' then
        print(('printf("%s\\n", f_get_fpointer(&engine, %d, %s, (%s))(%s));\n')
            :format(print_format(ftype[1]), f, fret, fargs_str, args))
    else
        print(('f_get_fpointer(&engine, %d, %s, (%s))(%s);\n')
            :format(f, fret, fargs_str, args))
    end
end

-- Create a test case
--
-- Receive a table with the following structure:
-- t = {
--   decls = string?,           (declarations that should come before code)
--   success = bool,            (true if the verification should succeed)
--   after = string?,           (code that will run after the function)
--   functions = {              (list of the functions of the module)
--     args = {string...}?,     (list of arguments to the first function)
--     type = {ret, args...},   (function type)
--     vararg = bool?,          (true if the function is variadic)
--     code = string?,          (code that implements the function)
--     ext = string?,           (external function name)
--   },
-- }
function test.case(t)
    setup_test(t.decls)
    assert(t.functions)
    for i, f in ipairs(t.functions) do
        assert(f.type)
        if f.ext then
            add_ext_function(i - 1, f.type, f.ext, f.variadic)
        else
            add_function(i - 1, f.type)
            print(assert(f.code))
        end
    end
    print('    f_printer(&module, stdout);\n')
    if t.success then
        verify_sucess()
        for i, f in ipairs(t.functions) do
            if f.args then
                local args = table.concat(f.args, ', ')
                compile()
                run_function(i - 1, f.type, args)
            end
        end
        if t.after then print(t.after) end
    else
        verify_fail()
    end
    teardown_test()
end

return test

