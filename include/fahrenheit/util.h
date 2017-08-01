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

#ifndef fahrenheit_util_h
#define fahrenheit_util_h

/** \file util.h
 *
 * This module provides utility functions that ease the creation of a chain of
 * instruction.
 */

/** Obtain an offset of an array's element
 * Notice that the array and the offset parameters must be IR values. */
#define f_arr_offset(b, arr_type, arr, index) \
    f_offset(b, arr, \
        f_binop(b, FMul, index, \
            f_consti(b, sizeof(arr_type), index->type)), 0)

/** Obtain an element from an array */
#define f_arr_get(b, arr_type, arr, index, type) \
    f_load(b, f_arr_offset(b, arr_type, arr, index), type)

/** Store a value in an array */
#define f_arr_set(b, arr_type, arr, index, val) \
    f_store(b, f_arr_offset(b, arr_type, arr, index), val)

/** Obtain an offset of a struct's field */
#define f_field_offset(b, strukt, addr, field) \
    f_offset(b, addr, f_consti(b, offsetof(strukt, field, FInt32)), 0)

/** Obtain a struct's field value */
#define f_field_get(b, strukt, addr, field, type) \
    f_load(b, f_offsetof(b, strukt, addr, field), type)

/** Store a value in a struct's field */
#define f_field_set(b, strukt, addr, field, val) \
    f_store(b, f_offsetof(b, strukt, addr, field), val)

#endif

