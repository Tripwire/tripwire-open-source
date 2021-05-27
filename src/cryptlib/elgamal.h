#ifndef ELGAMAL_H
#define ELGAMAL_H

#include "nbtheory.h"

class ElGamalCryptoPublicKey : public PK_WithPrecomputation<PK_FixedLengthEncryptor>
{
public:
    ElGamalCryptoPublicKey(const Integer &p, const Integer &g, const Integer &y);
    ElGamalCryptoPublicKey(BufferedTransformation &bt);

    ~ElGamalCryptoPublicKey();

    void DEREncode(BufferedTransformation &bt) const;

    void Precompute(unsigned int precomputationStorage=16);
    void LoadPrecomputation(BufferedTransformation &storedPrecomputation);
    void SavePrecomputation(BufferedTransformation &storedPrecomputation) const;

    void Encrypt(RandomNumberGenerator &rng, const uint8_t *plainText, unsigned int plainTextLength, uint8_t *cipherText);

    unsigned int MaxPlainTextLength() const {return STDMIN(255U, modulusLen-3);}
    unsigned int CipherTextLength() const {return 2*modulusLen;}

protected:
    ElGamalCryptoPublicKey() {}
    void RawEncrypt(const Integer &k, const Integer &m, Integer &a, Integer &b) const;
    unsigned int ExponentBitLength() const;

    Integer p, g, y;
    unsigned int modulusLen;
    ModExpPrecomputation gpc, ypc;
};

class ElGamalCryptoPrivateKey : public ElGamalCryptoPublicKey, public PK_FixedLengthDecryptor
{
public:
    ElGamalCryptoPrivateKey(const Integer &p, const Integer &g, const Integer &y, const Integer &x);
    ElGamalCryptoPrivateKey(RandomNumberGenerator &rng, unsigned int pbits);
    // generate a random private key, given p and g
    ElGamalCryptoPrivateKey(RandomNumberGenerator &rng, const Integer &p, const Integer &g);

    ~ElGamalCryptoPrivateKey();

    ElGamalCryptoPrivateKey(BufferedTransformation &bt);
    void DEREncode(BufferedTransformation &bt) const;

    unsigned int Decrypt(const uint8_t *cipherText, uint8_t *plainText);

protected:
    void RawDecrypt(const Integer &a, const Integer &b, Integer &m) const;

    Integer x;
};

class ElGamalSigPublicKey : public PK_WithPrecomputation<PK_Verifier>
{
public:
    ElGamalSigPublicKey(const Integer &p, const Integer &q, const Integer &g, const Integer &y);
    ElGamalSigPublicKey(BufferedTransformation &bt);

    void DEREncode(BufferedTransformation &bt) const;

    void Precompute(unsigned int precomputationStorage=16);
    void LoadPrecomputation(BufferedTransformation &storedPrecomputation);
    void SavePrecomputation(BufferedTransformation &storedPrecomputation) const;

    bool Verify(const uint8_t *message, unsigned int messageLen, const uint8_t *signature);

    // message length for signature is unlimited, but only message digests should be signed
    unsigned int MaxMessageLength() const {return 0xffff;}
    unsigned int SignatureLength() const {return 2*qLen;}

    const Integer& GetPrime() { return p; }
    const Integer& GetParameterQ() { return q; }
    const Integer& GetParameterG() { return g; }
    const Integer& GetParameterY() { return y; }

protected:
    ElGamalSigPublicKey() {}
    bool RawVerify(const Integer &m, const Integer &a, const Integer &b) const;
    unsigned int ExponentBitLength() const;

    Integer p, q, g, y;
    unsigned int qLen;
    ModExpPrecomputation gpc, ypc;
};

class ElGamalSigPrivateKey : public ElGamalSigPublicKey, public PK_WithPrecomputation<PK_Signer>
{
public:
    ElGamalSigPrivateKey(const Integer &p, const Integer &q, const Integer &g, const Integer &y, const Integer &x);
    ElGamalSigPrivateKey(RandomNumberGenerator &rng, unsigned int pbits);
    // generate a random private key, given p and g
    ElGamalSigPrivateKey(RandomNumberGenerator &rng, const Integer &p, const Integer &q, const Integer &g);

    ElGamalSigPrivateKey(BufferedTransformation &bt);
    void DEREncode(BufferedTransformation &bt) const;

    void Sign(RandomNumberGenerator &rng, const uint8_t *message, unsigned int messageLen, uint8_t *signature);

    const Integer& GetParameterX() { return x; }

protected:
    void RawSign(RandomNumberGenerator &rng, const Integer &m, Integer &a, Integer &b) const;

    Integer x;
};

#endif
