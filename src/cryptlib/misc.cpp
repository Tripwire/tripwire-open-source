// misc.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "misc.h"
#include "words.h"

void xorbuf(uint8_t *buf, const uint8_t *mask, unsigned int count)
{
    if (((ptr_size_type)buf | (ptr_size_type)mask | count) % WORD_SIZE == 0)
        XorWords((word *)buf, (const word *)mask, count/WORD_SIZE);
    else
    {
        for (unsigned int i=0; i<count; i++)
            buf[i] ^= mask[i];
    }
}

void xorbuf(uint8_t *output, const uint8_t *input, const uint8_t *mask, unsigned int count)
{
    if (((ptr_size_type)output | (ptr_size_type)input | (ptr_size_type)mask | count) % WORD_SIZE == 0)
        XorWords((word *)output, (const word *)input, (const word *)mask, count/WORD_SIZE);
    else
    {
        for (unsigned int i=0; i<count; i++)
            output[i] = input[i] ^ mask[i];
    }
}

unsigned int Parity(unsigned long value)
{
    for (unsigned int i=8*sizeof(value)/2; i>0; i/=2)
        value ^= value >> i;
    return (unsigned int)value&1;
}

unsigned int BytePrecision(unsigned long value)
{
    unsigned int i;
    for (i=sizeof(value); i; --i)
        if (value >> (i-1)*8)
            break;

    return i;
}

unsigned int BitPrecision(unsigned long value)
{
    if (!value)
        return 0;

    unsigned int l=0, h=8*sizeof(value);

    while (h-l > 1)
    {
        unsigned int t = (l+h)/2;
        if (value >> t)
            l = t;
        else
            h = t;
    }

    return h;
}

unsigned long Crop(unsigned long value, int size)
{
    if (size < 8*(int)sizeof(value))
        return (value & ((1L << size) - 1));
    else
        return value;
}

