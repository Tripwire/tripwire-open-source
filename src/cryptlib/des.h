#ifndef DES_H
#define DES_H

#include "cryptlib.h"
#include "misc.h"

class DES : public BlockTransformation
{
public:
    DES(const byte *userKey, CipherDir);

    void ProcessBlock(const byte *inBlock, byte * outBlock);
    void ProcessBlock(byte * inoutBlock)
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
    DESEncryption(const byte * userKey)
        : DES (userKey, ENCRYPTION) {}
};

class DESDecryption : public DES
{
public:
    DESDecryption(const byte * userKey)
        : DES (userKey, DECRYPTION) {}
};

class DES_EDE_Encryption : public BlockTransformation
{
public:
    DES_EDE_Encryption(const byte * userKey)
        : e(userKey, ENCRYPTION), d(userKey + DES::KEYLENGTH, DECRYPTION) {}

    void ProcessBlock(const byte *inBlock, byte * outBlock);
    void ProcessBlock(byte * inoutBlock);

    enum {KEYLENGTH=16, BLOCKSIZE=8};
    unsigned int BlockSize() const {return BLOCKSIZE;}

private:
    DES e, d;
};

class DES_EDE_Decryption : public BlockTransformation
{
public:
    DES_EDE_Decryption(const byte * userKey)
        : d(userKey, DECRYPTION), e(userKey + DES::KEYLENGTH, ENCRYPTION) {}

    void ProcessBlock(const byte *inBlock, byte * outBlock);
    void ProcessBlock(byte * inoutBlock);

    enum {KEYLENGTH=16, BLOCKSIZE=8};
    unsigned int BlockSize() const {return BLOCKSIZE;}

private:
    DES d, e;
};

class TripleDES_Encryption : public BlockTransformation
{
public:
    TripleDES_Encryption(const byte * userKey)
        : e1(userKey, ENCRYPTION), d(userKey + DES::KEYLENGTH, DECRYPTION),
          e2(userKey + 2*DES::KEYLENGTH, ENCRYPTION) {}

    void ProcessBlock(const byte *inBlock, byte * outBlock);
    void ProcessBlock(byte * inoutBlock);

    enum {KEYLENGTH=24, BLOCKSIZE=8};
    unsigned int BlockSize() const {return BLOCKSIZE;}

private:
    DES e1, d, e2;
};

class TripleDES_Decryption : public BlockTransformation
{
public:
    TripleDES_Decryption(const byte * userKey)
        : d1(userKey + 2*DES::KEYLENGTH, DECRYPTION), e(userKey + DES::KEYLENGTH, ENCRYPTION),
          d2(userKey, DECRYPTION) {}

    void ProcessBlock(const byte *inBlock, byte * outBlock);
    void ProcessBlock(byte * inoutBlock);

    enum {KEYLENGTH=24, BLOCKSIZE=8};
    unsigned int BlockSize() const {return BLOCKSIZE;}

private:
    DES d1, e, d2;
};

#endif
