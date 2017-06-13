/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2016 Gabriel de Quadros Ligneul
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef common_h
#define common_h

#include <assert.h>

/*
 * Track memory used
 */

static int usedmem = 0;

static void *checkmem(void *addr, size_t oldsize, size_t newsize) {
  usedmem = usedmem - oldsize + newsize;
  return mem_alloc_(addr, oldsize, newsize);
}

/*
 * Common parts in the tests
 */

#define TEST_SETUP(ftype) \
  { \
  int f, bb; \
  FModule m; \
  FBuilder b; \
  mem_alloc = checkmem; \
  f_initmodule(&m); \
  f = f_addfunction(&m, ftype); \
  bb = f_addbblock(&m, f); \
  b = f_builder(&m, f, bb); \
  (void)b; \
  { \

#define TEST_TEARDOWN \
  } \
  f_closemodule(&m); \
  assert(usedmem == 0); \
  } \

#endif

