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
/* Useful defines/typedefs */

#ifndef __SHA_H
#define __SHA_H

#ifndef __TYPES_H
#include "types.h"
#endif

/* The SHS block size and message digest sizes, in bytes */

#define SHS_BLOCKSIZE   64
#define SHS_DIGESTSIZE  20

/* The structure for storing SHS info */

typedef struct {
           uint32_t digest[ 5 ];            /* Message digest */
           uint32_t countLo, countHi;       /* 64-bit bit count */
           uint32_t data[ 16 ];             /* SHS data buffer */
           } SHS_INFO;

/* Whether the machine is little-endian or not */

//int sig_sha_get();
void shsInit(SHS_INFO *shsInfo);
void shsUpdate(SHS_INFO* shsInfo, uint8_t* buffer, int count);
void shsFinal(SHS_INFO* shsInfo);

/* The next def turns on the change to the algorithm introduced by NIST at
 * the behest of the NSA.  It supposedly corrects a weakness in the original
 * formulation.  Bruce Schneier described it thus in a posting to the
 * Cypherpunks mailing list on June 21, 1994 (as told to us by Steve Bellovin):
 *
 *  This is the fix to the Secure Hash Standard, NIST FIPS PUB 180:
 *
 *       In Section 7 of FIPS 180 (page 9), the line which reads
 *
 *       "b) For t=16 to 79 let Wt = Wt-3 XOR Wt-8 XOR Wt-14 XOR
 *       Wt-16."
 *
 *       is to be replaced by
 *
 *       "b) For t=16 to 79 let Wt = S1(Wt-3 XOR Wt-8 XOR Wt-14 XOR
 *       Wt-16)."
 *
 *       where S1 is a left circular shift by one bit as defined in
 *       Section 3 of FIPS 180 (page 6):
 *
 *       S1(X) = (X<<1) OR (X>>31).
 *
 */

#define NEW_SHA

#endif //__SHA_H

