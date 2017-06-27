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

local test = {}

-- All fahrenheit basic types
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

-- Convert a fahrenheit type to a C type
function test.convert_type(type)
    if type == 'FBool' then
        return 'unsigned char'
    elseif type == 'FInt8' then
        return 'i8'
    elseif type == 'FInt16' then
        return 'i16'
    elseif type == 'FInt32' then
        return 'i32'
    elseif type == 'FInt64' then
        return 'i64'
    elseif type == 'FFloat' then
        return 'float'
    elseif type == 'FDouble' then
        return 'double'
    elseif type == 'FPointer' then
        return 'void *'
    elseif type == 'FVoid' then
        return 'void'
    end
end

-- Should be the first function called in a test generator
function test.preamble()
    print([[
#include <stdio.h>
#include <stdlib.h>
#include <stdplus/stdplus.h>
#include <fahrenheit/fahrenheit.h>

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
  mem_alloc = checkmem;
  (void)err;
]])
end

-- Should be the last function called in a test generator
function test.epilog()
    print('  return 0;\n}')
end

-- Common setup for fahrenheit tests
-- Declare the variables module, engine, functions[n], bblocks[n], 
-- v[n] (values) and b (builder)
function test.setup()
    print([[
  {
    FModule module;
    FEngine engine;
    int functions[100] = {0};
    int bblocks[100] = {0};
    FValue values[100] = {0};
    FBuilder b;
    f_initmodule(&module);
    f_init_engine(&engine);
    (void)functions;
    (void)bblocks;
    (void)values;
    (void)b;
]])
end

-- End a test case (must be called after setup)
function test.teardown()
    print([[
    f_closemodule(&module);
    f_close_engine(&engine);
    test(usedmem == 0);
  }
]])
end

-- Add a function with the given type
function test.add_function(n, ...)
    local args = {...}
    assert(#args >= 1)
    print(([[
    functions[%d] = f_addfunction(&module, f_ftype(&module, %s, %d, %s));
]]):format(n, args[1], #args - 1, table.concat(args, ', ')))
end

--[==[
function 
    function = f_addfunction(&module, ]] .. ftype .. [[);
    bblock = f_addbblock(&module, function);
    b = f_builder(&module, function, bblock);
--]==]

-- Call the verify function and expect success
function test.verify_sucess()
    print([[
    if(f_verifymodule(&module, err)) {
      fprintf(stderr, "error: %s\n", err);
      exit(1);
    }
]])
end

-- Call the verify function and expect an error
function test.verify_fail()
    print([[
    if(!f_verifymodule(&module, err)) {
      fprintf(stderr, "expected an error");
      exit(1);
    }
]])
end

return test

--[[

Compilation
    assert(f_compile(&e, &m) == 0);
    fptr = f_get_fpointer(e, 0, i32, (void));
    assert(fptr() == 123);
    f_close_engine(&e);

--]]

