#ifndef ITERHASH_H
#define ITERHASH_H

#include "cryptlib.h"
#include "misc.h"

/* The following classes are explicitly instantiated in iterhash.cpp

    IteratedHash<word32>
    IteratedHash<word64>    // #ifdef WORD64_AVAILABLE
*/

template <class T> class IteratedHash : public virtual HashModule
{
public:
    IteratedHash(unsigned int blockSize, unsigned int digestSize);
    ~IteratedHash();
    void Update(const uint8_t *input, unsigned int length);

    typedef T HashWordType;

protected:
    void PadLastBlock(unsigned int lastBlockSize, uint8_t padFirst=0x80);
    virtual void Init() =0;
    virtual void HashBlock(const T *input) =0;

    unsigned int blockSize;
    word32 countLo, countHi;    // 64-bit bit count
    SecBlock<T> data;           // Data buffer
    SecBlock<T> digest;         // Message digest
};

template <class T> IteratedHash<T>::IteratedHash(unsigned int blockSize, unsigned int digestSize)
    : blockSize(blockSize), data(blockSize/sizeof(T)), digest(digestSize/sizeof(T))
{
}

template <class T> IteratedHash<T>::~IteratedHash()
{
}

template <class T> void IteratedHash<T>::Update(const uint8_t *input, unsigned int len)
{
    word32 tmp = countLo;
    if ((countLo = tmp + ((word32)len << 3)) < tmp)
        countHi++;             // Carry from low to high
    countHi += len >> 29;

    assert((blockSize & (blockSize-1)) == 0);   // blockSize is a power of 2
    unsigned int num = (unsigned int)(tmp >> 3) & (blockSize-1);

    if (num != 0)
    {
        if ((num+len) >= blockSize)
        {
            memcpy((uint8_t *)data.ptr+num, input, blockSize-num);
            HashBlock(data);
            input += (blockSize-num);
            len-=(blockSize - num);
            num=0;
            // drop through and do the rest
        }
        else
        {
            memcpy((uint8_t *)data.ptr+num, input, len);
            return;
        }
    }

    // we now can process the input data in blocks of blockSize
    // chars and save the leftovers to this->data.
    if (len >= blockSize)
    {
        if ((ptr_size_type)input % sizeof(T))   // test for alignment
            do
            {   // copy input first if it's not aligned correctly
                memcpy(data, input, blockSize);
                HashBlock(data);
                input+=blockSize;
                len-=blockSize;
            } while (len >= blockSize);
        else
            do
            {
                HashBlock((T *)input);
                input+=blockSize;
                len-=blockSize;
            } while (len >= blockSize);
    }

    memcpy(data, input, len);
}

template <class T> void IteratedHash<T>::PadLastBlock(unsigned int lastBlockSize, uint8_t padFirst)
{
    unsigned int num = (unsigned int)(countLo >> 3) & (blockSize-1);
    assert(num < blockSize);
    ((uint8_t *)data.ptr)[num++]=padFirst;
    if (num <= lastBlockSize)
        memset((uint8_t *)data.ptr+num, 0, lastBlockSize-num);
    else
    {
        memset((uint8_t *)data.ptr+num, 0, blockSize-num);
        HashBlock(data);
        memset(data, 0, lastBlockSize);
    }
}

// provide empty definitions to avoid instantiation warnings
//template <class T> void IteratedHash<T>::Init() {}
//template <class T> void IteratedHash<T>::HashBlock(const T *input) {}

#endif
