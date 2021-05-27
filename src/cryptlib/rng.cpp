// rng.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "rng.h"

#include <time.h>
#include <math.h>

// linear congruential generator
// originally by William S. England

// do not use for cryptographic purposes

/*
** Original_numbers are the original published m and q in the
** ACM article above.  John Burton has furnished numbers for
** a reportedly better generator.  The new numbers are now
** used in this program by default.
*/

#ifndef LCRNG_ORIGINAL_NUMBERS
const word32 LC_RNG::m=2147483647L;
const word32 LC_RNG::q=44488L;

const word16 LC_RNG::a=(unsigned int)48271L;
const word16 LC_RNG::r=3399;
#else
const word32 LC_RNG::m=2147483647L;
const word32 LC_RNG::q=127773L;

const word16 LC_RNG::a=16807;
const word16 LC_RNG::r=2836;
#endif

uint8_t LC_RNG::GetByte()
{
    word32 hi = seed/q;
    word32 lo = seed%q;

    long test = a*lo - r*hi;

    if (test > 0)
        seed = test;
    else
        seed = test+ m;

    return (seedBytes[0] ^ seedBytes[1] ^ seedBytes[2] ^ seedBytes[3]);
}

// ********************************************************

X917RNG::X917RNG(BlockTransformation *c, const uint8_t *seed)
    : cipher(c),
      S(cipher->BlockSize()),
      dtbuf(S),
      randseed(seed, S),
      randbuf(S),
      randbuf_counter(0)
{
    time_t tstamp1 = time(0);
    xorbuf(dtbuf, (uint8_t *)&tstamp1, STDMIN((int)sizeof(tstamp1), S));
    cipher->ProcessBlock(dtbuf);
    clock_t tstamp2 = clock();
    xorbuf(dtbuf, (uint8_t *)&tstamp2, STDMIN((int)sizeof(tstamp2), S));
    cipher->ProcessBlock(dtbuf);
}

uint8_t X917RNG::GetByte()
{
    if (randbuf_counter==0)
    {
        // calculate new enciphered timestamp
        clock_t tstamp = clock();
        xorbuf(dtbuf, (uint8_t *)&tstamp, STDMIN((int)sizeof(tstamp), S));
        cipher->ProcessBlock(dtbuf);

        // combine enciphered timestamp with seed
        xorbuf(randseed, dtbuf, S);

        // generate a new block of random bytes
        cipher->ProcessBlock(randseed, randbuf);

        // compute new seed vector
        for (unsigned int i=0; i<(unsigned int)S; i++)
            randseed[i] = randbuf[i] ^ dtbuf[i];
        cipher->ProcessBlock(randseed);

        randbuf_counter=S;
    }
    return(randbuf[(unsigned int)--randbuf_counter]);
}

MaurerRandomnessTest::MaurerRandomnessTest()
    : sum(0.0), n(0)
{
    for (unsigned i=0; i<V; i++)
        tab[i] = 0;
}

inline void MaurerRandomnessTest::Put(uint8_t inByte)
{
    if (n >= Q)
        sum += log(double(n - tab[inByte]));
    tab[inByte] = n;
    n++;
}

void MaurerRandomnessTest::Put(const uint8_t *inString, unsigned int length)
{
    while (length--)
        Put(*inString++);
}

double MaurerRandomnessTest::GetTestValue() const
{
    double fTu = (sum/(n-Q))/log(2.0);  // this is the test value defined by Maurer

    double value = fTu * 0.1392;        // arbitrarily normalize it to
    return value > 1.0 ? 1.0 : value;   // a number between 0 and 1
}
