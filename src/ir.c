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

/* Add a instruction to the current block */
static FInstr *addinstr(FBuilder b, enum FType type, enum FInstrTag tag) {
  FBBlock *bb = f_get_bblock_by_builder(b);
  FInstr i;
  i.type = type;
  i.tag = tag;
  vec_push(*bb, i);
  return vec_getref(*bb, vec_size(*bb) - 1);
}

/* Obtain the last value add by the builder */
static FValue lastvalue(FBuilder b) {
  FBBlock *bb = f_get_bblock_by_builder(b);
  return f_value(b.bblock, vec_size(*bb) - 1);
}

void f_initmodule(FModule *m) {
  vec_init(m->functions);
  vec_init(m->ftypes);
}

void f_closemodule(FModule *m) {
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

int f_addfunction(FModule *m, int ftype) {
  FFunction f;
  f.tag = FModFunc;
  f.type = ftype;
  vec_init(f.u.bblocks);
  vec_push(m->functions, f);
  return vec_size(m->functions) - 1;
}

int f_addextfunction(FModule *m, int ftype, FFunctionPtr ptr) {
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

int f_addbblock(FModule *m, int function) {
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

void f_setbblock(FBuilder *b, int bblock) {
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

FValue f_consti(FBuilder b, ui64 val, enum FType type) {
  FInstr *i = addinstr(b, type, FKonst);
  i->u.konst.i = val;
  return lastvalue(b);
}

FValue f_constf(FBuilder b, double val, enum FType type) {
  FInstr *i = addinstr(b, type, FKonst);
  i->u.konst.f = val;
  return lastvalue(b);
}

FValue f_constp(FBuilder b, void *val) {
  FInstr *i = addinstr(b, FPointer, FKonst);
  i->u.konst.p = val;
  return lastvalue(b);
}

FValue f_getarg(FBuilder b, int n, enum FType type) {
  FInstr *i = addinstr(b, type, FGetarg);
  i->u.getarg.n = n;
  return lastvalue(b);
}

FValue f_load(FBuilder b, FValue addr, enum FType type) {
  FInstr *i = addinstr(b, type, FLoad);
  i->u.load.addr = addr;
  return lastvalue(b);
}

FValue f_store(FBuilder b, FValue addr, FValue val) {
  FInstr *i = addinstr(b, FVoid, FStore);
  i->u.store.addr = addr;
  i->u.store.val = val;
  return lastvalue(b);
}

FValue f_offset(FBuilder b, FValue addr, FValue offset) {
  FInstr *i = addinstr(b, FPointer, FOffset);
  i->u.offset.addr = addr;
  i->u.offset.offset = offset;
  return lastvalue(b);
}

FValue f_cast(FBuilder b, FValue val, enum FType type) {
  FInstr *i = addinstr(b, type, FCast);
  i->u.cast.val = val;
  return lastvalue(b);
}

FValue f_binop(FBuilder b, enum FBinopTag op, FValue lhs, FValue rhs) {
  FInstr *lhsi = f_instr(b.module, b.function, lhs);
  FInstr *i = addinstr(b, lhsi->type, FBinop);
  i->u.binop.op = op;
  i->u.binop.lhs = lhs;
  i->u.binop.rhs = rhs;
  return lastvalue(b);
}

FValue f_cmp(FBuilder b, enum FCmpTag op, FValue lhs, FValue rhs) {
  FInstr *i = addinstr(b, FBool, FCmp);
  i->u.cmp.op = op;
  i->u.cmp.lhs = lhs;
  i->u.cmp.rhs = rhs;
  return lastvalue(b);
}

FValue f_jmpif(FBuilder b, FValue cond, int truebr, int falsebr) {
  FInstr *i = addinstr(b, FVoid, FJmpIf);
  i->u.jmpif.cond = cond;
  i->u.jmpif.truebr = truebr;
  i->u.jmpif.falsebr = falsebr;
  return lastvalue(b);
}

FValue f_jmp(FBuilder b, int dest) {
  FInstr *i = addinstr(b, FVoid, FJmp);
  i->u.jmp.dest = dest;
  return lastvalue(b);
}

FValue f_select(FBuilder b, FValue cond, FValue truev, FValue falsev) {
  FInstr *truevi = f_instr(b.module, b.function, truev);
  FInstr *i = addinstr(b, truevi->type, FSelect);
  i->u.select.cond = cond;
  i->u.select.truev = truev;
  i->u.select.falsev = falsev;
  return lastvalue(b);
}

FValue f_ret(FBuilder b, FValue val) {
  FInstr *i = addinstr(b, FVoid, FRet);
  i->u.ret.val = val;
  return lastvalue(b);
}

FValue f_call(FBuilder b, int function, ...) {
  int a;
  va_list args;
  FFunction *f = f_get_function(b.module, function);
  FFunctionType *ftype = f_get_ftype(b.module, f->type);
  FInstr *i = addinstr(b, ftype->ret, FCall);
  i->u.call.function = function;
  i->u.call.args = mem_newarray(FValue, ftype->nargs);
  i->u.call.nargs = ftype->nargs;
  va_start(args, function);
  for (a = 0; a < ftype->nargs; ++a)
    i->u.call.args[a] = va_arg(args, FValue);
  va_end(args);
  return lastvalue(b);
}

FValue f_callv(FBuilder b, int function, FValue *args) {
  int a;
  FFunction *f = f_get_function(b.module, function);
  FFunctionType *ftype = f_get_ftype(b.module, f->type);
  FInstr *i = addinstr(b, ftype->ret, FCall);
  i->u.call.function = function;
  i->u.call.args = mem_newarray(FValue, ftype->nargs);
  i->u.call.nargs = ftype->nargs;
  for (a = 0; a < ftype->nargs; ++a)
    i->u.call.args[a] = args[a];
  return lastvalue(b);
}

FValue f_phi(FBuilder b, enum FType type) {
  FInstr *i = addinstr(b, type, FPhi);
  vec_init(i->u.phi.inc);
  return lastvalue(b);
}

void f_addincoming(FBuilder b, FValue phi, int bb, FValue value) {
  FInstr *i = f_instr(b.module,  b.function, phi);
  FPhiInc inc;
  inc.bb = bb;
  inc.value = value;
  vec_push(i->u.phi.inc, inc);
}

