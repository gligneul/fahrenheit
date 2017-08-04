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
  int bb_ended;
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

/* Obtain the instruction */
static FInstr *get_instr(VerifyState *vs, FValue v) {
  /* TODO: verify if the value is reachable */
  verify(vs, !f_null(v), "null value");
  return f_instr(vs->m, vs->f, v);
}

/* Verify if the basic block is valid */
static void verify_bb(VerifyState *vs, int bb) {
  FFunction *f = f_get_function(vs->m, vs->f);
  int nbbs = vec_size(f->u.bblocks);
  verify(vs, bb >= 1 && bb < nbbs, "invalid basic block %d", bb);
}

/* Verify if the instruction is the last one */
static void verify_end(VerifyState *vs) {
  FBBlock *bb = f_get_bblock(vs->m,  vs->f, vs->bb);
  verify(vs, vs->i == (int)vec_size(*bb) - 1,
    "instruction after basic block end");
  vs->bb_ended = 1;
}

/* Verify an instruction */
static void verify_instr(VerifyState *vs) {
  FFunctionType *ftype = f_get_ftype_by_function(vs->m, vs->f);
  FInstr *i = f_instr(vs->m, vs->f, f_value(vs->bb, vs->i));
  switch (i->tag) {
    case FKonst:
      /* Don't need to verify constants */
      break;
    case FGetarg: {
      int n = i->u.getarg.n;
      verify(vs, n < ftype->nargs, "invalid argument");
      break;
    }
    case FLoad: {
      FInstr *addr = get_instr(vs, i->u.load.addr);
      verify(vs, addr->type == FPointer, "load from non pointer");
      break;
    }
    case FStore: {
      FInstr *addr = get_instr(vs, i->u.store.addr);
      FInstr *val = get_instr(vs, i->u.store.val);
      verify(vs, addr->type == FPointer, "store in non pointer");
      verify(vs, val->type != FVoid, "store void value");
      break;
    }
    case FOffset: {
      FInstr *addr = get_instr(vs, i->u.offset.addr);
      FInstr *offset = get_instr(vs, i->u.offset.offset);
      verify(vs, addr->type == FPointer, "store in non pointer");
      verify(vs, f_is_int(offset->type), "offset must be an integer");
      break;
    }
    case FCast: {
      const char *err = "invalid cast";
      FInstr *v = get_instr(vs, i->u.cast.val);
      switch (i->u.cast.op) {
        case FUIntCast:
        case FSIntCast:
          verify(vs, f_is_int(i->type) && f_is_int(v->type), err);
          break;
        case FFloatCast:
          verify(vs, f_is_float(i->type) && f_is_float(v->type), err);
          break;
        case FFloatToUInt:
        case FFloatToSInt:
          verify(vs, f_is_int(i->type) && f_is_float(v->type), err);
          break;
        case FUIntToFloat:
        case FSIntToFloat:
          verify(vs, f_is_float(i->type) && f_is_int(v->type), err);
          break;
      }
      break;
    }
    case FBinop: {
      enum FBinopTag op = i->u.binop.op; 
      enum FType lhs_type = get_instr(vs, i->u.binop.lhs)->type;
      enum FType rhs_type = get_instr(vs, i->u.binop.rhs)->type;
      verify(vs, lhs_type == rhs_type, "type mismatch in binop");
      if (op >= FAdd && op <= FDiv)
        verify(vs, f_is_num(lhs_type), "invalid binop type");
      else
        verify(vs, f_is_int(lhs_type), "invalid binop type");
      break;
    }
    case FIntCmp: {
      enum FIntCmpTag op = i->u.intcmp.op;
      enum FType lhs_type = get_instr(vs, i->u.intcmp.lhs)->type;
      enum FType rhs_type = get_instr(vs, i->u.intcmp.rhs)->type;
      verify(vs, lhs_type == rhs_type, "type mismatch");
      if (op == FIntEq || op == FIntNe)
        verify(vs, lhs_type == FPointer || f_is_int(lhs_type),
            "invalid integer comparison");
      else
        verify(vs, f_is_int(lhs_type), "invalid integer comparison");
      break;
    }
    case FFpCmp: {
      enum FType lhs_type = get_instr(vs, i->u.fpcmp.lhs)->type;
      enum FType rhs_type = get_instr(vs, i->u.fpcmp.rhs)->type;
      verify(vs, lhs_type == rhs_type, "type mismatch in cmp");
      verify(vs, f_is_float(lhs_type), "invalid float comparison");
      break;
    }
    case FJmpIf: {
      FInstr *cond = get_instr(vs, i->u.jmpif.cond);
      verify(vs, cond->type == FBool, "condition must be boolean");
      verify_bb(vs, i->u.jmpif.truebr);
      verify_bb(vs, i->u.jmpif.falsebr);
      verify_end(vs);
      break;
    }
    case FJmp: {
      verify_bb(vs, i->u.jmpif.truebr);
      verify_end(vs);
      break;
    }
    case FSelect: {
      FInstr *cond = get_instr(vs, i->u.jmpif.cond);
      enum FType lhs_type = get_instr(vs, i->u.select.truev)->type;
      enum FType rhs_type = get_instr(vs, i->u.select.falsev)->type;
      verify(vs, cond->type == FBool, "select condition must be boolean");
      verify(vs, lhs_type == rhs_type, "type mismatch in select");
      break;
    }
    case FRet: {
      const char *err = "return type missmatch";
      FValue retv = i->u.ret.val;
      if(ftype->ret == FVoid) {
        verify(vs, f_null(retv), err);
      }
      else {
        FInstr *reti = get_instr(vs, retv);
        verify(vs, ftype->ret == reti->type, err);
      }
      verify_end(vs);
      break;
    }
    case FCall:
      verify(vs, 0, "instruction not verified");
      break;
    case FPhi:
      verify(vs, 0, "instruction not verified");
      break;
  }
}

int f_verify_module(FModule *m, char *err) {
  if (vec_empty(m->functions)) {
    sprintf(err, "module with no functions");
    return 1;
  }
  else {
    vec_for(m->functions, i, {
      if (f_verify_function(m, i, err)) return 1;
    });
    return 0;
  }
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
        vs.bb_ended = 0;
        vec_for(*bblock, i, {
          vs.i = i;
          verify_instr(&vs);
        });
        verify(&vs, vs.bb_ended, "basic block not terminated");
      });
      break;
  }
  return 0;
}

