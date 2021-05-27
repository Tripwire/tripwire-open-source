//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2019 Tripwire,
// Inc. Tripwire is a registered trademark of Tripwire, Inc.  All rights
// reserved.
// 
// This program is free software.  The contents of this file are subject
// to the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.  You may redistribute it and/or modify it
// only in compliance with the GNU General Public License.
// 
// This program is distributed in the hope that it will be useful.
// However, this program is distributed AS-IS WITHOUT ANY
// WARRANTY; INCLUDING THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS
// FOR A PARTICULAR PURPOSE.  Please see the GNU General Public License
// for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.
// 
// Nothing in the GNU General Public License or any other license to use
// the code or files shall permit you to use Tripwire's trademarks,
// service marks, or other intellectual property without Tripwire's
// prior written consent.
// 
// If you have any questions, please contact Tripwire, Inc. at either
// info@tripwire.org or www.tripwire.org.
//
/*
 *  haval.h:  specifies the interface to the HAVAL (V.1) hashing library.
 *
 *      HAVAL is a one-way hashing algorithm with the following
 *      collision-resistant property:
 *             It is computationally infeasible to find two or more
 *             messages that are hashed into the same fingerprint.
 *
 *  Reference:
 *       Y. Zheng, J. Pieprzyk and J. Seberry:
 *       ``HAVAL --- a one-way hashing algorithm with variable
 *       length of output'', Advances in Cryptology --- AUSCRYPT'92,
 *       Lecture Notes in Computer Science, Springer-Verlag, 1993.
 *
 *      This library provides routines to hash
 *        -  a string,
 *        -  a file,
 *        -  input from the standard input device,
 *        -  a 32-word block, and
 *        -  a string of specified length.
 *
 *  Author:     Yuliang Zheng
 *              Department of Computer Science
 *              University of Wollongong
 *              Wollongong, NSW 2522, Australia
 *              Email: yuliang@cs.uow.edu.au
 *              Voice: +61 42 21 4331 (office)
 *
 *  Date:       June 1993
 *
 *      Copyright (C) 1993 by C^3SR. All rights reserved. 
 *      This program may not be sold or used as inducement to
 *      buy a product without the written permission of C^3SR.
 *
 *  Descriptions:
 *
 *         Note:
 *            1. In general, HAVAL is faster on a little endian
 *               machine than on a big endian one.
 *
 *            2. The test program "havaltest.c" provides an option
 *               for testing the endianity of your machine.
 *
 *            3. The speed of HAVAL is even more remarkable on a
 *               machine that has a large number of internal registers.
 *
 *   PASS     define the number of passes        (3, 4, or 5)
 *   FPTLEN   define the length of a fingerprint (128, 160, 192, 224 or 256)
 */

#ifndef __HAVAL_H
#define __HAVAL_H

#ifndef PASS
#define PASS       3        /* 3, 4, or 5 */
#endif

#ifndef FPTLEN  
#define FPTLEN     128      /* 128, 160, 192, 224 or 256 */
#endif

#ifndef __TYPES_H
#include "types.h"
#endif

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

typedef uint32_t haval_word; /* a HAVAL word = 32 bits */

typedef struct {
  haval_word    count[2];                /* number of bits in a message */
  haval_word    fingerprint[8];          /* current state of fingerprint */    
  haval_word    block[32];               /* buffer for a 32-word block */ 
  uint8_t       remainder[32*4];         /* unhashed chars (No.<128) */
} haval_state;

/* Do not remove this line.  Protyping depends on it! 
#if defined(__STDC__) || defined(__cplusplus)
#define P_(s) s
#else
#define P_(s) ()
#endif
*/

#define P_(s) s
//Old prototyping stuff... I will ignore it for now.
#if 0 //unused in OST
void haval_string P_((char *, uint8_t *)); /* hash a string */
int  haval_file P_((char *, uint8_t *));   /* hash a file */
void haval_stdin P_((void));                     /* filter -- hash input from stdin */
#endif

void haval_start P_((haval_state *));            /* initialization */
void haval_hash P_((haval_state* state, uint8_t* str, int str_len));
void haval_end P_((haval_state *, uint8_t *)); /* finalization */
void haval_hash_block P_((haval_state *));       /* hash a 32-word block */

#endif //__HAVAL_H

