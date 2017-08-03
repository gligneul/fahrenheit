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

#ifndef fahrenheit_verify_h
#define fahrenheit_verify_h

/** @file verify.h
 *
 * @defgroup verify
 * @brief Verify if the IR is well formed
 *
 * @{
 */

/** The required size for error messages */
#define FVerifyBufferSize 1024

struct FModule;

/** Verify if the IR module is well formed.
 * Return a value diferent from 0 if an error is found.
 * Return by reference the error message. The err parameter should be
 * pre-allocated with at least FVerifyBufferSize size. err can be NULL. */
int f_verify_module(struct FModule *m, char *err);

/** Verify if the IR function is well formed
 * Return a value diferent from 0 if an error is found.
 * Return by reference the error message. The err parameter should be
 * pre-allocated with at least FVerifyBufferSize size. err can be NULL. */
int f_verify_function(struct FModule *m, int function, char *err);

/**@}*/

#endif

