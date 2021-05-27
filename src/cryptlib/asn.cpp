// asn.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "asn.h"
#include "misc.h"
#include <stdlib.h>
#include <stdio.h>

unsigned int DERLengthEncode(unsigned int length, uint8_t *output)
{
    unsigned int i=0;
    if (length <= 0x7f)
    {
        output[i++] = uint8_t(length);
    }
    else
    {
        output[i++] = uint8_t(BytePrecision(length) | 0x80);
        for (int j=BytePrecision(length); j; --j)
        {
            output[i++] = uint8_t (length >> (j-1)*8);
        }
    }
    return i;
}

unsigned int DERLengthEncode(unsigned int length, BufferedTransformation &bt)
{
    uint8_t buf[10];   // should be more than enough
    unsigned int i = DERLengthEncode(length, buf);
    assert(i <= 10);
    bt.Put(buf, i);
    return i;
}

bool BERLengthDecode(BufferedTransformation &bt, unsigned int &length)
{
    uint8_t b;

    if (!bt.Get(b))
        BERDecodeError();

    if (!(b & 0x80))
        length = b;
    else
    {
        unsigned int lengthBytes = b & 0x7f;
        if (bt.MaxRetrieveable() < lengthBytes)
            BERDecodeError();

        bt.Get(b);
        while (!b && lengthBytes>1)
        {
            bt.Get(b);
            lengthBytes--;
        }

        switch (lengthBytes)
        {
        case 0:
            return false;   // indefinite length
        case 1:
            length = b;
            break;
        case 2:
            length = b << 8;
            length |= (bt.Get(b), b);
            break;
        default:
            BERDecodeError();
        }
    }
    return true;
}

BERSequenceDecoder::BERSequenceDecoder(BufferedTransformation &inQueue)
    : inQueue(inQueue)
{
    uint8_t b;
    if (!inQueue.Get(b) || b != (SEQUENCE | CONSTRUCTED))
        BERDecodeError();

    definiteLength = BERLengthDecode(inQueue, length);
}

BERSequenceDecoder::~BERSequenceDecoder()
{
    if (!definiteLength)
    {   // remove end-of-content octects
        word16 i;
        if (!inQueue.GetShort(i) || (i!=0))
        {
	    fputs("### Internal Error.\n### Invalid or corrupt key.\n### Exiting...\n\
", stderr);
	    exit(1); 
        }
    }
}

DERSequenceEncoder::DERSequenceEncoder(BufferedTransformation &outQueue)
    : outQueue(outQueue)
{
}

DERSequenceEncoder::~DERSequenceEncoder()
{
    unsigned int length = (unsigned int)CurrentSize();
    outQueue.Put(SEQUENCE | CONSTRUCTED);
    DERLengthEncode(length, outQueue);
    TransferTo(outQueue);
}
