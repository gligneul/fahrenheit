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

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>

#include <fahrenheit/ir.h>
#include <fahrenheit/verify.h>

typedef struct VerifyState {
  char *err;
  FModule *m;
  int f;
  int bb;
  int i;
  jmp_buf jmp;
} VerifyState;

/* Verify the condition and write the error message if it fails */
static void verify(VerifyState *vs, int cond, const char *format, ...) {
  if (!(cond)) {
    if (vs->err != NULL) {
      va_list args;
      va_start(args, format);
      vsprintf(vs->err, format, args);
      va_end(args);
    }
    longjmp(vs->jmp, 1);
  }
}

/* Verify an instruction */
static void verify_instr(VerifyState *vs) {
#if 0
  FKonst, FGetarg, FLoad, FStore, FOffset, FCast, FBinop,
  FCmp, FJmpIf, FJmp, FSelect, FRet, FCall, FPhi
#endif
  FInstr *i = f_instr(vs->m, vs->f, f_value(vs->bb, vs->i));
  switch (i->tag) {
    case FKonst:
      break;
    case FRet: {
      const char *err = "return type missmatch";
      FFunctionType *ftype = f_get_ftype_by_function(vs->m, vs->f);
      FValue retv = i->u.ret.val;
      if(ftype->ret == FVoid) {
        verify(vs, f_null(retv), err);
      }
      else {
        FInstr *reti;
        verify(vs, !f_null(retv), err);
        reti = f_instr(vs->m, vs->f, retv);
        verify(vs, ftype->ret == reti->type, err);
      }
      break;
    }
    default:
      verify(vs, 0, "instruction not verified");
      break;
  }
}

int f_verify_module(FModule *m, char *err) {
  vec_for(m->functions, i, {
    if (f_verify_function(m, i, err)) return 1;
  });
  return 0;
}

int f_verify_function(FModule *m, int function, char *err) {
  VerifyState vs;
  FFunction *f;
  vs.err = err;
  vs.m = m;
  vs.f = function;
  vs.bb = -1;
  vs.i = -1;
  if(setjmp(vs.jmp)) return 1;
  verify(&vs, function >= 0 && (size_t)function < vec_size(m->functions),
    "function not found");
  f = f_get_function(m, function);
  switch (f->tag) {
    case FExtFunc:
      break;
    case FModFunc:
      verify(&vs, vec_size(f->u.bblocks) > 0, "function without basic blocks");
      vec_for(f->u.bblocks, bb, {
        FBBlock *bblock = f_get_bblock(m, function, bb);
        vs.bb = bb;
        vec_for(*bblock, i, {
          vs.i = i;
          verify_instr(&vs);
        });
      });
      break;
  }
  return 0;
}

