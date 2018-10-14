// elgamal.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "elgamal.h"
#include "asn.h"

ElGamalCryptoPublicKey::ElGamalCryptoPublicKey(const Integer &p, const Integer &g, const Integer &y)
    : p(p), g(g), y(y), modulusLen(p.ByteCount()),
      gpc(p, g, ExponentBitLength(), 1), ypc(p, y, ExponentBitLength(), 1)
{
}

ElGamalCryptoPublicKey::~ElGamalCryptoPublicKey()
{
    // Note: This empty descrutor was added to get around a SunPro compiler bug.  
    // Without this destructor, the code core dumped.
}

ElGamalCryptoPublicKey::ElGamalCryptoPublicKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    p.BERDecode(seq);
    modulusLen=p.ByteCount();
    g.BERDecode(seq);
    y.BERDecode(seq);
    gpc.Precompute(p, g, ExponentBitLength(), 1);
    ypc.Precompute(p, y, ExponentBitLength(), 1);
}

void ElGamalCryptoPublicKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    p.DEREncode(seq);
    g.DEREncode(seq);
    y.DEREncode(seq);
}

void ElGamalCryptoPublicKey::Precompute(unsigned int precomputationStorage)
{
    gpc.Precompute(p, g, ExponentBitLength(), precomputationStorage);
    ypc.Precompute(p, y, ExponentBitLength(), precomputationStorage);
}

void ElGamalCryptoPublicKey::LoadPrecomputation(BufferedTransformation &bt)
{
    gpc.Load(p, bt);
    ypc.Load(p, bt);
}

void ElGamalCryptoPublicKey::SavePrecomputation(BufferedTransformation &bt) const
{
    gpc.Save(bt);
    ypc.Save(bt);
}

void ElGamalCryptoPublicKey::Encrypt(RandomNumberGenerator &rng, const uint8_t *plainText, unsigned int plainTextLength, uint8_t *cipherText)
{
    assert(plainTextLength <= MaxPlainTextLength());

    SecByteBlock block(modulusLen-1);
    rng.GetBlock(block, modulusLen-2-plainTextLength);
    memcpy(block+modulusLen-2-plainTextLength, plainText, plainTextLength);
    block[modulusLen-2] = plainTextLength;

    Integer m(block, modulusLen-1);
    Integer a,b;
    RawEncrypt(Integer(rng, ExponentBitLength()), m, a, b);

    a.Encode(cipherText, modulusLen);
    b.Encode(cipherText+modulusLen, modulusLen);
}

void ElGamalCryptoPublicKey::RawEncrypt(const Integer &k, const Integer &m, Integer &a, Integer &b) const
{
//    a = a_exp_b_mod_c(g, k, p);
//    b = m * a_exp_b_mod_c(y, k, p) % p;
    a = gpc.Exponentiate(k);
    b = m * ypc.Exponentiate(k) % p;
}

unsigned int ElGamalCryptoPublicKey::ExponentBitLength() const
{
    return 2*DiscreteLogWorkFactor(p.BitCount());
}

// *************************************************************

ElGamalCryptoPrivateKey::ElGamalCryptoPrivateKey(const Integer &p, const Integer &g, const Integer &y, const Integer &x)
    : ElGamalCryptoPublicKey(p, g, y), x(x)
{
}

ElGamalCryptoPrivateKey::ElGamalCryptoPrivateKey(RandomNumberGenerator &rng, unsigned int pbits)
{
    PrimeAndGenerator pg(1, rng, pbits);
    p = pg.Prime();
    modulusLen=p.ByteCount();
    g = pg.Generator();
    x.Randomize(rng, ExponentBitLength());
    gpc.Precompute(p, g, ExponentBitLength(), 1);
    y = gpc.Exponentiate(x);
    ypc.Precompute(p, y, ExponentBitLength(), 1);
}

ElGamalCryptoPrivateKey::ElGamalCryptoPrivateKey(RandomNumberGenerator &rng, const Integer &pIn, const Integer &gIn)
{
    p = pIn;
    modulusLen=p.ByteCount();
    g = gIn;
    x.Randomize(rng, ExponentBitLength());
    gpc.Precompute(p, g, ExponentBitLength(), 1);
    y = gpc.Exponentiate(x);
    ypc.Precompute(p, y, ExponentBitLength(), 1);
}

ElGamalCryptoPrivateKey::~ElGamalCryptoPrivateKey()
{
    // Note: This empty descrutor was added to get around a SunPro compiler bug.  
    // Without this destructor, the code core dumped.
}

ElGamalCryptoPrivateKey::ElGamalCryptoPrivateKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    p.BERDecode(seq);
    modulusLen=p.ByteCount();
    g.BERDecode(seq);
    y.BERDecode(seq);
    x.BERDecode(seq);
    gpc.Precompute(p, g, ExponentBitLength(), 1);
    ypc.Precompute(p, y, ExponentBitLength(), 1);
}

void ElGamalCryptoPrivateKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    p.DEREncode(seq);
    g.DEREncode(seq);
    y.DEREncode(seq);
    x.DEREncode(seq);
}

unsigned int ElGamalCryptoPrivateKey::Decrypt(const uint8_t *cipherText, uint8_t *plainText)
{
    Integer a(cipherText, modulusLen);
    Integer b(cipherText+modulusLen, modulusLen);
    Integer m;

    RawDecrypt(a, b, m);
    m.Encode(plainText, 1);
    unsigned int plainTextLength = plainText[0];
    if (plainTextLength > MaxPlainTextLength())
        return 0;
    m >>= 8;
    m.Encode(plainText, plainTextLength);
    return plainTextLength;
}

void ElGamalCryptoPrivateKey::RawDecrypt(const Integer &a, const Integer &b, Integer &m) const
{
    if (x.BitCount()+20 < p.BitCount()) // if x is short
        m = b * EuclideanMultiplicativeInverse(a_exp_b_mod_c(a, x, p), p) % p;
    else    // save a multiplicative inverse calculation
        m = b * a_exp_b_mod_c(a, p-1-x, p) % p;
}

// ******************************************************************

ElGamalSigPublicKey::ElGamalSigPublicKey(const Integer &p, const Integer &q, const Integer &g, const Integer &y)
    : p(p), q(q), g(g), y(y), qLen(q.ByteCount()),
      gpc(p, g, ExponentBitLength(), 1), ypc(p, y, ExponentBitLength(), 1)
{
}

ElGamalSigPublicKey::ElGamalSigPublicKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    p.BERDecode(seq);
    q.BERDecode(seq);
    g.BERDecode(seq);
    y.BERDecode(seq);
    qLen = q.ByteCount();
    gpc.Precompute(p, g, ExponentBitLength(), 1);
    ypc.Precompute(p, y, ExponentBitLength(), 1);
}

void ElGamalSigPublicKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    p.DEREncode(seq);
    q.DEREncode(seq);
    g.DEREncode(seq);
    y.DEREncode(seq);
}

void ElGamalSigPublicKey::Precompute(unsigned int precomputationStorage)
{
    gpc.Precompute(p, g, ExponentBitLength(), precomputationStorage);
    ypc.Precompute(p, y, ExponentBitLength(), precomputationStorage);
}

void ElGamalSigPublicKey::LoadPrecomputation(BufferedTransformation &bt)
{
    gpc.Load(p, bt);
    ypc.Load(p, bt);
}

void ElGamalSigPublicKey::SavePrecomputation(BufferedTransformation &bt) const
{
    gpc.Save(bt);
    ypc.Save(bt);
}

bool ElGamalSigPublicKey::Verify(const uint8_t *message, unsigned int messageLen, const uint8_t *signature)
{
    assert(messageLen <= MaxMessageLength());

    Integer m(message, messageLen);
    Integer r(signature, qLen);
    Integer s(signature+qLen, qLen);
    return RawVerify(m, r, s);
}

bool ElGamalSigPublicKey::RawVerify(const Integer &m, const Integer &r, const Integer &s) const
{
    // check r != 0 && r == (g^s * y^r + m) mod q
    return !!r && r == (gpc.CascadeExponentiate(s, ypc, r) + m) % q;
}

unsigned int ElGamalSigPublicKey::ExponentBitLength() const
{
    return q.BitCount();
}

// *************************************************************

ElGamalSigPrivateKey::ElGamalSigPrivateKey(const Integer &p, const Integer &q, const Integer &g, const Integer &y, const Integer &x)
    : ElGamalSigPublicKey(p, q, g, y), x(x)
{
}

ElGamalSigPrivateKey::ElGamalSigPrivateKey(RandomNumberGenerator &rng, unsigned int pbits)
{
    PrimeAndGenerator pg(1, rng, pbits, 2*DiscreteLogWorkFactor(pbits));
    p = pg.Prime();
    q = pg.SubPrime();
    g = pg.Generator();
    x.Randomize(rng, 2, q-2, Integer::ANY);
    gpc.Precompute(p, g, ExponentBitLength(), 1);
    y = gpc.Exponentiate(x);
    ypc.Precompute(p, y, ExponentBitLength(), 1);
    qLen = q.ByteCount();
}

ElGamalSigPrivateKey::ElGamalSigPrivateKey(RandomNumberGenerator &rng, const Integer &pIn, const Integer &qIn, const Integer &gIn)
{
    p = pIn;
    q = qIn;
    g = gIn;
    x.Randomize(rng, 2, q-2, Integer::ANY);
    gpc.Precompute(p, g, ExponentBitLength(), 1);
    y = gpc.Exponentiate(x);
    ypc.Precompute(p, y, ExponentBitLength(), 1);
    qLen = q.ByteCount();
}

ElGamalSigPrivateKey::ElGamalSigPrivateKey(BufferedTransformation &bt)
{
    BERSequenceDecoder seq(bt);
    p.BERDecode(seq);
    q.BERDecode(seq);
    g.BERDecode(seq);
    y.BERDecode(seq);
    x.BERDecode(seq);
    gpc.Precompute(p, g, ExponentBitLength(), 1);
    ypc.Precompute(p, y, ExponentBitLength(), 1);
    qLen = q.ByteCount();
}

void ElGamalSigPrivateKey::DEREncode(BufferedTransformation &bt) const
{
    DERSequenceEncoder seq(bt);
    p.DEREncode(seq);
    q.DEREncode(seq);
    g.DEREncode(seq);
    y.DEREncode(seq);
    x.DEREncode(seq);
}

void ElGamalSigPrivateKey::Sign(RandomNumberGenerator &rng, const uint8_t *message, unsigned int messageLen, uint8_t *signature)
{
    assert(messageLen <= MaxMessageLength());

    Integer m(message, messageLen);
    Integer r;
    Integer s;

    RawSign(rng, m, r, s);
    r.Encode(signature, qLen);
    s.Encode(signature+qLen, qLen);
}

void ElGamalSigPrivateKey::RawSign(RandomNumberGenerator &rng, const Integer &m, Integer &r, Integer &s) const
{
    do
    {
        Integer k(rng, 2, q-2, Integer::ANY);
        r = (gpc.Exponentiate(k) + m) % q;
        s = (k - x*r) % q;
    } while (!r);           // make sure r != 0
}
