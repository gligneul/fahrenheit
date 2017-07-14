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
#include <fahrenheit/printer.h>

/* State used to print a single function */
typedef struct PrinterState {
  FILE *f;
  FModule *m;
  int function;
  int **valueid;
} PrinterState;

static void printer_init(PrinterState *ps, FILE *f, FModule *m,
    int function) {
  FFunction *func = f_get_function(m, function);
  int id = 1;
  ps->f = f;
  ps->m = m;
  ps->function = function;
  ps->valueid = mem_newarray(int *, vec_size(func->u.bblocks));
  vec_for(func->u.bblocks, i, {
    FBBlock *bb = vec_getref(func->u.bblocks, i);
    ps->valueid[i] = mem_newarray(int, vec_size(*bb));
    vec_for(*bb, j, {
      FInstr *instr = f_instr(m, function, f_value(i, j));
      if (instr->tag != FKonst && instr->type != FVoid)
        ps->valueid[i][j] = id++;
      else
        ps->valueid[i][j] = -1;
    });
  });
}

static void printer_finish(PrinterState *ps) {
  FFunction *func = f_get_function(ps->m, ps->function);
  vec_for(func->u.bblocks, i, {
    FBBlock *bb = vec_getref(func->u.bblocks, i);
    mem_deletearray(ps->valueid[i], vec_size(*bb));
  });
  mem_deletearray(ps->valueid, vec_size(func->u.bblocks));
}

static void print_type(PrinterState *ps, enum FType type) {
  const char *str;
  switch(type) {
    case FBool:    str = "bool"; break;
    case FInt8:    str = "i8"; break;
    case FInt16:   str = "i16"; break;
    case FInt32:   str = "i32"; break;
    case FInt64:   str = "i64"; break;
    case FFloat:   str = "flt"; break;
    case FDouble:  str = "dbl"; break;
    case FPointer: str = "ptr"; break;
    case FVoid:    str = "void"; break;
  }
  fprintf(ps->f, "%s", str);
}

static void print_ftype(PrinterState *ps, FFunctionType *ftype) {
  int i;
  int n = ftype->nargs;
  if (n == 0)
    print_type(ps, FVoid);
  else {
    print_type(ps, ftype->args[0]);
    for (i = 1; i < n; ++i) {
      fprintf(ps->f, ", ");
      print_type(ps, ftype->args[i]);
    }
  }
  fprintf(ps->f, " -> ");
  print_type(ps, ftype->ret);
}

static void print_fname(PrinterState *ps, int function) {
  fprintf(ps->f, "@%02d", function + 1);
}

static void print_bblock(PrinterState *ps, int bb) {
  fprintf(ps->f, "bb%d", bb + 1);
}

static void print_value_id(PrinterState *ps, FValue v) {
  fprintf(ps->f, "$%03d", ps->valueid[v.bblock][v.instr]);
}

static void print_konst(PrinterState *ps, FInstr *i) {
  fprintf(ps->f, "const ");
  print_type(ps, i->type);
  fprintf(ps->f, " ");
  switch (i->type) {
    case FBool:
        fprintf(ps->f, "%s", i->u.konst.i ? "true" : "false"); break;
    case FInt8:    fprintf(ps->f, "%lu", i->u.konst.i); break;
    case FInt16:   fprintf(ps->f, "%lu", i->u.konst.i); break;
    case FInt32:   fprintf(ps->f, "%lu", i->u.konst.i); break;
    case FInt64:   fprintf(ps->f, "%lu", i->u.konst.i); break;
    case FFloat:   fprintf(ps->f, "%f", i->u.konst.f); break;
    case FDouble:  fprintf(ps->f, "%f", i->u.konst.f); break;
    case FPointer: fprintf(ps->f, "%p", i->u.konst.p); break;
    default: break;
  }
}

static void print_value(PrinterState *ps, FValue v) {
  if (f_null(v))
    fprintf(ps->f, "null");
  else {
    FInstr *i = f_instr(ps->m, ps->function, v);
    fprintf(ps->f, "(");
    if (i->tag == FKonst)
      print_konst(ps, i);
    else {
      print_type(ps, i->type);
      fprintf(ps->f, " ");
      if (i->type == FVoid)
        fprintf(ps->f, "$xxx");
      else
        print_value_id(ps, v);
    }
    fprintf(ps->f, ")");
  }
}

static void print_instruction(PrinterState *ps, int bblock, int instr) {
  FValue v = f_value(bblock, instr);
  FInstr *i = f_instr(ps->m, ps->function, v);
  if (i->tag == FKonst) return;
  if (i->type == FVoid)
    fprintf(ps->f, "         ");
  else {
    fprintf(ps->f, "  ");
    print_value_id(ps, v);
    fprintf(ps->f, " = ");
  }
  switch (i->tag) {
    case FKonst:
      /* constants are printed as values */
      break;
    case FGetarg: {
      fprintf(ps->f, "getarg %d", i->u.getarg.n);
      break;
    }
    case FLoad: {
      fprintf(ps->f, "load ");
      print_type(ps, i->type);
      fprintf(ps->f, " from ");
      print_value(ps, i->u.load.addr);
      break;
    }
    case FStore: {
      fprintf(ps->f, "store ");
      print_value(ps, i->u.store.val);
      fprintf(ps->f, " at ");
      print_value(ps, i->u.store.addr);
      break;
    }
    case FOffset: {
      fprintf(ps->f, "offset ");
      print_value(ps, i->u.offset.addr);
      fprintf(ps->f, " %c ", i->u.offset.negative ? '-' : '+');
      print_value(ps, i->u.offset.offset);
      break;
    }
    case FCast: {
      break;
    }
    case FBinop: {
      break;
    }
    case FCmp: {
      break;
    }
    case FJmpIf: {
      break;
    }
    case FJmp: {
      break;
    }
    case FSelect: {
      break;
    }
    case FRet: {
      FValue val = i->u.ret.val;
      fprintf(ps->f, "ret ");
      if (f_null(val))
        fprintf(ps->f, "void");
      else
        print_value(ps, val);
      break;
    }
    case FCall: {
      break;
    }
    case FPhi: {
      break;
    }
  }
  fprintf(ps->f, "\n");
}

static void print_function(PrinterState *ps) {
  FFunction *func = f_get_function(ps->m, ps->function);
  if (func->tag == FExtFunc)
    fprintf(ps->f, "external ");
  fprintf(ps->f, "function ");
  print_fname(ps, ps->function);
  fprintf(ps->f, " : ");
  print_ftype(ps, f_get_ftype(ps->m, ps->function));
  fprintf(ps->f, "\n");
  if (func->tag == FModFunc) {
    vec_for(func->u.bblocks, i, {
      FBBlock *bb = vec_getref(func->u.bblocks, i);
      fprintf(ps->f, " ");
      print_bblock(ps, i);
      fprintf(ps->f, "\n");
      vec_for(*bb, j, print_instruction(ps, i, j));
    });
  }
  fprintf(ps->f, "\n");
}

void f_printer(struct FModule *m, FILE *f) {
  fprintf(f, "Fahrenheit module\n");
  vec_for(m->functions, function, {
    PrinterState ps;
    printer_init(&ps, f, m, function);
    print_function(&ps);
    printer_finish(&ps);
  });
  fprintf(f, ".\n");
}

