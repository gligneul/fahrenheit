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

local test = require 'test'

test.preamble()

-- basic test, does nothing
test.setup()
test.verify_sucess()
test.teardown()

-- empty function
test.setup()
test.add_function(0, 'FVoid')
test.verify_fail()
test.teardown()

--print([[
--  FValue v = f_consti(b, 123, FInt64);
--  f_ret(b, v);
--]])

test.epilog()

--[[
#include "test.h"

int main(void) {
  TEST_SETUP;

  TEST_CASE_START(f_ftype(&m, FInt64, 0));
  FValue v = f_consti(b, 123, FInt64);
  f_ret(b, v);
  TEST_VERIFY_SUCCESS;
  {
    FEngine e;
    i32 (*fptr)(void);
    f_init_engine(&e);
    assert(f_compile(&e, &m) == 0);
    fptr = f_get_fpointer(e, 0, i32, (void));
    assert(fptr() == 123);
    f_close_engine(&e);
  }
  TEST_CASE_END;

  TEST_CASE_START(f_ftype(&m, FInt32, 0));
  FValue v = f_consti(b, 123, FInt64);
  f_ret(b, v);
  TEST_VERIFY_FAIL;
  TEST_CASE_END;

  TEST_TEARDOWN;
  return 0;
}
--]]

