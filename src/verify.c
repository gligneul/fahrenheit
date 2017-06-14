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

/* Verify a condition, if it fails then write the message to 'err' and return */
#define VERIFY(cond, msg) \
  if(!(cond)) { \
    writeerr(err, msg); \
    return 1; \
  }

/* Write the error to the string and return 1 */
static void writeerr(char *err, const char *format, ...) {
  va_list args;
  if (err != NULL) {
    va_start(args, format);
    vsprintf(err, format, args);
    va_end(args);
  }
}

/* Verify an instruction */
static int verifyinstr(FModule *m, int function, FInstr *i, char *err) {
#if 0
  FKonst, FGetarg, FLoad, FStore, FOffset, FCast, FBinop,
  FCmp, FJmpIf, FJmp, FSelect, FRet, FCall, FPhi
#endif
  switch (i->tag) {
    case FKonst:
      break;
    case FRet: {
      FInstr *retv = f_instr(m, function, i->u.ret.val);
      FFunctionType *ftype = f_get_ftype_by_function(m, function);
      VERIFY(ftype->ret == retv->type, "return type missmatch");
      break;
    }
    default:
      writeerr(err, "instruction not verified");
      return 1;
  }
  return 0;
}

int f_verifymodule(FModule *m, char *err) {
  vec_for(m->functions, i, {
    if (f_verifyfunction(m, i, err)) return 1;
  });
  return 0;
}

int f_verifyfunction(FModule *m, int function, char *err) {
  FFunction *f;
  if (function < 0 || (size_t)function >= vec_size(m->functions))
    return writeerr(err, "function not found"), 1;
  f = f_get_function(m, function);
  switch (f->tag) {
    case FExtFunc:
      break;
    case FModFunc:
      vec_foreach(f->u.bblocks, bb, {
        vec_foreach(*bb, i, {
          if (verifyinstr(m, function, i, err)) return 1;
        });
      });
      break;
  }
  return 0;
}

