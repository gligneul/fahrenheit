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

/* This is the intermediate representation for fahrenheit */

#ifndef fahrenheit_ir_h
#define fahrenheit_ir_h

#include <stdplus/stdplus.h>

/* Declarations ***************************************************************/

/* Basic types */
enum FType {
  FBool, FInt8, FInt16, FInt32, FInt64,
  FFloat, FDouble, FPointer, FVoid
};

/* Instruction types */
enum FInstrTag {
  FKonst, FGetarg, FLoad, FStore, FOffset, FCast, FBinop,
  FCmp, FJmpIf, FJmp, FSelect, FRet, /*FCall,*/ FPhi
};

/* Binary operations */
enum FBinopTag {
  FAdd, FSub, FMul, FDiv, FRem, FShl, FShr, FAnd, FOr, FXor
};

/* Comparison operations */
enum FCmpTag {
  FEq, FNe, FLe, FLt, FGe, FGt
};

/* A value is a reference to an instruction inside a basic block */
typedef struct FValue {
  int bblock;
  int instr;
} FValue;

VEC_DECLARE(FValue);

/* Null value */
extern const FValue FNullValue;

/* Phi incoming values */
typedef struct FPhiInc {
  int bb;
  FValue value;
} FPhiInc;

VEC_DECLARE(FPhiInc);

/* SSA instructions */
typedef struct FInstr {
  enum FType type;
  enum FInstrTag tag;
  union {
    union { double f; ui64 i; void *p; } konst;
    struct { int n; } getarg;
    struct { FValue addr; } load;
    struct { FValue addr; FValue val; } store;
    struct { FValue addr; FValue offset; } offset;
    struct { FValue val; } cast;
    struct { enum FBinopTag op; FValue lhs; FValue rhs; } binop;
    struct { enum FCmpTag op; FValue lhs; FValue rhs; } cmp;
    struct { FValue cond; int truebr; int falsebr; } jmpif;
    struct { int dest; } jmp;
    struct { FValue cond; FValue truev; FValue falsev; } select;
    struct { FValue val; } ret;
/*    struct { FValue func; Vector(FValue) args; } call;*/
    struct { Vector(FPhiInc) inc; } phi;
  } u;
} FInstr;

/* A basic block is an array of instructions. A function is an array of
 * basic blocks. A module is an array of functions. */
VEC_DECLARE(FInstr);
typedef Vector(FInstr) FBBlock;

VEC_DECLARE(FBBlock);
typedef Vector(FBBlock) FFunction;

VEC_DECLARE(FFunction);

typedef struct FModule {
  Vector(FFunction) functions;
} FModule;

/* A builder is used to create new instructions */
typedef struct FBuilder {
  FModule *module;
  int function;
  int bblock;
} FBuilder;

/* Functions ******************************************************************/

/* init/close a module */
void f_initmodule(FModule *m);
void f_closemodule(FModule *m);

/* Add a function to the module */
int f_addfunction(FModule *m);

/* Add a basic block to the function */
int f_addbblock(FModule *m, int function);

/* Create a builder */
FBuilder f_builder(FModule *m, int function, int bblock);

/* Change the block of the builder */
void f_setbblock(FBuilder *b, int bblock);

/* Create a value */
FValue f_value(int bblock, int instr);

/* Compare two values */
int f_same(FValue a, FValue b);

/* Verify is a value is null */
int f_null(FValue v);

/* Obtain the instruction given the value */
FInstr* f_instr(FModule *m, int f, FValue v);

/*
 * Create the respective instruction
 */
FValue f_consti(FBuilder b, ui64 val, enum FType type);
FValue f_constf(FBuilder b, double val, enum FType type);
FValue f_constp(FBuilder b, void *val);
FValue f_getarg(FBuilder b, int n, enum FType type);
FValue f_load(FBuilder b, FValue addr, enum FType type);
FValue f_store(FBuilder b, FValue addr, FValue val);
FValue f_offset(FBuilder b, FValue addr, FValue offset);
FValue f_cast(FBuilder b, FValue val, enum FType type);
FValue f_binop(FBuilder b, enum FBinopTag op, FValue lhs, FValue rhs);
FValue f_cmp(FBuilder b, enum FCmpTag op, FValue lhs, FValue rhs);
FValue f_jmpif(FBuilder b, FValue cond, int truebr, int falsebr);
FValue f_jmp(FBuilder b, int dest);
FValue f_select(FBuilder b, FValue cond, FValue truev, FValue falsev);
FValue f_ret(FBuilder b, FValue val);
/*FValue f_call(FBuilder b, enum FType rettype, FValue func);*/
FValue f_phi(FBuilder b, enum FType type);

/* Add an incoming value to a phi */
void f_addincoming(FBuilder b, FValue phi, int bb, FValue value);

#endif

