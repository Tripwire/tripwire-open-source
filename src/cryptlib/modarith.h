#ifndef MODARITH_H
#define MODARITH_H

// implementations are in integer.cpp

#include "cryptlib.h"
#include "misc.h"
#include "integer.h"
#include "algebra.h"

class ModularArithmetic : public AbstractRing<Integer>
{
public:
    ModularArithmetic(const Integer &modulus)
        : modulus(modulus), result((word)0, modulus.reg.size) {}

    ModularArithmetic(const ModularArithmetic &ma)
        : modulus(ma.modulus), result((word)0, modulus.reg.size) {}

    const Integer& GetModulus() const {return modulus;}

    virtual Integer ConvertIn(const Integer &a) const
        {return a%modulus;}

    virtual Integer ConvertOut(const Integer &a) const
        {return a;}

    bool Equal(const Integer &a, const Integer &b) const
        {return a==b;}

    Integer Zero() const
        {return Integer::Zero();}

    Integer Add(const Integer &a, const Integer &b) const;

    Integer& Accumulate(Integer &a, const Integer &b) const;

    Integer Inverse(const Integer &a) const;

    Integer Subtract(const Integer &a, const Integer &b) const;

    Integer& Reduce(Integer &a, const Integer &b) const;

    Integer Double(const Integer &a) const
        {return Add(a, a);}

    virtual Integer One() const
        {return Integer::One();}

    virtual Integer Multiply(const Integer &a, const Integer &b) const
        {return a*b%modulus;}

    virtual Integer Square(const Integer &a) const
        {return a.Squared()%modulus;}

    virtual bool IsUnit(const Integer &a) const
        {return Integer::Gcd(a, modulus).IsUnit();}

    virtual Integer MultiplicativeInverse(const Integer &a) const;

    Integer Divide(const Integer &a, const Integer &b) const
        {return Multiply(a, MultiplicativeInverse(b));}

    virtual Integer Exponentiate(const Integer &a, const Integer &e) const;

    virtual Integer CascadeExponentiate(const Integer &x, const Integer &e1, const Integer &y, const Integer &e2) const;

    unsigned int MaxElementBitLength() const
        {return (modulus-1).BitCount();}

    unsigned int MaxElementByteLength() const
        {return (modulus-1).ByteCount();}

protected:
    Integer modulus, result;
};

// do modular arithmetics in Montgomery representation for increased speed
class MontgomeryRepresentation : public ModularArithmetic
{
public:
    MontgomeryRepresentation(const Integer &modulus);   // modulus must be odd

    Integer ConvertIn(const Integer &a) const
        {return (a<<(WORD_BITS*modulus.reg.size))%modulus;}

    Integer ConvertOut(const Integer &a) const;

    Integer One() const
        {return Integer::Power2(WORD_BITS*modulus.reg.size)%modulus;}

    Integer Multiply(const Integer &a, const Integer &b) const;

    Integer Square(const Integer &a) const;

    Integer MultiplicativeInverse(const Integer &a) const;

    Integer Exponentiate(const Integer &a, const Integer &e) const
        {return AbstractRing<Integer>::Exponentiate(a, e);}

    Integer CascadeExponentiate(const Integer &x, const Integer &e1, const Integer &y, const Integer &e2) const
        {return AbstractRing<Integer>::CascadeExponentiate(x, e1, y, e2);}

private:
    Integer u;
    SecWordBlock workspace;
};

// another alternative representation, e-mail me if you figure out how it works :)
class HalfMontgomeryRepresentation : public ModularArithmetic
{
public:
    HalfMontgomeryRepresentation(const Integer &modulus);   // modulus must be odd

    Integer ConvertIn(const Integer &a) const
        {return (a<<(WORD_BITS*modulus.reg.size/2))%modulus;}

    Integer ConvertOut(const Integer &a) const;

    Integer One() const
        {return Integer::Power2(WORD_BITS*modulus.reg.size/2)%modulus;}

    Integer Multiply(const Integer &a, const Integer &b) const;

    Integer Square(const Integer &a) const;

    Integer MultiplicativeInverse(const Integer &a) const;

    Integer Exponentiate(const Integer &a, const Integer &e) const
        {return AbstractRing<Integer>::Exponentiate(a, e);}

    Integer CascadeExponentiate(const Integer &x, const Integer &e1, const Integer &y, const Integer &e2) const
        {return AbstractRing<Integer>::CascadeExponentiate(x, e1, y, e2);}

private:
    Integer v, u;
    SecWordBlock workspace;
};

class GFP : public ModularArithmetic
{
public:
    GFP(const Integer &modulus)
        : ModularArithmetic(modulus) {}

    bool IsUnit(const Integer &a) const
        {return !!a;}

    Integer FieldSize() const
        {return modulus;}
};

#endif
