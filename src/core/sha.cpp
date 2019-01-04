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
//#ifndef lint
/*static char rcsid[] = "$Tripwire: sha.cpp,v 1.1 2000/10/28 01:15:20 itripn Exp $";*/
//#endif

/*
 * sha.c
 *
 *  signature function hook for SHA.
 *
 * Gene Kim
 * Purdue University
 * August 10, 1993
 */

/* --------------------------------- SHS.H ------------------------------- */

/* NIST proposed Secure Hash Standard.

   Written 2 September 1992, Peter C. Gutmann.
   This implementation placed in the public domain.

   Comments to pgut1@cs.aukuni.ac.nz */


/* --------------------------------- SHS.C ------------------------------- */

/* NIST proposed Secure Hash Standard.

   Written 2 September 1992, Peter C. Gutmann.
   This implementation placed in the public domain.

   Comments to pgut1@cs.aukuni.ac.nz */
#include "stdcore.h"
#include <stdio.h>
#include <sys/types.h>

#if HAVE_MEMORY_H
# include <memory.h>
#endif

#include "sha.h"

/* The SHS f()-functions */

#define f1(x,y,z)   ( ( x & y ) | ( ~x & z ) )              /* Rounds  0-19 */
#define f2(x,y,z)   ( x ^ y ^ z )                           /* Rounds 20-39 */
#define f3(x,y,z)   ( ( x & y ) | ( x & z ) | ( y & z ) )   /* Rounds 40-59 */
#define f4(x,y,z)   ( x ^ y ^ z )                           /* Rounds 60-79 */

/* The SHS Mysterious Constants */

#define K1  0x5A827999L     /* Rounds  0-19 */
#define K2  0x6ED9EBA1L     /* Rounds 20-39 */
#define K3  0x8F1BBCDCL     /* Rounds 40-59 */
#define K4  0xCA62C1D6L     /* Rounds 60-79 */

/* SHS initial values */

#define h0init  0x67452301L
#define h1init  0xEFCDAB89L
#define h2init  0x98BADCFEL
#define h3init  0x10325476L
#define h4init  0xC3D2E1F0L

/* 32-bit rotate - kludged with shifts */

#define S(n,X)  ( ( X << n ) | ( X >> ( 32 - n ) ) )

/* The initial expanding function */

#ifdef NEW_SHA
#define expand(count)   temp = W[ count - 3 ] ^ W[ count - 8 ] ^ W[ count - 14 ] ^ W[ count - 16 ];W[ count ] = S(1, temp)
#else
#define expand(count)   W[ count ] = W[ count - 3 ] ^ W[ count - 8 ] ^ W[ count - 14 ] ^ W[ count - 16 ]
#endif

/* The four SHS sub-rounds */

#define subRound1(count)    \
    { \
    temp = S( 5, A ) + f1( B, C, D ) + E + W[ count ] + K1; \
    E = D; \
    D = C; \
    C = S( 30, B ); \
    B = A; \
    A = temp; \
    }

#define subRound2(count)    \
    { \
    temp = S( 5, A ) + f2( B, C, D ) + E + W[ count ] + K2; \
    E = D; \
    D = C; \
    C = S( 30, B ); \
    B = A; \
    A = temp; \
    }

#define subRound3(count)    \
    { \
    temp = S( 5, A ) + f3( B, C, D ) + E + W[ count ] + K3; \
    E = D; \
    D = C; \
    C = S( 30, B ); \
    B = A; \
    A = temp; \
    }

#define subRound4(count)    \
    { \
    temp = S( 5, A ) + f4( B, C, D ) + E + W[ count ] + K4; \
    E = D; \
    D = C; \
    C = S( 30, B ); \
    B = A; \
    A = temp; \
    }

/* The two buffers of 5 32-bit words */

uint32_t h0, h1, h2, h3, h4;
uint32_t A, B, C, D, E;

/* Initialize the SHS values */

void shsInit(SHS_INFO* shsInfo)
{
    /* Set the h-vars to their initial values */
    shsInfo->digest[ 0 ] = h0init;
    shsInfo->digest[ 1 ] = h1init;
    shsInfo->digest[ 2 ] = h2init;
    shsInfo->digest[ 3 ] = h3init;
    shsInfo->digest[ 4 ] = h4init;

    /* Initialise bit count */
    shsInfo->countLo = shsInfo->countHi = 0L;
    }

/* Perform the SHS transformation.  Note that this code, like MD5, seems to
   break some optimizing compilers - it may be necessary to split it into
   sections, eg based on the four subrounds */

void shsTransform(SHS_INFO *shsInfo)
{
    uint32_t W[ 80 ], temp;
    int i;

    /* Step A.  Copy the data buffer into the local work buffer */
    for( i = 0; i < 16; i++ )
    W[ i ] = shsInfo->data[ i ];

    /* Step B.  Expand the 16 words into 64 temporary data words */
    expand( 16 ); expand( 17 ); expand( 18 ); expand( 19 ); expand( 20 );
    expand( 21 ); expand( 22 ); expand( 23 ); expand( 24 ); expand( 25 );
    expand( 26 ); expand( 27 ); expand( 28 ); expand( 29 ); expand( 30 );
    expand( 31 ); expand( 32 ); expand( 33 ); expand( 34 ); expand( 35 );
    expand( 36 ); expand( 37 ); expand( 38 ); expand( 39 ); expand( 40 );
    expand( 41 ); expand( 42 ); expand( 43 ); expand( 44 ); expand( 45 );
    expand( 46 ); expand( 47 ); expand( 48 ); expand( 49 ); expand( 50 );
    expand( 51 ); expand( 52 ); expand( 53 ); expand( 54 ); expand( 55 );
    expand( 56 ); expand( 57 ); expand( 58 ); expand( 59 ); expand( 60 );
    expand( 61 ); expand( 62 ); expand( 63 ); expand( 64 ); expand( 65 );
    expand( 66 ); expand( 67 ); expand( 68 ); expand( 69 ); expand( 70 );
    expand( 71 ); expand( 72 ); expand( 73 ); expand( 74 ); expand( 75 );
    expand( 76 ); expand( 77 ); expand( 78 ); expand( 79 );

    /* Step C.  Set up first buffer */
    A = shsInfo->digest[ 0 ];
    B = shsInfo->digest[ 1 ];
    C = shsInfo->digest[ 2 ];
    D = shsInfo->digest[ 3 ];
    E = shsInfo->digest[ 4 ];

    /* Step D.  Serious mangling, divided into four sub-rounds */
    subRound1( 0 ); subRound1( 1 ); subRound1( 2 ); subRound1( 3 );
    subRound1( 4 ); subRound1( 5 ); subRound1( 6 ); subRound1( 7 );
    subRound1( 8 ); subRound1( 9 ); subRound1( 10 ); subRound1( 11 );
    subRound1( 12 ); subRound1( 13 ); subRound1( 14 ); subRound1( 15 );
    subRound1( 16 ); subRound1( 17 ); subRound1( 18 ); subRound1( 19 );
    subRound2( 20 ); subRound2( 21 ); subRound2( 22 ); subRound2( 23 );
    subRound2( 24 ); subRound2( 25 ); subRound2( 26 ); subRound2( 27 );
    subRound2( 28 ); subRound2( 29 ); subRound2( 30 ); subRound2( 31 );
    subRound2( 32 ); subRound2( 33 ); subRound2( 34 ); subRound2( 35 );
    subRound2( 36 ); subRound2( 37 ); subRound2( 38 ); subRound2( 39 );
    subRound3( 40 ); subRound3( 41 ); subRound3( 42 ); subRound3( 43 );
    subRound3( 44 ); subRound3( 45 ); subRound3( 46 ); subRound3( 47 );
    subRound3( 48 ); subRound3( 49 ); subRound3( 50 ); subRound3( 51 );
    subRound3( 52 ); subRound3( 53 ); subRound3( 54 ); subRound3( 55 );
    subRound3( 56 ); subRound3( 57 ); subRound3( 58 ); subRound3( 59 );
    subRound4( 60 ); subRound4( 61 ); subRound4( 62 ); subRound4( 63 );
    subRound4( 64 ); subRound4( 65 ); subRound4( 66 ); subRound4( 67 );
    subRound4( 68 ); subRound4( 69 ); subRound4( 70 ); subRound4( 71 );
    subRound4( 72 ); subRound4( 73 ); subRound4( 74 ); subRound4( 75 );
    subRound4( 76 ); subRound4( 77 ); subRound4( 78 ); subRound4( 79 );

    /* Step E.  Build message digest */
    shsInfo->digest[ 0 ] += A;
    shsInfo->digest[ 1 ] += B;
    shsInfo->digest[ 2 ] += C;
    shsInfo->digest[ 3 ] += D;
    shsInfo->digest[ 4 ] += E;
    }

#ifndef WORDS_BIGENDIAN

/* When run on a little-endian CPU we need to perform byte reversal on an
   array of longwords.  It is possible to make the code endianness-
   independant by fiddling around with data at the byte level, but this
   makes for very slow code, so we rely on the user to sort out endianness
   at compile time */

static void byteReverse(uint32_t* buffer, int byteCount)
    {
    uint32_t value;
    int count;

    byteCount /= sizeof( uint32_t );
    for( count = 0; count < byteCount; count++ )
    {
    value = ( buffer[ count ] << 16 ) | ( buffer[ count ] >> 16 );
    buffer[ count ] = ( ( value & 0xFF00FF00L ) >> 8 ) | ( ( value & 0x00FF00FFL ) << 8 );
    }
    }
#endif /* #ifndef WORDS_BIGENDIAN */

/* Update SHS for a block of data.  This code assumes that the buffer size
   is a multiple of SHS_BLOCKSIZE bytes long, which makes the code a lot
   more efficient since it does away with the need to handle partial blocks
   between calls to shsUpdate() */

void shsUpdate(SHS_INFO* shsInfo, uint8_t* buffer, int count)
    {
    /* Update bitcount */
    if( ( shsInfo->countLo + ( ( uint32_t ) count << 3 ) ) < shsInfo->countLo )
    shsInfo->countHi++; /* Carry from low to high bitCount */
    shsInfo->countLo += ( ( uint32_t ) count << 3 );
    shsInfo->countHi += ( ( uint32_t ) count >> 29 );

    /* Process data in SHS_BLOCKSIZE chunks */
    while( count >= SHS_BLOCKSIZE )
    {
    memcpy( (char *) shsInfo->data, (char *) buffer, SHS_BLOCKSIZE );
#ifndef WORDS_BIGENDIAN
    byteReverse( shsInfo->data, SHS_BLOCKSIZE );
#endif /* #ifndef WORDS_BIGENDIAN */
    shsTransform( shsInfo );
    buffer += SHS_BLOCKSIZE;
    count -= SHS_BLOCKSIZE;
    }

    /* Handle any remaining bytes of data.  This should only happen once
       on the final lot of data */
    memcpy( (char *) shsInfo->data, (char *) buffer, count );
    }

void shsFinal(SHS_INFO *shsInfo)
{
    int count;
    uint32_t lowBitcount = shsInfo->countLo, highBitcount = shsInfo->countHi;

    /* Compute number of bytes mod 64 */
    count = ( int ) ( ( shsInfo->countLo >> 3 ) & 0x3F );

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    ( ( uint8_t * ) shsInfo->data )[ count++ ] = 0x80;

    /* Pad out to 56 mod 64 */
    if( count > 56 )
    {
    /* Two lots of padding:  Pad the first block to 64 bytes */
    memset( ( char * ) shsInfo->data + count, 0, 64 - count );
#ifndef WORDS_BIGENDIAN
    byteReverse( shsInfo->data, SHS_BLOCKSIZE );
#endif /* #ifndef WORDS_BIGENDIAN */
    shsTransform( shsInfo );

    /* Now fill the next block with 56 bytes */
    memset( (char *) shsInfo->data, 0, 56 );
    }
    else
    /* Pad block to 56 bytes */
    memset( ( char * ) shsInfo->data + count, 0, 56 - count );
#ifndef WORDS_BIGENDIAN 
    byteReverse( shsInfo->data, SHS_BLOCKSIZE );
#endif /* #ifndef WORDS_BIGENDIAN */

    /* Append length in bits and transform */
    shsInfo->data[ 14 ] = highBitcount;
    shsInfo->data[ 15 ] = lowBitcount;

    shsTransform( shsInfo );
#ifndef WORDS_BIGENDIAN 
    byteReverse( shsInfo->data, SHS_DIGESTSIZE );
#endif /* #ifndef WORDS_BIGENDIAN */
    }

#ifdef TEST

/* ----------------------------- SHS Test code --------------------------- */

/* Size of buffer for SHS speed test data */

#define TEST_BLOCK_SIZE     ( SHS_DIGESTSIZE * 100 )

/* Number of bytes of test data to process */

#define TEST_BYTES          10000000L
#define TEST_BLOCKS         ( TEST_BYTES / TEST_BLOCK_SIZE )

void main()
    {
    SHS_INFO shsInfo;
    time_t endTime, startTime;
    uint8_t data[ TEST_BLOCK_SIZE ];
    long i;

    /* Test output data (this is the only test data given in the SHS
       document, but chances are if it works for this it'll work for
       anything) */
    shsInit( &shsInfo );
    shsUpdate( &shsInfo, ( uint8_t * ) "abc", 3 );
    shsFinal( &shsInfo );
    if( shsInfo.digest[ 0 ] != 0x0164B8A9L ||
    shsInfo.digest[ 1 ] != 0x14CD2A5EL ||
    shsInfo.digest[ 2 ] != 0x74C4F7FFL ||
    shsInfo.digest[ 3 ] != 0x082C4D97L ||
    shsInfo.digest[ 4 ] != 0xF1EDF880L )
    {
    puts( "Error in SHS implementation" );
    exit( -1 );
    }

    /* Now perform time trial, generating MD for 10MB of data.  First,
       initialize the test data */
    memset( data, 0, TEST_BLOCK_SIZE );

    /* Get start time */
    printf( "SHS time trial.  Processing %ld characters...\n", TEST_BYTES );
    time( &startTime );

    /* Calculate SHS message digest in TEST_BLOCK_SIZE byte blocks */
    shsInit( &shsInfo );
    for( i = TEST_BLOCKS; i > 0; i-- )
    shsUpdate( &shsInfo, data, TEST_BLOCK_SIZE );
    shsFinal( &shsInfo );

    /* Get finish time and time difference */
    time( &endTime );
    printf( "Seconds to process test input: %ld\n", endTime - startTime );
    printf( "Characters processed per second: %ld\n", TEST_BYTES / ( endTime - startTime ) );
    }

#endif

