#ifndef SHA_H
#define SHA_H

#include "iterhash.h"

class SHA : public IteratedHash<word32>
{
public:
    SHA();
    void Final(uint8_t *hash);
    unsigned int DigestSize() const {return DIGESTSIZE;};

    static void CorrectEndianess(word32 *out, const word32 *in, unsigned int byteCount)
    {
#ifdef WORDS_BIGENDIAN
        if (in!=out)
            memcpy(out, in, byteCount);
#else
        byteReverse(out, in, byteCount);
#endif
    }

    static void Transform(word32 *digest, const word32 *data );

    enum {DIGESTSIZE = 20, DATASIZE = 64};

private:
    void Init();
    void HashBlock(const word32 *input);
};

#endif
