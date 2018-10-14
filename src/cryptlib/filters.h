#ifndef FILTERS_H
#define FILTERS_H

#include "cryptlib.h"
#include "misc.h"
#include "smartptr.h"

class Filter : public BufferedTransformation
{
public:
    Filter(BufferedTransformation *outQ = NULL);
    Filter(const Filter &source);

    bool Attachable() {return true;}
    void Detach(BufferedTransformation *newOut = NULL);
    void Attach(BufferedTransformation *newOut);
    void Close()
        {InputFinished(); outQueue->Close();}

    unsigned long MaxRetrieveable()
        {return outQueue->MaxRetrieveable();}

    unsigned int Get(uint8_t &outByte)
        {return outQueue->Get(outByte);}
    unsigned int Get(uint8_t *outString, unsigned int getMax)
        {return outQueue->Get(outString, getMax);}

    unsigned int Peek(uint8_t &outByte) const
        {return outQueue->Peek(outByte);}

    BufferedTransformation *OutQueue() {return outQueue.get();}

protected:
    member_ptr<BufferedTransformation> outQueue;

private:
    void operator=(const Filter &); // assignment not allowed
};

class BlockFilterBase : public Filter
{
public:
    BlockFilterBase(BlockTransformation &cipher,
                    BufferedTransformation *outQueue);
    virtual ~BlockFilterBase() {}

    void Put(uint8_t inByte)
    {
        if (inBufSize == S)
            ProcessBuf();
        inBuf[inBufSize++]=inByte;
    }

    void Put(const uint8_t *inString, unsigned int length);

protected:
    void ProcessBuf();

    BlockTransformation &cipher;
    const unsigned int S;
    SecByteBlock inBuf;
    unsigned int inBufSize;
};

class BlockEncryptionFilter : public BlockFilterBase
{
public:
    BlockEncryptionFilter(BlockTransformation &cipher, BufferedTransformation *outQueue = NULL)
        : BlockFilterBase(cipher, outQueue) {}

protected:
    void InputFinished();
};

class BlockDecryptionFilter : public BlockFilterBase
{
public:
    BlockDecryptionFilter(BlockTransformation &cipher, BufferedTransformation *outQueue = NULL)
        : BlockFilterBase(cipher, outQueue) {}

protected:
    void InputFinished();
};

class StreamCipherFilter : public Filter
{
public:
    StreamCipherFilter(StreamCipher &c,
                       BufferedTransformation *outQueue = NULL)
        : Filter(outQueue), cipher(c) {}

    void Put(uint8_t inByte)
        {outQueue->Put(cipher.ProcessByte(inByte));}

    void Put(const uint8_t *inString, unsigned int length);

private:
    StreamCipher &cipher;
};

class HashFilter : public Filter
{
public:
    HashFilter(HashModule &hm, BufferedTransformation *outQueue = NULL)
        : Filter(outQueue), hash(hm) {}

    void InputFinished();

    void Put(uint8_t inByte)
        {hash.Update(&inByte, 1);}

    void Put(const uint8_t *inString, unsigned int length)
        {hash.Update(inString, length);}

private:
    HashModule &hash;
};

class Source : public Filter
{
public:
    Source(BufferedTransformation *outQ = NULL)
        : Filter(outQ) {}

    void Put(uint8_t)
        {Pump(1);}
    void Put(const uint8_t *, unsigned int length)
        {Pump(length);}
    void InputFinished()
        {PumpAll();}

    virtual unsigned int Pump(unsigned int size) =0;
    virtual unsigned long PumpAll() =0;
};

class Sink : public BufferedTransformation
{
public:
    unsigned long MaxRetrieveable()
        {return 0;}
    unsigned int Get(uint8_t &)
        {return 0;}
    unsigned int Get(uint8_t *, unsigned int)
        {return 0;}
    unsigned int Peek(uint8_t &) const
        {return 0;}
};

class BitBucket : public Sink
{
public:
    void Put(uint8_t) {}
    void Put(const uint8_t *, unsigned int) {}
};

BufferedTransformation *Insert(const uint8_t *in, unsigned int length, BufferedTransformation *outQueue);
unsigned int Extract(Source *source, uint8_t *out, unsigned int length);

#endif
