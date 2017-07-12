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

#include <assert.h>
#include <stdarg.h>

#include <fahrenheit/ir.h>

const FValue FNullValue = {-1, -1};

void f_init_module(FModule *m) {
  vec_init(m->functions);
  vec_init(m->ftypes);
}

void f_close_module(FModule *m) {
  vec_foreach(m->functions, func, {
    switch (func->tag) {
      case FExtFunc:
        break;
      case FModFunc:
        vec_foreach(func->u.bblocks, bb, {
          vec_foreach(*bb, i, {
            switch (i->tag) {
              case FPhi:
                vec_close(i->u.phi.inc);
                break;
              case FCall:
                mem_deletearray(i->u.call.args, i->u.call.nargs);
                break;
              default:
                break;
            }
          });
          vec_close(*bb);
        });
        vec_close(func->u.bblocks);
        break;
    }
  });
  vec_close(m->functions);
  vec_foreach(m->ftypes, ftype, {
    mem_deletearray(ftype->args, ftype->nargs);
  });
  vec_close(m->ftypes);
}

int f_ftype(FModule *m, enum FType ret, int nargs, ...) {
  int i;
  va_list args;
  FFunctionType type;
  va_start(args, nargs);
  type.ret = ret;
  type.args = mem_newarray(enum FType, nargs);
  type.nargs = nargs;
  for (i = 0; i < nargs; ++i)
    type.args[i] = va_arg(args, enum FType);
  va_end(args);
  vec_push(m->ftypes, type);
  return vec_size(m->ftypes) - 1;
}

int f_ftypev(FModule *m, enum FType ret, int nargs, enum FType *args) {
  int i;
  FFunctionType type;
  type.ret = ret;
  type.args = mem_newarray(enum FType, nargs);
  type.nargs = nargs;
  for (i = 0; i < nargs; ++i)
    type.args[i] = args[i];
  vec_push(m->ftypes, type);
  return vec_size(m->ftypes) - 1;
}

FFunctionType* f_get_ftype(FModule *m, int ftype) {
  return vec_getref(m->ftypes, ftype);
}

FFunctionType* f_get_ftype_by_function(FModule* m, int function) {
  return f_get_ftype(m, f_get_function(m, function)->type);
}

int f_add_function(FModule *m, int ftype) {
  FFunction f;
  f.tag = FModFunc;
  f.type = ftype;
  vec_init(f.u.bblocks);
  vec_push(m->functions, f);
  return vec_size(m->functions) - 1;
}

int f_add_extfunction(FModule *m, int ftype, FFunctionPtr ptr) {
  FFunction f;
  f.tag = FExtFunc;
  f.type = ftype;
  f.u.ptr = ptr;
  vec_push(m->functions, f);
  return vec_size(m->functions) - 1;
}

FFunction *f_get_function(FModule *m, int function) {
  return vec_getref(m->functions, function);
}

int f_add_bblock(FModule *m, int function) {
  FFunction *f = vec_getref(m->functions, function);
  FBBlock bb;
  assert(f->tag == FModFunc);
  vec_init(bb);
  vec_push(f->u.bblocks, bb);
  return vec_size(f->u.bblocks) - 1;
}

FBBlock *f_get_bblock(FModule *m, int function, int bblock) {
  FFunction *f = f_get_function(m, function);
  assert(f->tag == FModFunc);
  return vec_getref(f->u.bblocks, bblock);
}

FBBlock *f_get_bblock_by_builder(FBuilder b) {
  return f_get_bblock(b.module, b.function, b.bblock);
}

FBuilder f_builder(FModule *m, int function, int bblock) {
  FBuilder b;
  b.module = m;
  b.function = function;
  b.bblock = bblock;
  return b;
}

void f_set_bblock(FBuilder *b, int bblock) {
  b->bblock = bblock;
}

FValue f_value(int bblock, int instr) {
  FValue v;
  v.bblock = bblock;
  v.instr = instr;
  return v;
}

int f_same(FValue a, FValue b) {
  return a.bblock == b.bblock && a.instr == b.instr;
}

int f_null(FValue v) {
  return f_same(v, FNullValue);
}

FInstr* f_instr(FModule *m, int function, FValue v) {
  FBBlock *bb = f_get_bblock(m, function, v.bblock);
  return vec_getref(*bb, v.instr);
}

