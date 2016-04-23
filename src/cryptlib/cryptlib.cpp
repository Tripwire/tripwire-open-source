// cryptlib.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "cryptlib.h"
#include "misc.h"

unsigned int RandomNumberGenerator::GetBit()
{
    return Parity(GetByte());
}

void RandomNumberGenerator::GetBlock(byte *output, unsigned int size)
{
    while (size--)
        *output++ = GetByte();
}

word32 RandomNumberGenerator::GetLong(word32 min, word32 max)
{
    word32 range = max-min;
    const int maxBytes = BytePrecision(range);
    const int maxBits = BitPrecision(range);

    word32 value;

    do
    {
        value = 0;
        for (int i=0; i<maxBytes; i++)
            value = (value << 8) | GetByte();

        value = Crop(value, maxBits);
    } while (value > range);

    return value+min;
}

void StreamCipher::ProcessString(byte *outString, const byte *inString, unsigned int length)
{
    while(length--)
        *outString++ = ProcessByte(*inString++);
}

void StreamCipher::ProcessString(byte *inoutString, unsigned int length)
{
    for(;length--; inoutString++)
        *inoutString = ProcessByte(*inoutString);
}

bool MessageAuthenticationCode::Verify(const byte *macIn)
{
    SecByteBlock mac(DigestSize());
    Final(mac);
    return memcmp(mac, macIn, DigestSize()) == 0;
}

void BufferedTransformation::TransferTo(BufferedTransformation &target)
{
    SecByteBlock buf(256);
    unsigned int l;

    while ((l=Get(buf, 256)) != 0)
        target.Put(buf, l);
}

unsigned int BufferedTransformation::TransferTo(BufferedTransformation &target, unsigned int size)
{
    SecByteBlock buf(256);
    unsigned int l, total = 0;

    while (size && (l=Get(buf, STDMIN(size, 256U))))
    {
        target.Put(buf, l);
        size -= l;
        total += l;
    }
    return total;
}

void BufferedTransformation::PutShort(word16 value, bool highFirst)
{
    if (highFirst)
    {
        Put(value>>8);
        Put(byte(value));
    }
    else
    {
        Put(byte(value));
        Put(value>>8);
    }
}

void BufferedTransformation::PutLong(word32 value, bool highFirst)
{
    if (highFirst)
    {
        for (int i=0; i<4; i++)
            Put(byte(value>>((3-i)*8)));
    }
    else
    {
        for (int i=0; i<4; i++)
            Put(byte(value>>(i*8)));
    }
}

int BufferedTransformation::GetShort(word16 &value, bool highFirst)
{
    if (MaxRetrieveable()<2)
        return 0;

    byte buf[2];
    Get(buf, 2);

    if (highFirst)
        value = (buf[0] << 8) | buf[1];
    else
        value = (buf[1] << 8) | buf[0];

    return 2;
}

int BufferedTransformation::GetLong(word32 &value, bool highFirst)
{
    if (MaxRetrieveable()<4)
        return 0;

    byte buf[4];
    Get(buf, 4);

    if (highFirst)
        value = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf [3];
    else
        value = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf [0];

    return 4;
}

unsigned int BufferedTransformation::Skip(unsigned int skipMax)
{
    byte b;
    unsigned int skipActual=0;

    while (skipMax-- && Get(b))
        skipActual++;
    return skipActual;
}

unsigned int PK_FixedLengthCryptoSystem::MaxPlainTextLength(unsigned int cipherTextLength) const
{
    if (cipherTextLength == CipherTextLength())
        return MaxPlainTextLength();
    else
        return 0;
}

unsigned int PK_FixedLengthCryptoSystem::CipherTextLength(unsigned int plainTextLength) const
{
    if (plainTextLength <= MaxPlainTextLength())
        return CipherTextLength();
    else
        return 0;
}

unsigned int PK_FixedLengthDecryptor::Decrypt(const byte *cipherText, unsigned int cipherTextLength, byte *plainText)
{
    if (cipherTextLength != CipherTextLength())
        return 0;

    return Decrypt(cipherText, plainText);
}

bool PK_VerifierWithRecovery::Verify(const byte *message, unsigned int messageLength, const byte *signature)
{
    SecByteBlock recovered(MaxMessageLength());
    unsigned int rLen = Recover(signature, recovered);
    return (rLen==messageLength && memcmp(recovered, message, rLen)==0);
}
