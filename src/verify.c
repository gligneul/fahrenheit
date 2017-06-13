/*
 * MIT License
 * 
 * Copyright (c) 2017 Gabriel de Quadros Ligneul
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdarg.h>

#include <fahrenheit/verify.h>
#include <fahrenheit/ir.h>

static int writeerr(char *err, const char *format, ...) {
  va_list args;
  if(err != NULL) {
    va_start(args, format);
    vsprintf(err, format, args);
    va_end(args);
  }
  return 1;
}

int f_verifymodule(struct FModule *m, char *err) {
  vec_for(m->functions, i, {
    if(f_verifyfunction(m, i, err)) break;
  });
  return 0;
}

int f_verifyfunction(struct FModule *m, int function, char *err) {
  FFunction *f;
  if(function < 0 || (size_t)function >= vec_size(m->functions))
    return writeerr(err, "function not found");
  f = vec_getref(m->functions, function);
  (void)f;
  return 0;
}

