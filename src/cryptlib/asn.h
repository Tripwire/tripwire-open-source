#ifndef ASN_H
#define ASN_H

#include "cryptlib.h"
#include "queue.h"

// NOTE: these tags and flags are NOT COMPLETE!
enum ASNTag {INTEGER=0x02, BIT_STRING=0x03, SEQUENCE=0x10};
enum ASNIdFlag {CONSTRUCTED = 0x20};

unsigned int DERLengthEncode(unsigned int length, ibyte *output);
unsigned int DERLengthEncode(unsigned int length, BufferedTransformation &);

#ifdef THROW_EXCEPTIONS
#define BERDecodeError() throw BERDecodeErr()
#else
#define BERDecodeError() assert (false)
#endif

class BERDecodeErr : public CryptlibException {public: BERDecodeErr() : CryptlibException("BER decode error") {}};

bool BERLengthDecode(BufferedTransformation &, unsigned int &);

class BERSequenceDecoder : public BufferedTransformation
{
public:
    BERSequenceDecoder(BufferedTransformation &inQueue);
    ~BERSequenceDecoder();

    void Put(ibyte) {}
    void Put(const ibyte *, unsigned int) {}

    unsigned long MaxRetrieveable()
        {return inQueue.MaxRetrieveable();}
    unsigned int Get(ibyte &outByte)
        {return inQueue.Get(outByte);}
    unsigned int Get(ibyte *outString, unsigned int getMax)
        {return inQueue.Get(outString, getMax);}
    unsigned int Peek(ibyte &outByte) const
        {return inQueue.Peek(outByte);}

private:
    BufferedTransformation &inQueue;
    bool definiteLength;
    unsigned int length;
};

class DERSequenceEncoder : public ByteQueue
{
public:
    DERSequenceEncoder(BufferedTransformation &outQueue);
    ~DERSequenceEncoder();
private:
    BufferedTransformation &outQueue;
};

#endif

