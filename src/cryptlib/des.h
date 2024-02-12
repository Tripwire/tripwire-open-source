#ifndef DES_H
#define DES_H

#include "cryptlib.h"
#include "misc.h"

class DES : public BlockTransformation
{
public:
    DES(const ibyte *userKey, CipherDir);

    void ProcessBlock(const ibyte *inBlock, ibyte * outBlock);
    void ProcessBlock(ibyte * inoutBlock)
        {DES::ProcessBlock(inoutBlock, inoutBlock);}

    enum {KEYLENGTH=8, BLOCKSIZE=8};
    unsigned int BlockSize() const {return BLOCKSIZE;}

protected:
    static const word32 Spbox[8][64];

    SecBlock<word32> k;
};

class DESEncryption : public DES
{
public:
    DESEncryption(const ibyte * userKey)
        : DES (userKey, ENCRYPTION) {}
};

class DESDecryption : public DES
{
public:
    DESDecryption(const ibyte * userKey)
        : DES (userKey, DECRYPTION) {}
};

class DES_EDE_Encryption : public BlockTransformation
{
public:
    DES_EDE_Encryption(const ibyte * userKey)
        : e(userKey, ENCRYPTION), d(userKey + DES::KEYLENGTH, DECRYPTION) {}

    void ProcessBlock(const ibyte *inBlock, ibyte * outBlock);
    void ProcessBlock(ibyte * inoutBlock);

    enum {KEYLENGTH=16, BLOCKSIZE=8};
    unsigned int BlockSize() const {return BLOCKSIZE;}

private:
    DES e, d;
};

class DES_EDE_Decryption : public BlockTransformation
{
public:
    DES_EDE_Decryption(const ibyte * userKey)
        : d(userKey, DECRYPTION), e(userKey + DES::KEYLENGTH, ENCRYPTION) {}

    void ProcessBlock(const ibyte *inBlock, ibyte * outBlock);
    void ProcessBlock(ibyte * inoutBlock);

    enum {KEYLENGTH=16, BLOCKSIZE=8};
    unsigned int BlockSize() const {return BLOCKSIZE;}

private:
    DES d, e;
};

class TripleDES_Encryption : public BlockTransformation
{
public:
    TripleDES_Encryption(const ibyte * userKey)
        : e1(userKey, ENCRYPTION), d(userKey + DES::KEYLENGTH, DECRYPTION),
          e2(userKey + 2*DES::KEYLENGTH, ENCRYPTION) {}

    void ProcessBlock(const ibyte *inBlock, ibyte * outBlock);
    void ProcessBlock(ibyte * inoutBlock);

    enum {KEYLENGTH=24, BLOCKSIZE=8};
    unsigned int BlockSize() const {return BLOCKSIZE;}

private:
    DES e1, d, e2;
};

class TripleDES_Decryption : public BlockTransformation
{
public:
    TripleDES_Decryption(const ibyte * userKey)
        : d1(userKey + 2*DES::KEYLENGTH, DECRYPTION), e(userKey + DES::KEYLENGTH, ENCRYPTION),
          d2(userKey, DECRYPTION) {}

    void ProcessBlock(const ibyte *inBlock, ibyte * outBlock);
    void ProcessBlock(ibyte * inoutBlock);

    enum {KEYLENGTH=24, BLOCKSIZE=8};
    unsigned int BlockSize() const {return BLOCKSIZE;}

private:
    DES d1, e, d2;
};

#endif
