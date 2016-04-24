// nbtheory.h - written and placed in the public domain by Wei Dai

#ifndef NBTHEORY_H
#define NBTHEORY_H

#include "modarith.h"
#include "eprecomp.h"
#include "smartptr.h"

NAMESPACE_BEGIN(NumberTheory)
    // export a table of small primes
    extern const unsigned maxPrimeTableSize;
    extern unsigned primeTableSize;
    extern word16 primeTable[];

    // build up the table to maxPrimeTableSize
    void BuildPrimeTable();

    // ************ primality testing ****************

    // generate a provable prime
    Integer ProvablePrime(RandomNumberGenerator &rng, unsigned int bits);

    bool IsSmallPrime(const Integer &p);

    // returns true if p is divisible by some prime less than bound
    // bound not be greater than the largest entry in the prime table
    bool TrialDivision(const Integer &p, unsigned bound);

    // returns true if p is NOT divisible by small primes
    bool SmallDivisorsTest(const Integer &p);

    // These is no reason to use these two, use the ones below instead
    bool IsFermatProbablePrime(const Integer &n, const Integer &b);
    bool IsLucasProbablePrime(const Integer &n);

    bool IsStrongProbablePrime(const Integer &n, const Integer &b);
    bool IsStrongLucasProbablePrime(const Integer &n);

    // Rabin-Miller primality test, i.e. repeating the strong probable prime test 
    // for several rounds with random bases
    bool RabinMillerTest(RandomNumberGenerator &rng, const Integer &w, unsigned int rounds);

    // small divisors test + strong probable prime test + strong Lucas probable prime test
    // should be good enough for all practical purposes
    // but feel free to change this to suit your level of paranoia
    bool IsPrime(const Integer &p);

    // use a fast sieve to find the next probable prime after p
    // returns true iff successful
    bool NextPrime(Integer &p, const Integer &max, bool blumInt=false);

    // ********** other number theoretic functions ************

    inline Integer GCD(const Integer &a, const Integer &b) 
        {return Integer::Gcd(a,b);}
    inline Integer LCM(const Integer &a, const Integer &b)
        {return a/GCD(a,b)*b;}
    inline Integer EuclideanMultiplicativeInverse(const Integer &a, const Integer &b)
        {return a.InverseMod(b);}

    // use Chinese Remainder Theorem to calculate x given x mod p and x mod q
    Integer CRT(const Integer &xp, const Integer &p, const Integer &xq, const Integer &q);
    // use this one if u = inverse of p mod q has been precalculated
    Integer CRT(const Integer &xp, const Integer &p, const Integer &xq, const Integer &q, const Integer &u);

    // if b is prime, then Jacobi(a, b) returns 0 if a%b==0, 1 if a is quadratic residue mod b, -1 otherwise
    // check a number theory book for what Jacobi symbol means when b is not prime
    int Jacobi(const Integer &a, const Integer &b);

    // calculates the Lucas function V_e(p, 1) mod n
    Integer Lucas(const Integer &e, const Integer &p, const Integer &n);
    // calculates x such that m==Lucas(e, x, p*q), p q primes
    Integer InverseLucas(const Integer &e, const Integer &m, const Integer &p, const Integer &q);
    // use this one if u=inverse of p mod q has been precalculated
    Integer InverseLucas(const Integer &e, const Integer &m, const Integer &p, const Integer &q, const Integer &u);

    inline Integer ModularExponentiation(const Integer &a, const Integer &e, const Integer &m)
        {return a_exp_b_mod_c(a, e, m);}
    // returns x such that x*x%p == a, p prime
    Integer ModularSquareRoot(const Integer &a, const Integer &p);
    // returns x such that a==ModularExponentiation(x, e, p*q), p q primes,
    // and e relatively prime to (p-1)*(q-1)
    Integer ModularRoot(const Integer &a, const Integer &e, const Integer &p, const Integer &q);
    // use this one if dp=d%(p-1), dq=d%(q-1), (d is inverse of e mod (p-1)*(q-1))
    // and u=inverse of p mod q have been precalculated
    Integer ModularRoot(const Integer &a, const Integer &dp, const Integer &dq, const Integer &p, const Integer &q, const Integer &u);

    // returns log base 2 of estimated number of operations to calculate discrete log or factor a number
    unsigned int DiscreteLogWorkFactor(unsigned int bitlength);
    unsigned int FactoringWorkFactor(unsigned int bitlength);
NAMESPACE_END

USING_NAMESPACE(NumberTheory)

// ********************************************************

class PrimeAndGenerator
{
public:
    // generate a random prime p of the form 2*q+delta, where q is also prime
    // warning: this is slow!
    PrimeAndGenerator(signed int delta, RandomNumberGenerator &rng, unsigned int pbits);
    // generate a random prime p of the form 2*r*q+delta, where q is also prime
    PrimeAndGenerator(signed int delta, RandomNumberGenerator &rng, unsigned int pbits, unsigned qbits);

    const Integer& Prime() const {return p;}
    const Integer& SubPrime() const {return q;}
    const Integer& Generator() const {return g;}

private:
    Integer p, q, g;
};

// ********************************************************

class ModExpPrecomputation
{
public:
    ModExpPrecomputation() {}
    ModExpPrecomputation(const ModExpPrecomputation &mep);
    ModExpPrecomputation(const Integer &modulus, const Integer &base, unsigned int maxExpBits, unsigned int storage);
    ~ModExpPrecomputation();

    void operator=(const ModExpPrecomputation &);
    
    void Precompute(const Integer &modulus, const Integer &base, unsigned int maxExpBits, unsigned int storage);
    void Load(const Integer &modulus, BufferedTransformation &storedPrecomputation);
    void Save(BufferedTransformation &storedPrecomputation) const;

    Integer Exponentiate(const Integer &exponent) const;
    Integer CascadeExponentiate(const Integer &exponent, ModExpPrecomputation pc2, const Integer &exponent2) const;

private:
    typedef MultiplicativeGroup<MontgomeryRepresentation> MR_MG;
    member_ptr<MontgomeryRepresentation> mr;
    member_ptr<MR_MG> mg;
    member_ptr< ExponentiationPrecomputation<MR_MG> > ep;
};

#endif
