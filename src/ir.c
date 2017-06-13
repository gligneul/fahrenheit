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

#include <fahrenheit/ir.h>

const FValue FNullValue = {-1, -1};

/* Obtain the reference to given basic block */
static FBBlock *getbblock(FModule* m, int f, int bb) {
  FFunction *function = vec_getref(m->functions, f);
  return vec_getref(*function, bb);
}

/* Obtain the basic block given the builder */
static FBBlock *getbuilderbblock(FBuilder b) {
  return getbblock(b.module, b.function, b.bblock);
}

/* Add a instruction to the current block */
static FInstr *addinstr(FBuilder b, enum FType type, enum FInstrTag tag) {
  FBBlock *bb = getbuilderbblock(b);
  FInstr i;
  i.type = type;
  i.tag = tag;
  vec_push(*bb, i);
  return vec_getref(*bb, vec_size(*bb) - 1);
}

/* Obtain the last value add by the builder */
static FValue lastvalue(FBuilder b) {
  FBBlock *bb = getbuilderbblock(b);
  return f_value(b.bblock, vec_size(*bb) - 1);
}

void f_initmodule(FModule *m) {
  vec_init(m->functions);
}

void f_closemodule(FModule *m) {
  vec_foreach(m->functions, func, {
    vec_foreach(*func, bb, {
      vec_foreach(*bb, i, {
        if(i->tag == FPhi)
          vec_close(i->u.phi.inc);
      });
      vec_close(*bb);
    });
    vec_close(*func);
  });
  vec_close(m->functions);
}

int f_addfunction(FModule *m) {
  int n = vec_size(m->functions);
  FFunction f;
  vec_init(f);
  vec_push(m->functions, f);
  return n;
}

int f_addbblock(FModule *m, int function) {
  FFunction *f = vec_getref(m->functions, function);
  int n = vec_size(*f);
  FBBlock bb;
  vec_init(bb);
  vec_push(*f, bb);
  return n;
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

FInstr* f_instr(FModule *m, int f, FValue v) {
  FBBlock *bb = getbblock(m, f, v.bblock);
  return vec_getref(*bb, v.instr);
}

FValue f_consti(FBuilder b, ui64 val, enum FType type) {
  FInstr *i = addinstr(b, type, FKonst);
  i->u.konst.i = val;
  return lastvalue(b);
}

FValue f_constf(FBuilder b, f64 val, enum FType type) {
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

#if 0
FValue f_call(FBuilder b, enum FType rettype, FValue func) {
  FInstr *i = addinstr(b, rettype, );
  i->u.
  return lastvalue(b);
}
#endif

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

