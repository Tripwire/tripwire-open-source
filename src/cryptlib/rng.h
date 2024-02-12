#ifndef RNG_H
#define RNG_H

#include "cryptlib.h"
#include "filters.h"

// linear congruential generator
// originally by William S. England

// do not use for cryptographic purposes

class LC_RNG : public RandomNumberGenerator
{
public:
    LC_RNG(word32 init_seed)
        : seedBytes((ibyte *)&seed) {seed=init_seed;}

    ibyte GetByte();

    word32 GetSeed() {return seed;}

private:
    word32 seed;
    ibyte *const seedBytes;

    static const word32 m;
    static const word32 q;
    static const word16 a;
    static const word16 r;
};

// RNG derived from ANSI X9.17 Appendix C

class X917RNG : public RandomNumberGenerator
{
public:
    // cipher will be deleted by destructor
    X917RNG(BlockTransformation *cipher, const ibyte *seed);

    ibyte GetByte();

private:
    member_ptr<BlockTransformation> cipher;
    const int S;            // blocksize of cipher
    SecByteBlock dtbuf;     // buffer for enciphered timestamp
    SecByteBlock randseed, randbuf;
    int randbuf_counter;    // # of unused bytes left in randbuf
};

// This class implements Maurer's Universal Statistical Test for Random Bit Generators
// it is intended for measuring the randomness of *PHYSICAL* RNGs.
// For more details see his paper in Journal of Cryptology, 1992.

class MaurerRandomnessTest : public Sink
{
public:
    MaurerRandomnessTest();

    void Put(ibyte inByte);
    void Put(const ibyte *inString, unsigned int length);

    // BytesNeeded() returns how many more bytes of input is needed by the test
    // GetTestValue() should not be called before BytesNeeded()==0
    unsigned int BytesNeeded() const {return n >= (Q+K) ? 0 : Q+K-n;}

    // returns a number between 0.0 and 1.0, describing the quality of the
    // random numbers entered
    double GetTestValue() const;

private:
    enum {L=8, V=256, Q=2000, K=2000};
    double sum;
    unsigned int n;
    unsigned int tab[V];
};

#endif
