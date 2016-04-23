// nbtheory.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "nbtheory.h"
#include "misc.h"
#include "asn.h"
#include "algebra.h"

#include <math.h>

#include "algebra.cpp"
#include "eprecomp.cpp"

USING_NAMESPACE(std)

NAMESPACE_BEGIN(NumberTheory)

const unsigned maxPrimeTableSize = 3511;
unsigned primeTableSize=552;

word16 primeTable[maxPrimeTableSize] =
    {2, 3, 5, 7, 11, 13, 17, 19,
    23, 29, 31, 37, 41, 43, 47, 53,
    59, 61, 67, 71, 73, 79, 83, 89,
    97, 101, 103, 107, 109, 113, 127, 131,
    137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223,
    227, 229, 233, 239, 241, 251, 257, 263,
    269, 271, 277, 281, 283, 293, 307, 311,
    313, 317, 331, 337, 347, 349, 353, 359,
    367, 373, 379, 383, 389, 397, 401, 409,
    419, 421, 431, 433, 439, 443, 449, 457,
    461, 463, 467, 479, 487, 491, 499, 503,
    509, 521, 523, 541, 547, 557, 563, 569,
    571, 577, 587, 593, 599, 601, 607, 613,
    617, 619, 631, 641, 643, 647, 653, 659,
    661, 673, 677, 683, 691, 701, 709, 719,
    727, 733, 739, 743, 751, 757, 761, 769,
    773, 787, 797, 809, 811, 821, 823, 827,
    829, 839, 853, 857, 859, 863, 877, 881,
    883, 887, 907, 911, 919, 929, 937, 941,
    947, 953, 967, 971, 977, 983, 991, 997,
    1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049,
    1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097,
    1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163,
    1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223,
    1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283,
    1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321,
    1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423,
    1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459,
    1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511,
    1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571,
    1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619,
    1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693,
    1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747,
    1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811,
    1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877,
    1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949,
    1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003,
    2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069,
    2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129,
    2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203,
    2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267,
    2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311,
    2333, 2339, 2341, 2347, 2351, 2357, 2371, 2377,
    2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423,
    2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503,
    2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579,
    2591, 2593, 2609, 2617, 2621, 2633, 2647, 2657,
    2659, 2663, 2671, 2677, 2683, 2687, 2689, 2693,
    2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741,
    2749, 2753, 2767, 2777, 2789, 2791, 2797, 2801,
    2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861,
    2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939,
    2953, 2957, 2963, 2969, 2971, 2999, 3001, 3011,
    3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079,
    3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167,
    3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221,
    3229, 3251, 3253, 3257, 3259, 3271, 3299, 3301,
    3307, 3313, 3319, 3323, 3329, 3331, 3343, 3347,
    3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413,
    3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491,
    3499, 3511, 3517, 3527, 3529, 3533, 3539, 3541,
    3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607,
    3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671,
    3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727,
    3733, 3739, 3761, 3767, 3769, 3779, 3793, 3797,
    3803, 3821, 3823, 3833, 3847, 3851, 3853, 3863,
    3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923,
    3929, 3931, 3943, 3947, 3967, 3989, 4001, 4003};

void BuildPrimeTable()
{
    unsigned int p=primeTable[primeTableSize-1];
    for (unsigned int i=primeTableSize; i<maxPrimeTableSize; i++)
    {
        int j;
        do
        {
            p+=2;
            for (j=1; j<54; j++)
                if (p%primeTable[j] == 0)
                    break;
        } while (j!=54);
        primeTable[i] = p;
    }
    primeTableSize = maxPrimeTableSize;
}

bool IsSmallPrime(const Integer &p)
{
    BuildPrimeTable();

    if (p>primeTable[primeTableSize-1])
        return false;
    if (p==primeTable[primeTableSize-1])
        return true;

    for (unsigned i=0; primeTable[i]<=p; i++)
        if (p == primeTable[i])
            return true;

    return false;
}

bool TrialDivision(const Integer &p, unsigned bound)
{
    assert(primeTable[primeTableSize-1] >= bound);

    unsigned int i;
    for (i = 0; primeTable[i]<bound; i++)
        if ((p % primeTable[i]) == 0)
            return true;

    if (bound == primeTable[i])
        return (p % bound == 0);
    else
        return false;
}

bool SmallDivisorsTest(const Integer &p)
{
    BuildPrimeTable();
    return !TrialDivision(p, primeTable[primeTableSize-1]);
}

bool IsFermatProbablePrime(const Integer &n, const Integer &b)
{
    assert(n>1 && b>Integer::Zero() && b<n);
    return a_exp_b_mod_c(b, n-1, n)==1;
}

bool IsStrongProbablePrime(const Integer &n, const Integer &b)
{
    assert(n>1 && b>Integer::Zero() && b<n);

    if ((n.IsEven() && n!=2) || GCD(b, n) != 1)
        return false;

    Integer nminus1 = (n-1);
    unsigned int a;

    // calculate a = largest power of 2 that divides (n-1)
    for (a=0; ; a++)
        if (nminus1.GetBit(a))
            break;
    Integer m = nminus1>>a;

    Integer z = a_exp_b_mod_c(b, m, n);
    if (z==1 || z==nminus1)
        return true;
    for (unsigned j=1; j<a; j++)
    {
        z = z.Squared()%n;
        if (z==nminus1)
            return true;
        if (z==1)
            return false;
    }
    return false;
}

bool RabinMillerTest(RandomNumberGenerator &rng, const Integer &w, unsigned int rounds)
{
    Integer b;
    for (unsigned int i=0; i<rounds; i++)
    {
        b.Randomize(rng, 2, w-1);
        if (!IsStrongProbablePrime(w, b))
            return false;
    }
    return true;
}

bool IsLucasProbablePrime(const Integer &n)
{
    assert(n>1);

    if (n.IsEven())
        return n==2;

    Integer b=1, d;
    unsigned int i=0;
    int j;

    do
    {
        if (++i==64 && n.IsSquare())    // avoid infinite loop if n is a square
            return false;
        ++b; ++b;
        d = (b.Squared()-4)%n;
    }
    while ((j=Jacobi(d,n)) == 1);

    if (j==0) 
        return false;
    else
        return Lucas(n+1, b, n)==2;
}

bool IsStrongLucasProbablePrime(const Integer &n)
{
    assert(n>1);

    if (n.IsEven())
        return n==2;

    Integer b=1, d;
    unsigned int i=0;
    int j;

    do
    {
        if (++i==64 && n.IsSquare())    // avoid infinite loop if n is a square
            return false;
        ++b; ++b;
        d = (b.Squared()-4)%n;
    }
    while ((j=Jacobi(d,n)) == 1);

    if (j==0) 
        return false;

    Integer n1 = n-j;
    unsigned int a;

    // calculate a = largest power of 2 that divides n1
    for (a=0; ; a++)
        if (n1.GetBit(a))
            break;
    Integer m = n1>>a;

    Integer z = Lucas(m, b, n);
    if (z==2 || z==n-2)
        return true;
    for (i=1; i<a; i++)
    {
        z = (z.Squared()-2)%n;
        if (z==n-2)
            return true;
        if (z==2)
            return false;
    }
    return false;
}

bool IsPrime(const Integer &p)
{
    return (IsSmallPrime(p) || (SmallDivisorsTest(p) 
            && IsStrongProbablePrime(p, 3) && IsStrongLucasProbablePrime(p)));
}

class RemainderTable
{
public:
    RemainderTable(const Integer &p);
    bool HasZero() const;
    void Increment();
    void IncrementBy(unsigned int i);
    void IncrementBy(const RemainderTable &rtQ);

private:
    SecBlock<word16> table;
};

RemainderTable::RemainderTable(const Integer &p)
    : table((BuildPrimeTable(), primeTableSize))
{
    for (unsigned int i=0; i<primeTableSize; i++)
        table[i] = (word16)(p%primeTable[i]);
}

bool RemainderTable::HasZero() const
{
    unsigned int i;
    for (i=0; i<primeTableSize; i++)
        if (!table[i])
            break;

    return (i!=primeTableSize);
}

void RemainderTable::Increment()
{
    for (unsigned int i=0; i<primeTableSize; i++)
    {
        table[i]++;
        if (table[i]==primeTable[i])
            table[i] = 0;
    }
}

void RemainderTable::IncrementBy(unsigned int increment)
{
    for (unsigned int i=0; i<primeTableSize; i++)
    {
        table[i] += increment;
        while (table[i]>=primeTable[i])
            table[i]-=primeTable[i];
    }
}

void RemainderTable::IncrementBy(const RemainderTable &rtQ)
{
    for (unsigned int i=0; i<primeTableSize; i++)
    {
        table[i] += rtQ.table[i];
        if (table[i]>=primeTable[i])
            table[i]-=primeTable[i];
    }
}

inline bool FastProbablePrimeTest(const Integer &n)
{
    return IsStrongProbablePrime(n,2);
}

bool NextPrime(Integer &p, const Integer &max, bool blumInt)
{
    BuildPrimeTable();

    if (p<primeTable[primeTableSize-1])
    {
        for (unsigned i=0; i<primeTableSize; i++)
            if (p<primeTable[i])
            {
                p = primeTable[i];
                return p <= max;
            }
        assert(false);  // shouldn't reach here
    }

    ++p;

    if (p.IsEven())
        ++p;

    if (blumInt && !p.GetBit(1))
        {++p; ++p;}

    if (p>max)
        return false;

    RemainderTable rt(p);

    while (rt.HasZero() || !FastProbablePrimeTest(p) || !IsPrime(p))
    {
        rt.IncrementBy(blumInt ? 4 : 2);
        ++p; ++p;
        if (blumInt)
            {++p; ++p;}

        if (p>max)
            return false;
    }

    return true;
}

Integer ProvablePrime(RandomNumberGenerator &rng, unsigned int bits)
{
    const unsigned smallPrimeBound = 29, c_opt=10;
    Integer p;

    BuildPrimeTable();
    if (bits < smallPrimeBound)
    {
        do
            p.Randomize(rng, Integer::Power2(bits-1), Integer::Power2(bits)-1, Integer::ODD);
        while (TrialDivision(p, 1 << ((bits+1)/2)));
    }
    else
    {
        const unsigned margin = bits > 50 ? 20 : (bits-10)/2;
        double relativeSize;
        do
            relativeSize = pow(2.0, double(rng.GetLong())/0xffffffff - 1);
        while (bits * relativeSize >= bits - margin);

        Integer a,b;
        Integer q = ProvablePrime(rng, unsigned(bits*relativeSize));
        Integer I = Integer::Power2(bits-2)/q;
        Integer I2 = I << 1;
        unsigned int trialDivisorBound = (unsigned int)STDMIN((unsigned long)primeTable[primeTableSize-1], (unsigned long)bits*bits/c_opt);
        bool success = false;
        do
        {
            p.Randomize(rng, I, I2, Integer::ANY);
            p *= q; p <<= 1; ++p;
            if (!TrialDivision(p, trialDivisorBound))
            {
                a.Randomize(rng, 2, p-1, Integer::ANY);
                b = a_exp_b_mod_c(a, (p-1)/q, p);
                success = (GCD(b-1, p) == 1) && (a_exp_b_mod_c(b, q, p) == 1);
            }
        }
        while (!success);
    }
    return p;
}

Integer CRT(const Integer &xp, const Integer &p, const Integer &xq, const Integer &q, const Integer &u)
{
    // isn't operator overloading great?
    return p * (u * (xq-xp) % q) + xp;
}

Integer CRT(const Integer &xp, const Integer &p, const Integer &xq, const Integer &q)
{
    return CRT(xp, p, xq, q, EuclideanMultiplicativeInverse(p, q));
}

Integer ModularSquareRoot(const Integer &a, const Integer &p)
{
    if (p%4 == 3)
        return a_exp_b_mod_c(a, (p+1)/4, p);

    Integer q=p-1;
    unsigned int r=0;
    while (q.IsEven())
    {
        r++;
        q >>= 1;
    }

    Integer n=2;
    while (Jacobi(n, p) != -1)
        ++n;

    Integer y = a_exp_b_mod_c(n, q, p);
    Integer x = a_exp_b_mod_c(a, (q-1)/2, p);
    Integer b = (x.Squared()%p)*a%p;
    x = a*x%p;
    Integer tempb, t;

    while (b != 1)
    {
        unsigned m=0;
        tempb = b;
        do
        {
            m++;
            b = b.Squared()%p;
            if (m==r)
                return Integer::Zero();
        }
        while (b != 1);

        t = y;
        for (unsigned i=0; i<r-m-1; i++)
            t = t.Squared()%p;
        y = t.Squared()%p;
        r = m;
        x = x*t%p;
        b = tempb*y%p;
    }

    assert(x.Squared()%p == a);
    return x;
}

Integer ModularRoot(const Integer &a, const Integer &dp, const Integer &dq,
                    const Integer &p, const Integer &q, const Integer &u)
{
    Integer p2 = ModularExponentiation((a % p), dp, p);
    Integer q2 = ModularExponentiation((a % q), dq, q);
    return CRT(p2, p, q2, q, u);
}

Integer ModularRoot(const Integer &a, const Integer &e,
                    const Integer &p, const Integer &q)
{
    Integer dp = EuclideanMultiplicativeInverse(e, p-1);
    Integer dq = EuclideanMultiplicativeInverse(e, q-1);
    Integer u = EuclideanMultiplicativeInverse(p, q);
    assert(!!dp && !!dq && !!u);
    return ModularRoot(a, dp, dq, p, q, u);
}

/*
Integer GCDI(const Integer &x, const Integer &y)
{
    Integer a=x, b=y;
    unsigned k=0;

    assert(!!a && !!b);

    while (a[0]==0 && b[0]==0)
    {
        a >>= 1;
        b >>= 1;
        k++;
    }

    while (a[0]==0)
        a >>= 1;

    while (b[0]==0)
        b >>= 1;

    while (1)
    {
        switch (a.Compare(b))
        {
            case -1:
                b -= a;
                while (b[0]==0)
                    b >>= 1;
                break;

            case 0:
                return (a <<= k);

            case 1:
                a -= b;
                while (a[0]==0)
                    a >>= 1;
                break;

            default:
                assert(false);
        }
    }
}

Integer EuclideanMultiplicativeInverse(const Integer &a, const Integer &b)
{
    assert(b.Positive());

    if (a.Negative())
        return EuclideanMultiplicativeInverse(a%b, b);

    if (b[0]==0)
    {
        if (!b || a[0]==0)
            return Integer::Zero();       // no inverse
        if (a==1)
            return 1;
        Integer u = EuclideanMultiplicativeInverse(b, a);
        if (!u)
            return Integer::Zero();       // no inverse
        else
            return (b*(a-u)+1)/a;
    }

    Integer u=1, d=a, v1=b, v3=b, t1, t3, b2=(b+1)>>1;

    if (a[0])
    {
        t1 = Integer::Zero();
        t3 = -b;
    }
    else
    {
        t1 = b2;
        t3 = a>>1;
    }

    while (!!t3)
    {
        while (t3[0]==0)
        {
            t3 >>= 1;
            if (t1[0]==0)
                t1 >>= 1;
            else
            {
                t1 >>= 1;
                t1 += b2;
            }
        }
        if (t3.Positive())
        {
            u = t1;
            d = t3;
        }
        else
        {
            v1 = b-t1;
            v3 = -t3;
        }
        t1 = u-v1;
        t3 = d-v3;
        if (t1.Negative())
            t1 += b;
    }
    if (d==1)
        return u;
    else
        return Integer::Zero();   // no inverse
}
*/

int Jacobi(const Integer &aIn, const Integer &bIn)
{
    assert(bIn.IsOdd());

    Integer b = bIn, a = aIn%bIn;
    int result = 1;

    while (!!a)
    {
        unsigned i=0;
        while (a.GetBit(i)==0)
            i++;
        a>>=i;

        if (i%2==1 && (b%8==3 || b%8==5))
            result = -result;

        if (a%4==3 && b%4==3)
            result = -result;

        swap(a, b);
        a %= b;
    }

    return (b==1) ? result : 0;
}

Integer Lucas(const Integer &e, const Integer &pIn, const Integer &n)
{
    unsigned i = e.BitCount();
    if (i==0)
        return 2;

    MontgomeryRepresentation m(n);
    Integer p=m.ConvertIn(pIn%n), two=m.ConvertIn(2);
    Integer v=p, v1=m.Subtract(m.Square(p), two);

    i--;
    while (i--)
    {
        if (e.GetBit(i))
        {
            // v = (v*v1 - p) % m;
            v = m.Subtract(m.Multiply(v,v1), p);
            // v1 = (v1*v1 - 2) % m;
            v1 = m.Subtract(m.Square(v1), two);
        }
        else
        {
            // v1 = (v*v1 - p) % m;
            v1 = m.Subtract(m.Multiply(v,v1), p);
            // v = (v*v - 2) % m;
            v = m.Subtract(m.Square(v), two);
        }
    }
    return m.ConvertOut(v);
}

// This is Peter Montgomery's unpublished Lucas sequence evalutation algorithm.
// The total number of multiplies and squares used is less than the binary
// algorithm (see above).  Unfortunately I can't get it to run as fast as
// the binary algorithm because of the extra overhead.
/*
Integer Lucas(const Integer &n, const Integer &P, const Integer &modulus)
{
    if (!n)
        return 2;

#define f(A, B, C)  m.Subtract(m.Multiply(A, B), C)
#define X2(A) m.Subtract(m.Square(A), two)
#define X3(A) m.Multiply(A, m.Subtract(m.Square(A), three))

    MontgomeryRepresentation m(modulus);
    Integer two=m.ConvertIn(2), three=m.ConvertIn(3);
    Integer A=m.ConvertIn(P), B, C, p, d=n, e, r, t, T, U;

    while (d!=1)
    {
        p = d;
        unsigned int b = WORD_BITS * p.WordCount();
        Integer alpha = (Integer(5)<<(2*b-2)).SquareRoot() - Integer::Power2(b-1);
        r = (p*alpha)>>b;
        e = d-r;
        B = A;
        C = two;
        d = r;

        while (d!=e)
        {
            if (d<e)
            {
                swap(d, e);
                swap(A, B);
            }

            unsigned int dm2 = d[0], em2 = e[0];
            unsigned int dm3 = d%3, em3 = e%3;

//          if ((dm6+em6)%3 == 0 && d <= e + (e>>2))
            if ((dm3+em3==0 || dm3+em3==3) && (t = e, t >>= 2, t += e, d <= t))
            {
                // #1
//              t = (d+d-e)/3;
//              t = d; t += d; t -= e; t /= 3;
//              e = (e+e-d)/3;
//              e += e; e -= d; e /= 3;
//              d = t;

//              t = (d+e)/3
                t = d; t += e; t /= 3;
                e -= t;
                d -= t;

                T = f(A, B, C);
                U = f(T, A, B);
                B = f(T, B, A);
                A = U;
                continue;
            }

//          if (dm6 == em6 && d <= e + (e>>2))
            if (dm3 == em3 && dm2 == em2 && (t = e, t >>= 2, t += e, d <= t))
            {
                // #2
//              d = (d-e)>>1;
                d -= e; d >>= 1;
                B = f(A, B, C);
                A = X2(A);
                continue;
            }

//          if (d <= (e<<2))
            if (d <= (t = e, t <<= 2))
            {
                // #3
                d -= e;
                C = f(A, B, C);
                swap(B, C);
                continue;
            }

            if (dm2 == em2)
            {
                // #4
//              d = (d-e)>>1;
                d -= e; d >>= 1;
                B = f(A, B, C);
                A = X2(A);
                continue;
            }

            if (dm2 == 0)
            {
                // #5
                d >>= 1;
                C = f(A, C, B);
                A = X2(A);
                continue;
            }

            if (dm3 == 0)
            {
                // #6
//              d = d/3 - e;
                d /= 3; d -= e;
                T = X2(A);
                C = f(T, f(A, B, C), C);
                swap(B, C);
                A = f(T, A, A);
                continue;
            }

            if (dm3+em3==0 || dm3+em3==3)
            {
                // #7
//              d = (d-e-e)/3;
                d -= e; d -= e; d /= 3;
                T = f(A, B, C);
                B = f(T, A, B);
                A = X3(A);
                continue;
            }

            if (dm3 == em3)
            {
                // #8
//              d = (d-e)/3;
                d -= e; d /= 3;
                T = f(A, B, C);
                C = f(A, C, B);
                B = T;
                A = X3(A);
                continue;
            }

            assert(em2 == 0);
            // #9
            e >>= 1;
            C = f(C, B, A);
            B = X2(B);
        }

        A = f(A, B, C);
    }

#undef f
#undef X2
#undef X3

    return m.ConvertOut(A);
}
*/

Integer InverseLucas(const Integer &e, const Integer &m, const Integer &p, const Integer &q, const Integer &u)
{
    Integer d = (m*m-4);
    Integer p2 = p-Jacobi(d,p);
    Integer q2 = q-Jacobi(d,q);
    return CRT(Lucas(EuclideanMultiplicativeInverse(e,p2), m, p), p, Lucas(EuclideanMultiplicativeInverse(e,q2), m, q), q, u);
}

Integer InverseLucas(const Integer &e, const Integer &m, const Integer &p, const Integer &q)
{
    return InverseLucas(e, m, p, q, EuclideanMultiplicativeInverse(p, q));
}

unsigned int FactoringWorkFactor(unsigned int n)
{
    // extrapolated from the table in Odlyzko's "The Future of Integer Factorization"
    // updated to reflect the factoring of RSA-130
    if (n<5) return 0;
    else return (unsigned int)(2.4 * pow(n, 1.0/3.0) * pow(log(double(n)), 2.0/3.0) - 5);
}

unsigned int DiscreteLogWorkFactor(unsigned int n)
{
    // assuming discrete log takes about the same time as factoring
    if (n<5) return 0;
    else return (unsigned int)(2.4 * pow(n, 1.0/3.0) * pow(log(double(n)), 2.0/3.0) - 5);
}

NAMESPACE_END

// ********************************************************

USING_NAMESPACE(NumberTheory)

// generate a random prime p of the form 2*q+delta, where q is also prime
// warning: this is slow!
PrimeAndGenerator::PrimeAndGenerator(signed int delta, RandomNumberGenerator &rng, unsigned int pbits)
{
    Integer minQ = Integer::Power2(pbits-2);
    Integer maxQ = Integer::Power2(pbits-1) - 1;

    do
    {
        q.Randomize(rng, minQ, maxQ, Integer::ODD);
        p = 2*q+delta;

        RemainderTable rtQ(q);
        RemainderTable rtP(p);

        while (rtQ.HasZero() || rtP.HasZero() ||
               !FastProbablePrimeTest(q) || !FastProbablePrimeTest(p) ||
               !IsPrime(q) || !IsPrime(p))
        {
            rtQ.IncrementBy(2);
            rtP.IncrementBy(4);
            ++q; ++q;
            ++p; ++p; ++p; ++p;
        }
    } while (q>maxQ);

    if (delta == 1)
    {
        // find g such that g is a quadratic residue mod p, then g has order q
        // g=4 always works, but this way we get the smallest quadratic residue (other than 1)
        for (g=2; Jacobi(g, p) != 1; ++g);
    }
    else
    {
        assert(delta == -1);
        // find g such that g*g-4 is a quadratic non-residue, 
        // and such that g has order q
        for (g=3; ; ++g)
            if (Jacobi(g*g-4, p)==-1 && Lucas(q, g, p)==2)
                break;
    }
}

// generate a random prime p of the form 2*r*q+delta, where q is also prime
PrimeAndGenerator::PrimeAndGenerator(signed int delta, RandomNumberGenerator &rng, unsigned int pbits, unsigned int qbits)
{
    assert(pbits > qbits);

    Integer minQ = Integer::Power2(qbits-1);
    Integer maxQ = Integer::Power2(qbits) - 1;
    Integer minP = Integer::Power2(pbits-1);
    Integer maxP = Integer::Power2(pbits) - 1;

    while (1)
    {
        q.Randomize(rng, minQ, maxQ, Integer::PRIME);
        Integer q2 = 2*q;
        RemainderTable rtq2(q2);

        // generate a random number of the form 2*r*q+delta
        p.Randomize(rng, minP, maxP, Integer::ANY);
        p = p - p%q2 + q2 + delta;
        RemainderTable rtp(p);

        // now increment p by 2*q until p is prime
        while (p<=maxP)
        {
            if (rtp.HasZero() || !FastProbablePrimeTest(p) || !IsPrime(p))
            {
                p += q2;
                rtp.IncrementBy(rtq2);
            }
            else
            {
                // find a random g of order q
                if (delta==1)
                {
                    do
                    {
                        Integer h(rng, 2, p-2, Integer::ANY);
                        g = a_exp_b_mod_c(h, (p-1)/q, p);
                    } while (g <= 1);
                    assert(a_exp_b_mod_c(g, q, p)==1);
                }
                else
                {
                    assert(delta==-1);
                    do
                    {
                        Integer h(rng, 3, p-2, Integer::ANY);
                        if (Jacobi(h*h-4, p)==1)
                            continue;
                        g = Lucas((p+1)/q, h, p);
                    } while (g <= 2);
                    assert(Lucas(q, g, p) == 2);
                }
                return;
            }
        }
    }
}

// ********************************************************

ModExpPrecomputation::~ModExpPrecomputation() {}

ModExpPrecomputation::ModExpPrecomputation(const Integer &modulus, const Integer &base, unsigned int maxExpBits, unsigned int storage)
{
    Precompute(modulus, base, maxExpBits, storage);
}

ModExpPrecomputation::ModExpPrecomputation(const ModExpPrecomputation &mep)
    : mr(new MontgomeryRepresentation(*mep.mr)), 
      mg(new MR_MG(*mr)), 
      ep(new ExponentiationPrecomputation<MR_MG>(*mg, *mep.ep))
{
}

void ModExpPrecomputation::Precompute(const Integer &modulus, const Integer &base, unsigned int maxExpBits, unsigned int storage)
{
    if (!mr.get() || mr->GetModulus()!=modulus)
    {
        mr.reset(new MontgomeryRepresentation(modulus));
        mg.reset(new MR_MG(*mr));
        ep.reset(NULL);
    }

    if (!ep.get() || ep->storage < storage)
        ep.reset(new ExponentiationPrecomputation<MR_MG>(*mg, mr->ConvertIn(base), maxExpBits, storage));
}

void ModExpPrecomputation::Load(const Integer &modulus, BufferedTransformation &bt)
{
    if (!mr.get() || mr->GetModulus()!=modulus)
    {
        mr.reset(new MontgomeryRepresentation(modulus));
        mg.reset(new MR_MG(*mr));
    }

    ep.reset(new ExponentiationPrecomputation<MR_MG>(*mg));
    BERSequenceDecoder seq(bt);
    ep->storage = (unsigned int)(Integer(seq).ConvertToLong());
    ep->exponentBase.BERDecode(seq);
    ep->g.resize(ep->storage);
    for (unsigned i=0; i<ep->storage; i++)
        ep->g[i].BERDecode(seq);
}

void ModExpPrecomputation::Save(BufferedTransformation &bt) const
{
    assert(ep.get() != 0);
    DERSequenceEncoder seq(bt);
    Integer(ep->storage).DEREncode(seq);
    ep->exponentBase.DEREncode(seq);
    for (unsigned i=0; i<ep->storage; i++)
        ep->g[i].DEREncode(seq);
}

Integer ModExpPrecomputation::Exponentiate(const Integer &exponent) const
{
    assert(mr.get() && ep.get());
    return mr->ConvertOut(ep->Exponentiate(exponent));
}

Integer ModExpPrecomputation::CascadeExponentiate(const Integer &exponent, 
                            ModExpPrecomputation pc2, const Integer &exponent2) const
{
    assert(mr.get() && ep.get());
    return mr->ConvertOut(ep->CascadeExponentiate(exponent, *pc2.ep, exponent2));
}
