// filters.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "filters.h"
#include "queue.h"
#include <memory>

#if __cplusplus >= 201103L
# define TW_UNIQUE_PTR std::unique_ptr
#else
# define TW_UNIQUE_PTR std::auto_ptr
#endif

Filter::Filter(BufferedTransformation *outQ)
    : outQueue(outQ ? outQ : new ByteQueue) 
{
}

Filter::Filter(const Filter &source)
    : outQueue(new ByteQueue)
{
}

void Filter::Detach(BufferedTransformation *newOut)
{
    TW_UNIQUE_PTR<BufferedTransformation> out(newOut ? newOut : new ByteQueue);
    outQueue->Close();
    outQueue->TransferTo(*out);
    outQueue.reset(out.release());
}

void Filter::Attach(BufferedTransformation *newOut)
{
    if (outQueue->Attachable())
        outQueue->Attach(newOut);
    else
        Detach(newOut);
}

BlockFilterBase::BlockFilterBase(BlockTransformation &c,
                                 BufferedTransformation *outQ)
    : Filter(outQ), cipher(c), S(cipher.BlockSize()), inBuf(S)
{
    inBufSize=0;
}

void BlockFilterBase::ProcessBuf()
{
    cipher.ProcessBlock(inBuf);
    outQueue->Put(inBuf, S);
    inBufSize=0;
}

void BlockFilterBase::Put(const uint8_t *inString, unsigned int length)
{
    while (length--)
        BlockFilterBase::Put(*inString++);
}

void BlockEncryptionFilter::InputFinished()
{
    if (inBufSize == S)
        ProcessBuf();
    // pad last block
    memset(inBuf+inBufSize, S-inBufSize, S-inBufSize);
    ProcessBuf();
}

void BlockDecryptionFilter::InputFinished()
{
    cipher.ProcessBlock(inBuf);
    if (inBuf[S-1] > S)
        inBuf[S-1] = 0;     // something's wrong with the padding
    outQueue->Put(inBuf, S-inBuf[S-1]);
    inBufSize=0;
}

void StreamCipherFilter::Put(const uint8_t *inString, unsigned int length)
{
    SecByteBlock temp(length);
    cipher.ProcessString(temp, inString, length);
    outQueue->Put(temp, length);
}

void HashFilter::InputFinished()
{
    SecByteBlock buf(hash.DigestSize());
    hash.Final(buf);
    outQueue->Put(buf, hash.DigestSize());
}

BufferedTransformation *Insert(const uint8_t *in, unsigned int length, BufferedTransformation *outQueue)
{
    outQueue->Put(in, length);
    return outQueue;
}

unsigned int Extract(Source *source, uint8_t *out, unsigned int length)
{
    while (source->MaxRetrieveable() < length && source->Pump(1));
    return source->Get(out, length);
}

