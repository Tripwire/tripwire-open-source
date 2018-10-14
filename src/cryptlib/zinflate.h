#ifndef ZINFLATE_H
#define ZINFLATE_H

#include "forkjoin.h"
#include "misc.h"
#include "queue.h"

class Inflator : public Fork
{
public:
    class Err : public CryptlibException {public: Err(const char *message) : CryptlibException(message) {}};
    class UnexpectedEndErr : public Err {public: UnexpectedEndErr() : Err("Inflator: unexpected end of compressed block") {}};

    Inflator(BufferedTransformation *output = NULL,
             BufferedTransformation *bypassed = NULL);

    void Put(uint8_t b)
        {Inflator::Put(&b, 1);}

    void Put(const uint8_t *inString, unsigned int length);
    void InputFinished();

private:
    struct huft {
      uint8_t e;                /* number of extra bits or operation */
      uint8_t b;                /* number of bits in this code or subcode */
      union {
        word16 n;              /* literal, length base, or distance base */
        struct huft *t;     /* pointer to next level of table */
      } v;
    };

    int huft_build (unsigned *, unsigned, unsigned, const word16 *, const word16 *,
                       huft **, int *);
    int huft_free (huft *);
    int inflate_codes (huft *, huft *, int, int);
    int inflate_stored (void);
    int inflate_fixed (void);
    int inflate_dynamic (void);
    int inflate_block (bool &);
    void flush_output(unsigned int w);

    static const word16 border[19];
    static const word16 cplens[31];
    static const word16 cplext[31];
    static const word16 cpdist[31];
    static const word16 cpdext[31];

    static const word16 mask_bits[18];

    ByteQueue inQueue;
    uint8_t NEXTBYTE();

    SecByteBlock slide;
    unsigned int wp;
    word32 bb;                         /* bit buffer */
    unsigned bk;                    /* bits in bit buffer */

    bool afterEnd;
};

#endif

