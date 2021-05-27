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
/***********************************************************************
 ** md5.h -- header file for implementation of MD5                    **
 ** RSA Data Security, Inc. MD5 Message-Digest Algorithm              **
 ** Created: 2/17/90 RLR                                              **
 ** Revised: 12/27/90 SRD,AJ,BSK,JT Reference C version               **
 ** Revised (for MD5): RLR 4/27/91                                    **
 **********************************************************************/

/* $Tripwire: md5.h,v 1.1 2000/10/28 01:15:20 itripn Exp $ */

#ifndef __MD5_H
#define __MD5_H

#ifndef __TYPES_H
#include "types.h"
#endif

/* Data structure for MD5 (Message-Digest) computation */
typedef struct {
  uint32_t i[2];                   /* number of _bits_ handled mod 2^64 */
  uint32_t buf[4];                                    /* scratch buffer */
  uint8_t in[64];                              /* input buffer */
  uint8_t digest[16];     /* actual digest after MD5Final call */
} MD5_CTX;

void MD5Init(MD5_CTX*);
void MD5Update(MD5_CTX*, uint8_t*, unsigned int);
void MD5Final(MD5_CTX*);

#endif //__MD5_H

