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

#ifndef fahrenheit_ir_h
#define fahrenheit_ir_h

/** \file ir.h
 *
 * This is the intermediate representation for fahrenheit.
 */

#include <stdplus/stdplus.h>

/* Declarations ***************************************************************/

/** Basic types */
enum FType {
  FBool, FInt8, FInt16, FInt32, FInt64,
  FFloat, FDouble, FPointer, FVoid
};

/** Instruction types */
enum FInstrTag {
  FKonst, FGetarg, FLoad, FStore, FOffset, FCast, FBinop,
  FCmp, FJmpIf, FJmp, FSelect, FRet, FCall, FPhi
};

/** Binary operations */
enum FBinopTag {
  FAdd, FSub, FMul, FDiv, FRem, FShl, FShr, FAnd, FOr, FXor
};

/** Comparison operations */
enum FCmpTag {
  FEq, FNe, FLe, FLt, FGe, FGt
};

/** A value is a reference to an instruction inside a basic block */
typedef struct FValue {
  int bblock;
  int instr;
} FValue;

VEC_DECLARE(FValue);

/** Null value */
extern const FValue FNullValue;

/** Phi incoming values */
typedef struct FPhiInc {
  int bb;
  FValue value;
} FPhiInc;

VEC_DECLARE(FPhiInc);

/** SSA instructions */
typedef struct FInstr {
  enum FType type;
  enum FInstrTag tag;
  union {
    union { double f; ui64 i; void *p; } konst;
    struct { int n; } getarg;
    struct { FValue addr; } load;
    struct { FValue addr; FValue val; } store;
    struct { FValue addr; FValue offset; int negative; } offset;
    struct { FValue val; } cast;
    struct { enum FBinopTag op; FValue lhs; FValue rhs; } binop;
    struct { enum FCmpTag op; FValue lhs; FValue rhs; } cmp;
    struct { FValue cond; int truebr; int falsebr; } jmpif;
    struct { int dest; } jmp;
    struct { FValue cond; FValue truev; FValue falsev; } select;
    struct { FValue val; } ret;
    struct { int function; FValue* args; int nargs; } call;
    struct { Vector(FPhiInc) inc; } phi;
  } u;
} FInstr;

VEC_DECLARE(FInstr);

/** A basic block is an array of instructions */
typedef Vector(FInstr) FBBlock;

VEC_DECLARE(FBBlock);

/** Function types */
typedef struct FFunctionType {
  enum FType ret;
  enum FType *args;
  int nargs;
} FFunctionType;

VEC_DECLARE(FFunctionType);

/** Function tags */
enum FFunctionTag {
  FExtFunc, FModFunc
};

/** Pointer to an external function */
typedef void (*FFunctionPtr)(void);

/** Function definition */
typedef struct FFunction {
  enum FFunctionTag tag;
  int type;
  union {
    FFunctionPtr ptr;           /* FExtFunc */
    Vector(FBBlock) bblocks;    /* FModFunc */
  } u;
} FFunction;

VEC_DECLARE(FFunction);

/** Module is the root structure */
typedef struct FModule {
  Vector(FFunction) functions;
  Vector(FFunctionType) ftypes;
} FModule;

/** A builder is used to create new instructions */
typedef struct FBuilder {
  FModule *module;
  int function;
  int bblock;
} FBuilder;

/* Functions ******************************************************************/

/** Initialize the module structure */
void f_init_module(FModule *m);

/** Free all module data */
void f_close_module(FModule *m);

/** Check if the type is an integer */
#define f_is_int()

/** Check if the type is float */
#define f_is_float()

/** Create a function type */
int f_ftype(FModule *m, enum FType ret, int nargs, ...);

/** Create a function type given an array
 * This function creates a copy of the args array. */
int f_ftypev(FModule *m, enum FType ret, int nargs, enum FType *args);

/** Obtain the function type given the index */
FFunctionType *f_get_ftype(FModule *m, int ftype);

/** Obtain the function type given the function index */
FFunctionType *f_get_ftype_by_function(FModule *m, int function);

/** Add a function to the module */
int f_add_function(FModule *m, int ftype);

/** Add an external function to the module */
int f_add_extfunction(FModule *m, int ftype, FFunctionPtr ptr);

/** Obtain a reference to a function given the index */
FFunction *f_get_function(FModule *m, int function);

/** Add a basic block to the function */
int f_add_bblock(FModule *m, int function);

/** Obtain a basic block given the function and the bblock indices
 * Calling this with an external function leads to undefined behavior. */
FBBlock *f_get_bblock(FModule *m, int function, int bblock);

/** Obtain a basic block given the builder */
FBBlock *f_get_bblock_by_builder(FBuilder b);

/** Create a builder */
FBuilder f_builder(FModule *m, int function, int bblock);

/** Change the block of the builder */
void f_set_bblock(FBuilder *b, int bblock);

/** Create a value */
FValue f_value(int bblock, int instr);

/** Compare two values */
int f_same(FValue a, FValue b);

/** Verify is a value is null */
int f_null(FValue v);

/** Obtain the instruction given the value */
FInstr* f_instr(FModule *m, int function, FValue v);

#endif

