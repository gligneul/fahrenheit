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

#ifndef fahrenheit_instructions_h
#define fahrenheit_instructions_h

/** \file instructions.h
 *
 * This module provides the functions to create the intermediate representation
 * instructions. 
 * Before creating the instructions, you must create a builder (which is
 * available at the IR module).
 */

#include <fahrenheit/ir.h>

/** Create a constant boolean */
FValue f_constb(FBuilder b, int val);

/** Create a integer constant of given type
 * The type must be an integer. */
FValue f_consti(FBuilder b, ui64 val, enum FType type);

/** Create a float point constant of given type
 * The type must be a float point. */
FValue f_constf(FBuilder b, double val, enum FType type);

/** Create a constant pointer */
FValue f_constp(FBuilder b, void *val);

/** Create a null pointer */
#define f_nullp(b) f_constp(b, NULL)

/** Obtain the nth function argument */
FValue f_getarg(FBuilder b, int n);

/** Load a value of given type at the given address
 * The address must be a pointer. */
FValue f_load(FBuilder b, FValue addr, enum FType type);

/** Store the value at the address
 * The address must be a pointer. */
FValue f_store(FBuilder b, FValue addr, FValue val);

/** Add an offset in bytes to the address, resulting in another address
 * The address must be a pointer and the offset an integer.
 * The negative parameter indicates if the offset is negative or positive. */
FValue f_offset(FBuilder b, FValue addr, FValue offset, int negative);

/** Cast the value to the given type
 * The casts follow the C convetions. */
FValue f_cast(FBuilder b, enum FCastTag op, FValue val, enum FType type);

/** Perform a binary operation over two operands
 * The operands must have exactaly the same type. */
FValue f_binop(FBuilder b, enum FBinopTag op, FValue lhs, FValue rhs);

/** Compare two integer values and return a boolean
 * The operands must have exactaly the same type.
 * The Eq and Nq operations also can be used to compare pointers. */
FValue f_intcmp(FBuilder b, enum FIntCmpTag op, FValue lhs, FValue rhs);

/** Compare two float pointe values and return a boolean
 * The operands must have exactaly the same type. */
FValue f_fpcmp(FBuilder b, enum FFpCmpTag op, FValue lhs, FValue rhs);

/** Jump to the true branch if the condition holds, else jump to the false one
 * The condition must be an integer. */
FValue f_jmpif(FBuilder b, FValue cond, int truebr, int falsebr);

/** Jump to the given branch */
FValue f_jmp(FBuilder b, int dest);

/** Return the true value if the condition holds, else return the false one
 * The values must have exactaly the same type and the condition must have be a
 * bolean. */
FValue f_select(FBuilder b, FValue cond, FValue truev, FValue falsev);

/** Return the given value
 * The value must match the function's return type. */
FValue f_ret(FBuilder b, FValue val);

/** Return void */
#define f_ret_void(b) f_ret(b, FNullValue)

/** Call the given function with the given arguments
 * The arguments must match the function type. */
FValue f_call(FBuilder b, int function, ...);

/** Call the given function with the given arguments
 * The arguments must match the function type.
 * Don't take the ownership of the array of arguments. */
FValue f_callv(FBuilder b, int function, FValue *args);

/** Create a phi instruction of the given type
 * This instruction must be at the begining of the basic block. */
FValue f_phi(FBuilder b, enum FType type);

/* Add an incoming value to a phi
 * The passed basic block must be an ancestral block and the value must have
 * the same type of the phi. */
void f_add_incoming(FBuilder b, FValue phi, int bb, FValue value);

#endif

