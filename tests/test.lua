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

-- Should be the first function called in a test generator
function test.preamble()
    print([[
#include <stdio.h>
#include <stdlib.h>
#include <stdplus/mem.h>
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
    print('\n    return 0;\n}')
end

-- Common setup for fahrenheit tests
-- Create the variables module, function, bblock, engine, fptr and b (builder)
function test.setup(ftype)
    print([[
  {
    FModule module;
    FEngine engine;
    i32 (*fptr)(void) = 0;
    int function;
    int bblock;
    FBuilder b;
    f_initmodule(&module);
    function = f_addfunction(&module, ]] .. ftype .. [[);
    bblock = f_addbblock(&module, function);
    b = f_builder(&module, function, bblock);
    f_init_engine(&engine);
    (void)b;
    (void)fptr;
    {
]])
end

-- End a test case (must be called after setup)
function test.teardown()
    print([[
    }
    f_closemodule(&module);
    if (engine.data != NULL) f_close_engine(&engine);
    test(usedmem == 0);
  }
]])
end

-- Call the verify function and expect success
function test.verify_sucess()
    print([[
      if(f_verifyfunction(&module, function, err)) {
        fprintf(stderr, "error: %s\n", err);
        exit(1);
      }
]])
end

-- Call the verify function and expect an error
function test.verify_fail()
    print([[
      if(!f_verifyfunction(&module, function, err)) {
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

