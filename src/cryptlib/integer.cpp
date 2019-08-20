// integer.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "integer.h"
#include "modarith.h"
#include "nbtheory.h"
#include "asn.h"
#include "words.h"

#include "iostream"
//USING_NAMESPACE(std)

#include "algebra.cpp"
#include "eprecomp.cpp"

#include <string.h>

#define MAKE_DWORD(lowWord, highWord) ((dword(highWord)<<WORD_BITS) | (lowWord))

union dword_union
{
    dword dw;
    struct _w {
#ifdef WORDS_BIGENDIAN
      word high;
      word low;
#else
      word low;
      word high;
#endif
    } w;
};

#if defined(_MSC_VER) && defined(_M_IX86) && (_M_IX86<=500)

// Add() and Subtract() are coded in Pentium assembly for a speed increase
// of about 10-20 percent for a RSA signature

static __declspec(naked) word __fastcall Add(word *C, const word *A, const word *B, unsigned int N)
{
    __asm
    {
        push ebp
        push ebx
        push esi
        push edi

        mov esi, [esp+24]
        mov ebx, [esp+20]

        sub ecx, edx
        xor eax, eax

        sub eax, esi
        lea ebx, [ebx+4*esi]

        sar eax, 1      // clears the carry flag
        jz  loopend

loopstart:
        mov    esi,[edx]
        mov    ebp,[edx+4]

        mov    edi,[ebx+8*eax]
        lea    edx,[edx+8]

        adc    esi,edi
        mov    edi,[ebx+8*eax+4]

        adc    ebp,edi
        inc    eax

        mov    [edx+ecx-8],esi
        mov    [edx+ecx-4],ebp

        jnz    loopstart

loopend:
        adc eax, 0
        pop edi
        pop esi
        pop ebx
        pop ebp
        ret 8
    }
}

static __declspec(naked) word __fastcall Subtract(word *C, const word *A, const word *B, unsigned int N)
{
    __asm
    {
        push ebp
        push ebx
        push esi
        push edi

        mov esi, [esp+24]
        mov ebx, [esp+20]

        sub ecx, edx
        xor eax, eax

        sub eax, esi
        lea ebx, [ebx+4*esi]

        sar eax, 1      // clears the carry flag
        jz  loopend

loopstart:
        mov    esi,[edx]
        mov    ebp,[edx+4]

        mov    edi,[ebx+8*eax]
        lea    edx,[edx+8]

        sbb    esi,edi
        mov    edi,[ebx+8*eax+4]

        sbb    ebp,edi
        inc    eax

        mov    [edx+ecx-8],esi
        mov    [edx+ecx-4],ebp

        jnz    loopstart

loopend:
        adc eax, 0
        pop edi
        pop esi
        pop ebx
        pop ebp
        ret 8
    }
}

#else   // defined(_MSC_VER) && defined(_M_IX86) && (_M_IX86<=500)

static word Add(word *C, const word *A, const word *B, unsigned int N)
{
    assert (N%2 == 0);

    word carry=0;
    for (unsigned i = 0; i < N; i+=2)
    {
        dword_union u;
        u.dw = (dword) carry + A[i] + B[i];
        C[i] = u.w.low;
        u.dw = (dword) u.w.high + A[i+1] + B[i+1];
        C[i+1] = u.w.low;
        carry = u.w.high;
    }
    return carry;
}

static word Subtract(word *C, const word *A, const word *B, unsigned int N)
{
    assert (N%2 == 0);

    word borrow=0;
    for (unsigned i = 0; i < N; i+=2)
    {
        dword_union u;
                u.dw = (dword) A[i] - B[i] - borrow;
        C[i] = u.w.low;
        u.dw = (dword) A[i+1] - B[i+1] - (word)(0-u.w.high);
        C[i+1] = u.w.low;
        borrow = 0-u.w.high;
    }
    return borrow;
}

#endif  // defined(_MSC_VER) && defined(_M_IX86) && (_M_IX86<=500)

static int Compare(const word *A, const word *B, unsigned int N)
{
    while (N--)
        if (A[N] > B[N])
            return 1;
        else if (A[N] < B[N])
            return -1;

    return 0;
}

static word Increment(word *A, unsigned int N, word B=1)
{
    assert(N);
    word t = A[0];
    A[0] = t+B;
    if (A[0] >= t)
        return 0;
    for (unsigned i=1; i<N; i++)
        if (++A[i])
            return 0;
    return 1;
}

static word Decrement(word *A, unsigned int N, word B=1)
{
    assert(N);
    word t = A[0];
    A[0] = t-B;
    if (A[0] <= t)
        return 0;
    for (unsigned i=1; i<N; i++)
        if (A[i]--)
            return 0;
    return 1;
}

static void TwosComplement(word *A, unsigned int N)
{
    Decrement(A, N);
    for (unsigned i=0; i<N; i++)
        A[i] = ~A[i];
}

static word LinearMultiply(word *C, const word *A, word B, unsigned int N)
{
    word carry=0;
    for(unsigned i=0; i<N; i++)
    {
        dword_union p;
                p.dw = (dword)A[i] * B + carry;
        C[i] = p.w.low;
        carry = p.w.high;
    }
    return carry;
}

static void AtomicMultiply(word *C, word A0, word A1, word B0, word B1)
{
    word s;
    dword_union d;

    if (A1 >= A0)
        if (B0 >= B1)
        {
            s = 0;
            d.dw = (dword)(A1-A0)*(B0-B1);
        }
        else
        {
            s = (A1-A0);
            d.dw = (dword)s*(word)(B0-B1);
        }
    else
        if (B0 > B1)
        {
            s = (B0-B1);
            d.dw = (word)(A1-A0)*(dword)s;
        }
        else
        {
            s = 0;
            d.dw = (dword)(A0-A1)*(B1-B0);
        }

    dword_union A0B0;
    A0B0.dw = (dword)A0*B0;
    C[0] = A0B0.w.low;

    dword_union A1B1;
    A1B1.dw = (dword)A1*B1;
    dword_union t;
    t.dw = (dword)A0B0.w.high + A0B0.w.low + d.w.low + A1B1.w.low;
    C[1] = t.w.low;

    t.dw = A1B1.dw + t.w.high + A0B0.w.high + d.w.high + A1B1.w.high - s;
    C[2] = t.w.low;
    C[3] = t.w.high;
}

static word AtomicMultiplyAdd(word *C, word A0, word A1, word B0, word B1)
{
    word s;
    dword_union d;

    if (A1 >= A0)
        if (B0 >= B1)
        {
            s = 0;
            d.dw = (dword)(A1-A0)*(B0-B1);
        }
        else
        {
            s = (A1-A0);
            d.dw = (dword)s*(word)(B0-B1);
        }
    else
        if (B0 > B1)
        {
            s = (B0-B1);
            d.dw = (word)(A1-A0)*(dword)s;
        }
        else
        {
            s = 0;
            d.dw = (dword)(A0-A1)*(B1-B0);
        }

    dword_union A0B0;
    A0B0.dw = (dword)A0*B0;
    dword_union t;
    t.dw = A0B0.dw + C[0];
    C[0] = t.w.low;

    dword_union A1B1;
        A1B1.dw = (dword)A1*B1;
    t.dw = (dword) t.w.high + A0B0.w.low + d.w.low + A1B1.w.low + C[1];
    C[1] = t.w.low;

    t.dw = (dword) t.w.high + A1B1.w.low + A0B0.w.high + d.w.high + A1B1.w.high - s + C[2];
    C[2] = t.w.low;

    t.dw = (dword) t.w.high + A1B1.w.high + C[3];
    C[3] = t.w.low;
    return t.w.high;
}

static inline void AtomicSquare(word *C, word A, word B)
{
#ifdef _MSC_VER
    // VC50 workaround
    AtomicMultiply(C, A, B, A, B);
#else
    dword_union t1;
    t1.dw = (dword) A*A;
    C[0] = t1.w.low;

    dword_union t2;
        t2.dw = (dword) A*B;
    t1.dw = (dword) t1.w.high + t2.w.low + t2.w.low;
    C[1] = t1.w.low;

    t1.dw = (dword) B*B + t1.w.high + t2.w.high + t2.w.high;
    C[2] = t1.w.low;
    C[3] = t1.w.high;
#endif
}

static inline void AtomicMultiplyBottom(word *C, word A0, word A1, word B0, word B1)
{
    dword_union t;
    t.dw = (dword)A0*B0;
    C[0] = t.w.low;
    C[1] = t.w.high + A0*B1 + A1*B0;
}

static inline void AtomicMultiplyBottomAdd(word *C, word A0, word A1, word B0, word B1)
{
    dword_union t;
    t.dw = (dword)A0*B0 + C[0];
    C[0] = t.w.low;
    C[1] += t.w.high + A0*B1 + A1*B0;
}

static void CombaMultiply(word *R, const word *A, const word *B)
{
    dword_union p;
    word c=0, d=0, e=0;

#define MulAcc(x, y)                                \
    p.dw = (dword)A[x] * B[y] + c;                      \
    c = p.w.low;                              \
    p.dw = (dword)d + p.w.high;                   \
    d = p.w.low;                              \
    e += p.w.high;

#define SaveMulAcc(s, x, y)                         \
    R[s] = c;                                       \
    p.dw = (dword)A[x] * B[y] + d;                      \
    c = p.w.low;                              \
    p.dw = (dword)e + p.w.high;                   \
    d = p.w.low;                              \
    e = p.w.high;

    p.dw = (dword)A[0] * B[0];
    R[0] = p.w.low;
    c = p.w.high;
    d = e = 0;

    MulAcc(0, 1);
    MulAcc(1, 0);

    SaveMulAcc(1, 2, 0);
    MulAcc(1, 1);
    MulAcc(0, 2);

    SaveMulAcc(2, 0, 3);
    MulAcc(1, 2);
    MulAcc(2, 1);
    MulAcc(3, 0);

    SaveMulAcc(3, 3, 1);
    MulAcc(2, 2);
    MulAcc(1, 3);

    SaveMulAcc(4, 2, 3);
    MulAcc(3, 2);

    R[5] = c;
    p.dw = (dword)A[3] * B[3] + d;
    R[6] = p.w.low;
    R[7] = e + p.w.high;

#undef MulAcc
#undef SaveMulAcc
}

static void AtomicInverseModPower2(word *C, word A0, word A1)
{
    assert(A0%2==1);

    const dword A = MAKE_DWORD(A0, A1);
    dword_union R;
    R.dw = A0%8;

    for (unsigned i=3; i<2*WORD_BITS; i*=2)
        R.dw = R.dw*(2-R.dw*A);

    assert(R.dw*A==1);

    C[0] = R.w.low;
    C[1] = R.w.high;
}

// ********************************************************

#define A0      A
#define A1      (A+N2)
#define B0      B
#define B1      (B+N2)

#define T0      T
#define T1      (T+N2)
#define T2      (T+N)
#define T3      (T+N+N2)

#define R0      R
#define R1      (R+N2)
#define R2      (R+N)
#define R3      (R+N+N2)

// R[2*N] - result = A*B
// T[2*N] - temporary work space
// A[N] --- multiplier
// B[N] --- multiplicant

void RecursiveMultiply(word *R, word *T, const word *A, const word *B, unsigned int N)
{
    assert(N>=2 && N%2==0);

    if (N==2)
        AtomicMultiply(R, A[0], A[1], B[0], B[1]);
    else if (N==4)
        CombaMultiply(R, A, B);
    else
    {
        const unsigned int N2 = N/2;
        int carry;

        int aComp = Compare(A0, A1, N2);
        int bComp = Compare(B0, B1, N2);

        switch (2*aComp + aComp + bComp)
        {
        case -4:
            Subtract(R0, A1, A0, N2);
            Subtract(R1, B0, B1, N2);
            RecursiveMultiply(T0, T2, R0, R1, N2);
            Subtract(T1, T1, R0, N2);
            carry = -1;
            break;
        case -2:
            Subtract(R0, A1, A0, N2);
            Subtract(R1, B0, B1, N2);
            RecursiveMultiply(T0, T2, R0, R1, N2);
            carry = 0;
            break;
        case 2:
            Subtract(R0, A0, A1, N2);
            Subtract(R1, B1, B0, N2);
            RecursiveMultiply(T0, T2, R0, R1, N2);
            carry = 0;
            break;
        case 4:
            Subtract(R0, A1, A0, N2);
            Subtract(R1, B0, B1, N2);
            RecursiveMultiply(T0, T2, R0, R1, N2);
            Subtract(T1, T1, R1, N2);
            carry = -1;
            break;
        default:
            SetWords(T0, 0, N);
            carry = 0;
        }

        RecursiveMultiply(R0, T2, A0, B0, N2);
        RecursiveMultiply(R2, T2, A1, B1, N2);

        // now T[01] holds (A1-A0)*(B0-B1), R[01] holds A0*B0, R[23] holds A1*B1

        carry += Add(T0, T0, R0, N);
        carry += Add(T0, T0, R2, N);
        carry += Add(R1, R1, T0, N);

        assert (carry >= 0 && carry <= 2);
        Increment(R3, N2, carry);
    }
}

// R[2*N] - result = A*A
// T[2*N] - temporary work space
// A[N] --- number to be squared

void RecursiveSquare(word *R, word *T, const word *A, unsigned int N)
{
    assert(N && N%2==0);

    if (N==2)
        AtomicSquare(R, A[0], A[1]);
    else if (N==4)
    {
        AtomicSquare(R, A[0], A[1]);
        AtomicSquare(R+4, A[2], A[3]);
        AtomicMultiply(T, A[0], A[1], A[2], A[3]);
        word carry = Add(R+2, R+2, T, 4);
        carry += Add(R+2, R+2, T, 4);
        Increment(R+6, 2, carry);
    }
    else
    {
        const unsigned int N2 = N/2;

        RecursiveSquare(R0, T2, A0, N2);
        RecursiveSquare(R2, T2, A1, N2);
        RecursiveMultiply(T0, T2, A0, A1, N2);

        word carry = Add(R1, R1, T0, N);
        carry += Add(R1, R1, T0, N);
        Increment(R3, N2, carry);
    }
}

// R[N] - bottom half of A*B
// T[N] - temporary work space
// A[N] - multiplier
// B[N] - multiplicant

void RecursiveMultiplyBottom(word *R, word *T, const word *A, const word *B, unsigned int N)
{
    assert(N>=2 && N%2==0);

    if (N==2)
        AtomicMultiplyBottom(R, A[0], A[1], B[0], B[1]);
    else if (N==4)
    {
        AtomicMultiply(R, A[0], A[1], B[0], B[1]);
        AtomicMultiplyBottomAdd(R+2, A[0], A[1], B[2], B[3]);
        AtomicMultiplyBottomAdd(R+2, A[2], A[3], B[0], B[1]);
    }
    else
    {
        const unsigned int N2 = N/2;

        RecursiveMultiply(R, T, A0, B0, N2);
        RecursiveMultiplyBottom(T0, T1, A1, B0, N2);
        Add(R1, R1, T0, N2);
        RecursiveMultiplyBottom(T0, T1, A0, B1, N2);
        Add(R1, R1, T0, N2);
    }
}

// R[N] --- upper half of A*B
// T[2*N] - temporary work space
// L[N] --- lower half of A*B
// A[N] --- multiplier
// B[N] --- multiplicant

void RecursiveMultiplyTop(word *R, word *T, const word *L, const word *A, const word *B, unsigned int N)
{
    assert(N>=2 && N%2==0);

    if (N==2)
    {
        AtomicMultiply(T, A[0], A[1], B[0], B[1]);
        R[0] = T[2];
        R[1] = T[3];
    }
    else
    {
        const unsigned int N2 = N/2;
        int carry;

        int aComp = Compare(A0, A1, N2);
        int bComp = Compare(B0, B1, N2);

        switch (2*aComp + aComp + bComp)
        {
        case -4:
            Subtract(R0, A1, A0, N2);
            Subtract(R1, B0, B1, N2);
            RecursiveMultiply(T0, T2, R0, R1, N2);
            Subtract(T1, T1, R0, N2);
            carry = -1;
            break;
        case -2:
            Subtract(R0, A1, A0, N2);
            Subtract(R1, B0, B1, N2);
            RecursiveMultiply(T0, T2, R0, R1, N2);
            carry = 0;
            break;
        case 2:
            Subtract(R0, A0, A1, N2);
            Subtract(R1, B1, B0, N2);
            RecursiveMultiply(T0, T2, R0, R1, N2);
            carry = 0;
            break;
        case 4:
            Subtract(R0, A1, A0, N2);
            Subtract(R1, B0, B1, N2);
            RecursiveMultiply(T0, T2, R0, R1, N2);
            Subtract(T1, T1, R1, N2);
            carry = -1;
            break;
        default:
            SetWords(T0, 0, N);
            carry = 0;
        }

        RecursiveMultiply(T2, R0, A1, B1, N2);

        // now T[01] holds (A1-A0)*(B0-B1), T[23] holds A1*B1

        CopyWords(R0, L+N2, N2);
        word c2 = Subtract(R0, R0, L, N2);
        c2 += Subtract(R0, R0, T0, N2);
        word t = (Compare(R0, T2, N2) == -1);

        carry += t;
        carry += Increment(R0, N2, c2+t);
        carry += Add(R0, R0, T1, N2);
        carry += Add(R0, R0, T3, N2);

        CopyWords(R1, T3, N2);
        assert (carry >= 0 && carry <= 2);
        Increment(R1, N2, carry);
    }
}

// R[NA+NB] - result = A*B
// T[NA+NB] - temporary work space
// A[NA] ---- multiplier
// B[NB] ---- multiplicant

void AsymmetricMultiply(word *R, word *T, const word *A, unsigned int NA, const word *B, unsigned int NB)
{
    if (NA == NB)
    {
        if (A == B)
            RecursiveSquare(R, T, A, NA);
        else
            RecursiveMultiply(R, T, A, B, NA);

        return;
    }

    if (NA > NB)
    {
        swap(A, B);
        swap(NA, NB);
    }

    assert(NB % NA == 0);
    assert((NB/NA)%2 == 0);     // NB is an even multiple of NA

    if (NA==2 && !A[1])
    {
        switch (A[0])
        {
        case 0:
            SetWords(R, 0, NB+2);
            return;
        case 1:
            CopyWords(R, B, NB);
            R[NB] = R[NB+1] = 0;
            return;
        default:
            R[NB] = LinearMultiply(R, B, A[0], NB);
            R[NB+1] = 0;
            return;
        }
    }

    RecursiveMultiply(R, T, A, B, NA);
    CopyWords(T+2*NA, R+NA, NA);

    unsigned i;

    for (i=2*NA; i<NB; i+=2*NA)
        RecursiveMultiply(T+NA+i, T, A, B+i, NA);
    for (i=NA; i<NB; i+=2*NA)
        RecursiveMultiply(R+i, T, A, B+i, NA);

    if (Add(R+NA, R+NA, T+2*NA, NB-NA))
        Increment(R+NB, NA);
}

// R[N] ----- result = A inverse mod 2**(WORD_BITS*N)
// T[3*N/2] - temporary work space
// A[N] ----- an odd number as input

void RecursiveInverseModPower2(word *R, word *T, const word *A, unsigned int N)
{
    if (N==2)
        AtomicInverseModPower2(R, A[0], A[1]);
    else
    {
        const unsigned int N2 = N/2;
        RecursiveInverseModPower2(R0, T0, A0, N2);
        T0[0] = 1;
        SetWords(T0+1, 0, N2-1);
        RecursiveMultiplyTop(R1, T1, T0, R0, A0, N2);
        RecursiveMultiplyBottom(T0, T1, R0, A1, N2);
        Add(T0, R1, T0, N2);
        TwosComplement(T0, N2);
        RecursiveMultiplyBottom(R1, T1, R0, T0, N2);
    }
}

// R[N] --- result = X/(2**(WORD_BITS*N)) mod M
// T[3*N] - temporary work space
// X[2*N] - number to be reduced
// M[N] --- modulus
// U[N] --- multiplicative inverse of M mod 2**(WORD_BITS*N)

void MontgomeryReduce(word *R, word *T, const word *X, const word *M, const word *U, unsigned int N)
{
    RecursiveMultiplyBottom(R, T, X, U, N);
    RecursiveMultiplyTop(T, T+N, X, R, M, N);
    if (Subtract(R, X+N, T, N))
    {
        word carry = Add(R, R, M, N);
        assert(carry);
    }
}

// R[N] --- result = X/(2**(WORD_BITS*N/2)) mod M
// T[2*N] - temporary work space
// X[2*N] - number to be reduced
// M[N] --- modulus
// U[N/2] - multiplicative inverse of M mod 2**(WORD_BITS*N/2)
// V[N] --- 2**(WORD_BITS*3*N/2) mod M

void HalfMontgomeryReduce(word *R, word *T, const word *X, const word *M, const word *U, const word *V, unsigned int N)
{
    assert(N%2==0 && N>=4);

#define M0      M
#define M1      (M+N2)
#define V0      V
#define V1      (V+N2)

#define X0      X
#define X1      (X+N2)
#define X2      (X+N)
#define X3      (X+N+N2)

    const unsigned int N2 = N/2;
    RecursiveMultiply(T0, T2, V0, X3, N2);
    int c2 = Add(T0, T0, X0, N);
    RecursiveMultiplyBottom(T3, T2, T0, U, N2);
    RecursiveMultiplyTop(T2, R, T0, T3, M0, N2);
    c2 -= Subtract(T2, T1, T2, N2);
    RecursiveMultiply(T0, R, T3, M1, N2);
    c2 -= Subtract(T0, T2, T0, N2);
    int c3 = -(int)Subtract(T1, X2, T1, N2);
    RecursiveMultiply(R0, T2, V1, X3, N2);
    c3 += Add(R, R, T, N);

    if (c2>0)
        c3 += Increment(R1, N2);
    else if (c2<0)
        c3 -= Decrement(R1, N2, -c2);

    assert(c3>=-1 && c3<=1);
    if (c3>0)
        Subtract(R, R, M, N);
    else if (c3<0)
        Add(R, R, M, N);

#undef M0
#undef M1
#undef V0
#undef V1

#undef X0
#undef X1
#undef X2
#undef X3
}

#undef A0
#undef A1
#undef B0
#undef B1

#undef T0
#undef T1
#undef T2
#undef T3

#undef R0
#undef R1
#undef R2
#undef R3

// do a 3 word by 2 word divide, returns quotient and leaves remainder in A
static word SubatomicDivide(word *A, word B0, word B1)
{
    // assert {A[2],A[1]} < {B1,B0}, so quotient can fit in a word
    assert(A[2] < B1 || (A[2]==B1 && A[1] < B0));

    dword_union p, u;
    word Q;

    // estimate the quotient: do a 2 word by 1 word divide
    if (B1+1 == 0)
        Q = A[2];
    else
        Q = word(MAKE_DWORD(A[1], A[2]) / (B1+1));

    // now subtract Q*B from A
    p.dw = (dword) B0*Q;
    u.dw = (dword) A[0] - p.w.low;
    A[0] = u.w.low;
    u.dw = (dword) A[1] - p.w.high - (word)(0-u.w.high) - (dword)B1*Q;
    A[1] = u.w.low;
    A[2] += u.w.high;

    // Q <= actual quotient, so fix it
    while (A[2] || A[1] > B1 || (A[1]==B1 && A[0]>=B0))
    {
        u.dw = (dword) A[0] - B0;
        A[0] = u.w.low;
        u.dw = (dword) A[1] - B1 - (word)(0-u.w.high);
        A[1] = u.w.low;
        A[2] += u.w.high;
        Q++;
        assert(Q);  // shouldn't overflow
    }

    return Q;
}

// do a 4 word by 2 word divide, returns 2 word quotient in Q0 and Q1
static inline void AtomicDivide(word &Q0, word &Q1, const word *A, word B0, word B1)
{
    if (!B0 && !B1) // if divisor is 0, we assume divisor==2**(2*WORD_BITS)
    {
        Q0 = A[2];
        Q1 = A[3];
    }
    else
    {
        word T[4];
        T[0] = A[0]; T[1] = A[1]; T[2] = A[2]; T[3] = A[3];
        Q1 = SubatomicDivide(T+1, B0, B1);
        Q0 = SubatomicDivide(T, B0, B1);

#ifdef DEBUG
        // multiply quotient and divisor and add remainder, make sure it equals dividend
        assert(!T[2] && !T[3] && (T[1] < B1 || (T[1]==B1 && T[0]<B0)));
        word P[4];
        AtomicMultiply(P, Q0, Q1, B0, B1);
        Add(P, P, T, 4);
        assert(memcmp(P, A, 4*WORD_SIZE)==0);
#endif
    }
}

// for use by Divide(), corrects the underestimated quotient {Q1,Q0}
static void CorrectQuotientEstimate(word *R, word *T, word &Q0, word &Q1, const word *B, unsigned int N)
{
    assert(N && N%2==0);

    if (Q1)
    {
        T[N] = T[N+1] = 0;
        unsigned i;
        for (i=0; i<N; i+=4)
            AtomicMultiply(T+i, Q0, Q1, B[i], B[i+1]);
        for (i=2; i<N; i+=4)
            if (AtomicMultiplyAdd(T+i, Q0, Q1, B[i], B[i+1]))
                T[i+5] += (++T[i+4]==0);
    }
    else
    {
        T[N] = LinearMultiply(T, B, Q0, N);
        T[N+1] = 0;
    }

    word borrow = Subtract(R, R, T, N+2);
    assert(!borrow && !R[N+1]);

    while (R[N] || Compare(R, B, N) >= 0)
    {
        R[N] -= Subtract(R, R, B, N);
        Q1 += (++Q0==0);
        assert(Q0 || Q1); // no overflow
    }
}

// R[NB] -------- remainder = A%B
// Q[NA-NB+2] --- quotient  = A/B
// T[NA+2*NB+4] - temp work space
// A[NA] -------- dividend
// B[NB] -------- divisor

void Divide(word *R, word *Q, word *T, const word *A, unsigned int NA, const word *B, unsigned int NB)
{
    assert(NA && NB && NA%2==0 && NB%2==0);
    assert(B[NB-1] || B[NB-2]);
    assert(NB <= NA);

    // set up temporary work space
    word *const TA=T;
    word *const TB=T+NA+2;
    word *const TP=T+NA+2+NB;

    // copy B into TB and normalize it so that TB has highest bit set to 1
    unsigned shiftWords = (B[NB-1]==0);
    TB[0] = TB[NB-1] = 0;
    CopyWords(TB+shiftWords, B, NB-shiftWords);
    unsigned shiftBits = WORD_BITS - BitPrecision(TB[NB-1]);
    assert(shiftBits < WORD_BITS);
    ShiftWordsLeftByBits(TB, NB, shiftBits);

    // copy A into TA and normalize it
    TA[0] = TA[NA] = TA[NA+1] = 0;
    CopyWords(TA+shiftWords, A, NA);
    ShiftWordsLeftByBits(TA, NA+2, shiftBits);

    if (TA[NA+1]==0 && TA[NA] <= 1)
    {
        Q[NA-NB+1] = Q[NA-NB] = 0;
        while (TA[NA] || Compare(TA+NA-NB, TB, NB) >= 0)
        {
            TA[NA] -= Subtract(TA+NA-NB, TA+NA-NB, TB, NB);
            ++Q[NA-NB];
        }
    }
    else
    {
        NA+=2;
        assert(Compare(TA+NA-NB, TB, NB) < 0);
    }

    word B0 = TB[NB-2] + 1;
    word B1 = TB[NB-1] + (B0==0);

    // start reducing TA mod TB, 2 words at a time
    for (unsigned i=NA-2; i>=NB; i-=2)
    {
        AtomicDivide(Q[i-NB], Q[i-NB+1], TA+i-2, B0, B1);
        CorrectQuotientEstimate(TA+i-NB, TP, Q[i-NB], Q[i-NB+1], TB, NB);
    }

    // copy TA into R, and denormalize it
    CopyWords(R, TA+shiftWords, NB);
    ShiftWordsRightByBits(R, NB, shiftBits);
}

static inline unsigned int EvenWordCount(const word *X, unsigned int N)
{
    while (N && X[N-2]==0 && X[N-1]==0)
        N-=2;
    return N;
}

// return k
// R[N] --- result = A^(-1) * 2^k mod M
// T[4*N] - temporary work space
// A[NA] -- number to take inverse of
// M[N] --- modulus

unsigned int AlmostInverse(word *R, word *T, const word *A, unsigned int NA, const word *M, unsigned int N)
{
    assert(NA<=N && N && N%2==0);

    word *b = T;
    word *c = T+N;
    word *f = T+2*N;
    word *g = T+3*N;
    unsigned int bcLen=2, fgLen=EvenWordCount(M, N);
    unsigned int k=0, s=0;

    SetWords(T, 0, 3*N);
    b[0]=1;
    CopyWords(f, A, NA);
    CopyWords(g, M, N);

    while (1)
    {
        word t=f[0];
        while (!t)
        {
            if (EvenWordCount(f, fgLen)==0)
            {
                SetWords(R, 0, N);
                return 0;
            }

            ShiftWordsRightByWords(f, fgLen, 1);
            if (c[bcLen-1]) bcLen+=2;
            assert(bcLen <= N);
            ShiftWordsLeftByWords(c, bcLen, 1);
            k+=WORD_BITS;
            t=f[0];
        }

        unsigned int i=0;
        while (t%2 == 0)
        {
            t>>=1;
            i++;
        }
        k+=i;

        if (t==1 && f[1]==0 && EvenWordCount(f, fgLen)==2)
        {
            if (s%2==0)
                CopyWords(R, b, N);
            else
                Subtract(R, M, b, N);
            return k;
        }

        ShiftWordsRightByBits(f, fgLen, i);
        t=ShiftWordsLeftByBits(c, bcLen, i);
        if (t)
        {
            c[bcLen] = t;
            bcLen+=2;
            assert(bcLen <= N);
        }

        if (f[fgLen-2]==0 && g[fgLen-2]==0 && f[fgLen-1]==0 && g[fgLen-1]==0)
            fgLen-=2;

        if (Compare(f, g, fgLen)==-1)
        {
            swap(f, g);
            swap(b, c);
            s++;
        }

        Subtract(f, f, g, fgLen);

        if (Add(b, b, c, bcLen))
        {
            b[bcLen] = 1;
            bcLen+=2;
            assert(bcLen <= N);
        }
    }
}

// R[N] - result = A/(2^k) mod M
// A[N] - input
// M[N] - modulus

void DivideByPower2Mod(word *R, const word *A, unsigned int k, const word *M, unsigned int N)
{
    CopyWords(R, A, N);

    while (k--)
    {
        if (R[0]%2==0)
            ShiftWordsRightByBits(R, N, 1);
        else
        {
            word carry = Add(R, R, M, N);
            ShiftWordsRightByBits(R, N, 1);
            R[N-1] += carry<<(WORD_BITS-1);
        }
    }
}

// R[N] - result = A*(2^k) mod M
// A[N] - input
// M[N] - modulus

void MultiplyByPower2Mod(word *R, const word *A, unsigned int k, const word *M, unsigned int N)
{
    CopyWords(R, A, N);

    while (k--)
        if (ShiftWordsLeftByBits(R, N, 1) || Compare(R, M, N)>=0)
            Subtract(R, R, M, N);
}

// ******************************************************************

static const unsigned int RoundupSizeTable[] = {2, 2, 2, 4, 4, 8, 8, 8, 8};

inline unsigned int RoundupSize(unsigned int n)
{
    if (n<=8)
        return RoundupSizeTable[n];
    else if (n<=16)
        return 16;
    else if (n<=32)
        return 32;
    else if (n<=64)
        return 64;
    else return 1U << BitPrecision(n-1);
}

Integer::Integer()
    : reg(2), sign(POSITIVE)
{
    reg[(unsigned int)0] = reg[(unsigned int)1] = 0;
}

Integer::Integer(const Integer& t)
    : reg(RoundupSize(t.WordCount())), sign(t.sign)
{
    CopyWords(reg, t.reg, reg.size);
}

Integer::Integer(long value)
    : reg(2)
{
    if (value >= 0)
        sign = POSITIVE;
    else
    {
        sign = NEGATIVE;
        value = -value;
    }
    reg[(unsigned int)0] = word(value);
#ifdef __GNUC__
    reg[(unsigned int)1] = 0;
#else
    reg[(unsigned int)1] = sizeof(value)>WORD_SIZE ? word(value>>WORD_BITS) : 0;
#endif
}

long Integer::ConvertToLong() const
{
    unsigned long value = reg[(unsigned int)0];
#ifndef __GNUC__
    value += (sizeof(value) > WORD_SIZE) ? (((unsigned long)(reg[(unsigned int)1])) << WORD_BITS) : 0;
#endif
    value = Crop(value, 8*sizeof(value)-1);
    return sign==POSITIVE ? value : -long(value);
}

Integer::Integer(const uint8_t *encodedInteger, unsigned int byteCount, Signedness s)
{
    Decode(encodedInteger, byteCount, s);
}

Integer::Integer(const uint8_t *BEREncodedInteger)
{
    BERDecode(BEREncodedInteger);
}

Integer::Integer(BufferedTransformation &bt)
{
    BERDecode(bt);
}

Integer::Integer(RandomNumberGenerator &rng, unsigned int bitcount)
{
    Randomize(rng, bitcount);
}

Integer::Integer(RandomNumberGenerator &rng, const Integer &min, const Integer &max, RandomNumberType rnType)
{
    Randomize(rng, min, max, rnType);
}

Integer Integer::Power2(unsigned int e)
{
    Integer r((word)0, bitsToWords(e+1)); 
    r.SetBit(e); 
    return r;
}

const Integer &Integer::Zero()
{
    static const Integer zero;
    return zero;
}

const Integer &Integer::One()
{
    static const Integer one(1,2);
    return one;
}

bool Integer::operator!() const
{
    return IsNegative() ? false : (reg[(unsigned int)0]==0 && WordCount()==0);
}

Integer& Integer::operator=(const Integer& t)
{
    if (this != &t)
    {
        reg.New(RoundupSize(t.WordCount()));
        CopyWords(reg, t.reg, reg.size);
        sign = t.sign;
    }
    return *this;
}

bool Integer::GetBit(unsigned int n) const
{
    if (n/WORD_BITS >= reg.size)
        return 0;
    else
        return bool((reg[n/WORD_BITS] >> (n % WORD_BITS)) & 1);
}

void Integer::SetBit(unsigned int n, bool value)
{
    if (value)
    {
        reg.CleanGrow(RoundupSize(bitsToWords(n+1)));
        reg[n/WORD_BITS] |= (word(1) << (n%WORD_BITS));
    }
    else
    {
        if (n/WORD_BITS < reg.size)
            reg[n/WORD_BITS] &= ~(word(1) << (n%WORD_BITS));
    }
}

uint8_t Integer::GetByte(unsigned int n) const
{
    if (n/WORD_SIZE >= reg.size)
        return 0;
    else
        return uint8_t(reg[n/WORD_SIZE] >> ((n%WORD_SIZE)*8));
}

void Integer::SetByte(unsigned int n, uint8_t value)
{
    reg.CleanGrow(RoundupSize(bytesToWords(n+1)));
    reg[n/WORD_SIZE] &= ~(word(0xff) << 8*(n%WORD_SIZE));
    reg[n/WORD_SIZE] |= (word(value) << 8*(n%WORD_SIZE));
}

Integer Integer::operator-() const
{
    Integer result(*this); 
    result.Negate(); 
    return result;
}

Integer Integer::AbsoluteValue() const
{
    Integer result(*this);
    result.sign = POSITIVE;
    return result;
}

void swap(Integer &a, Integer &b)
{
    swap(a.reg, b.reg);
    swap(a.sign, b.sign);
}

Integer::Integer(word value, unsigned int length)
    : reg(RoundupSize(length)), sign(POSITIVE)
{
    reg[(unsigned int)0] = value;
    SetWords(reg+(unsigned int)1, 0, reg.size-(unsigned int)1);
}


Integer::Integer(const char *str)
    : reg(2), sign(POSITIVE)
{
    word radix;
    unsigned length = strlen(str);

    SetWords(reg, 0, 2);

    if (length == 0)
        return;

    switch (str[length-1])
    {
    case 'h':
    case 'H':
        radix=16;
        break;
    case 'o':
    case 'O':
        radix=8;
        break;
    case 'b':
    case 'B':
        radix=2;
        break;
    default:
        radix=10;
    }

    for (unsigned i=0; i<length; i++)
    {
        word digit;

        if (str[i] >= '0' && str[i] <= '9')
            digit = str[i] - '0';
        else if (str[i] >= 'A' && str[i] <= 'F')
            digit = str[i] - 'A' + 10;
        else if (str[i] >= 'a' && str[i] <= 'f')
            digit = str[i] - 'a' + 10;
        else
            digit = radix;

        if (digit < radix)
        {
            *this *= radix;
            *this += digit;
        }
    }

    if (str[0] == '-')
        Negate();
}

unsigned int Integer::WordCount() const
{
    return CountWords(reg, reg.size);
}

unsigned int Integer::ByteCount() const
{
    unsigned wordCount = WordCount();
    if (wordCount)
        return (wordCount-1)*WORD_SIZE + BytePrecision(reg[wordCount-1]);
    else
        return 0;
}

unsigned int Integer::BitCount() const
{
    unsigned wordCount = WordCount();
    if (wordCount)
        return (wordCount-1)*WORD_BITS + BitPrecision(reg[wordCount-1]);
    else
        return 0;
}

void Integer::Decode(const uint8_t *input, unsigned int inputLen, Signedness s)
{
    sign = ((s==SIGNED) && (input[0] & 0x80)) ? NEGATIVE : POSITIVE;

    while (input[0]==0 && inputLen>0)
    {
        input++;
        inputLen--;
    }

    reg.CleanNew(RoundupSize(bytesToWords(inputLen)));

    for (unsigned i=0; i<inputLen; i++)
        reg[i/WORD_SIZE] |= input[inputLen-1-i] << (i%WORD_SIZE)*8;

    if (sign == NEGATIVE)
    {
        for (unsigned i=inputLen; i<reg.size*WORD_SIZE; i++)
            reg[i/WORD_SIZE] |= 0xff << (i%WORD_SIZE)*8;
        TwosComplement(reg, reg.size);
    }
}

unsigned int Integer::MinEncodedSize(Signedness signedness) const
{
    unsigned int outputLen = STDMAX(1U, ByteCount());
    if (signedness == UNSIGNED)
        return outputLen;
    if (NotNegative() && (GetByte(outputLen-1) & 0x80))
        outputLen++;
    if (IsNegative() && *this < -Power2(outputLen*8-1))
        outputLen++;
    return outputLen;
}

unsigned int Integer::Encode(uint8_t *output, unsigned int outputLen, Signedness signedness) const
{
    if (signedness == UNSIGNED || NotNegative())
    {
        for (unsigned i=0; i<outputLen; i++)
            output[i]=GetByte(outputLen-i-1);
    }
    else
    {
        // take two's complement of *this
        Integer temp = Integer::Power2(8*STDMAX(ByteCount(), outputLen)) + *this;
        for (unsigned i=0; i<outputLen; i++)
            output[i]=temp.GetByte(outputLen-i-1);
    }
    return outputLen;
}

unsigned int Integer::DEREncode(uint8_t *output) const
{
    unsigned int i=0;
    output[i++] = INTEGER;
    unsigned int bc = MinEncodedSize(SIGNED);
    SecByteBlock buf(bc);
    Encode(buf, bc, SIGNED);
    i += DERLengthEncode(bc, output+i);
    memcpy(output+i, buf, bc);
    return i+bc;
}

unsigned int Integer::DEREncode(BufferedTransformation &bt) const
{
    bt.Put(INTEGER);
    unsigned int bc = MinEncodedSize(SIGNED);
    SecByteBlock buf(bc);
    Encode(buf, bc, SIGNED);
    unsigned int lengthBytes = DERLengthEncode(bc, bt);
    bt.Put(buf, bc);
    return 1+lengthBytes+bc;
}

void Integer::BERDecode(const uint8_t *input)
{
    if (*input++ != INTEGER)
        BERDecodeError();
    int bc;
    if (!(*input & 0x80))
        bc = *input++;
    else
    {
        int lengthBytes = *input++ & 0x7f;
        if (lengthBytes > 2)
            BERDecodeError();
        bc = *input++;
        if (lengthBytes > 1)
            bc = (bc << 8) | *input++;
    }
    Decode(input, bc, SIGNED);
}

void Integer::BERDecode(BufferedTransformation &bt)
{
    uint8_t b;
    if (!bt.Get(b) || b != INTEGER)
        BERDecodeError();

    unsigned int bc;
    BERLengthDecode(bt, bc);

    SecByteBlock buf(bc);

    if (bc != bt.Get(buf, bc))
        BERDecodeError();
    Decode(buf, bc, SIGNED);
}

void Integer::Randomize(RandomNumberGenerator &rng, unsigned int nbits)
{
    const unsigned int nbytes = nbits/8 + 1;
    SecByteBlock buf(nbytes);
    rng.GetBlock(buf, nbytes);
    buf[(unsigned int)0] = (uint8_t)Crop(buf[(unsigned int)0], nbits % 8);
    Decode(buf, nbytes, UNSIGNED);
}

void Integer::Randomize(RandomNumberGenerator &rng, const Integer &min, const Integer &max)
{
    assert(max > min);

    Integer range = max - min;
    const unsigned int nbits = range.BitCount();

    do
    {
        Randomize(rng, nbits);
    }
    while (*this > range);

    *this += min;
}

void Integer::Randomize(RandomNumberGenerator &rng, const Integer &min, const Integer &max, RandomNumberType rnType)
{
    while (1)   // loop until a suitable rn has been generated
    {
        Randomize(rng, min, max);

        switch (rnType)
        {
            case ANY:
                return;

            case ODD:
                reg[(unsigned int)0] |= 1;
                if (*this <= max)
                    return;
                else
                    break;

            case PRIME:
            case BLUMINT:
                if (NextPrime(*this, max, rnType==BLUMINT))
                    return;
                else
                    break;

            default:
                assert(false);
        }
    }
}

std::istream& operator>>(std::istream& in, Integer &a)
{
    char c;
    unsigned int length = 0;
    SecBlock<char> str(length + 16);

    ws(in);

    do
    {
        in.read(&c, 1);
        str[length++] = c;
        if (length >= str.size)
            str.Grow(length + 16);
    }
    while (in && (c=='-' || (c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F') || c=='h' || c=='H' || c=='o' || c=='O' || c==',' || c=='.'));

    if (in.gcount())
        in.putback(c);
    str[length-1] = '\0';
    a = Integer(str);

    return in;
}

ostream& operator<<(ostream& out, const Integer &a)
{
    // Get relevant conversion specifications from ostream.
    long f = out.flags() & ios::basefield;  // Get base digits.
    int base, block;
    char suffix;
    switch(f)
    {
    case ios::oct :
        base = 8;
        block = 8;
        suffix = 'o';
        break;
    case ios::hex :
        base = 16;
        block = 4;
        suffix = 'h';
        break;
    default :
        base = 10;
        block = 3;
        suffix = '.';
    }

    SecBlock<char> s(a.BitCount() / (BitPrecision(base)-1) + 1);
    Integer temp1=a, temp2;
    unsigned i=0;
    const char vec[]="0123456789ABCDEF";

    if (a.IsNegative())
    {
        out << '-';
        temp1.Negate();
    }

    if (!a)
        out << '0';

    while (!!temp1)
    {
        s[i++]=vec[Integer::ShortDivide(temp2, temp1, base)];
        temp1=temp2;
    }

    while (i--)
    {
        out << s[i];
        if (i && !(i%block))
            out << ",";
    }
    return out << suffix;
}

Integer& Integer::operator++()
{
    if (NotNegative())
    {
        if (Increment(reg, reg.size))
        {
            reg.CleanGrow(2*reg.size);
            reg[reg.size/2]=1;
        }
    }
    else
    {
        word borrow = Decrement(reg, reg.size);
        assert(!borrow);
        if (WordCount()==0)
            *this = Zero();
    }
    return *this;
}

Integer& Integer::operator--()
{
    if (IsNegative())
    {
        if (Increment(reg, reg.size))
        {
            reg.CleanGrow(2*reg.size);
            reg[reg.size/2]=1;
        }
    }
    else
    {
        if (Decrement(reg, reg.size))
            *this = -One();
    }
    return *this;
}

void PositiveAdd(Integer &sum, const Integer &a, const Integer& b)
{
    word carry;
    if (a.reg.size == b.reg.size)
        carry = Add(sum.reg, a.reg, b.reg, a.reg.size);
    else if (a.reg.size > b.reg.size)
    {
        carry = Add(sum.reg, a.reg, b.reg, b.reg.size);
        CopyWords(sum.reg+b.reg.size, a.reg+b.reg.size, a.reg.size-b.reg.size);
        carry = Increment(sum.reg+b.reg.size, a.reg.size-b.reg.size, carry);
    }
    else
    {
        carry = Add(sum.reg, a.reg, b.reg, a.reg.size);
        CopyWords(sum.reg+a.reg.size, b.reg+a.reg.size, b.reg.size-a.reg.size);
        carry = Increment(sum.reg+a.reg.size, b.reg.size-a.reg.size, carry);
    }

    if (carry)
    {
        sum.reg.CleanGrow(2*sum.reg.size);
        sum.reg[sum.reg.size/2] = 1;
    }
    sum.sign = Integer::POSITIVE;
}

void PositiveSubtract(Integer &diff, const Integer &a, const Integer& b)
{
    unsigned aSize = a.WordCount();
    aSize += aSize%2;
    unsigned bSize = b.WordCount();
    bSize += bSize%2;

    if (aSize == bSize)
    {
        if (Compare(a.reg, b.reg, aSize) >= 0)
        {
            Subtract(diff.reg, a.reg, b.reg, aSize);
            diff.sign = Integer::POSITIVE;
        }
        else
        {
            Subtract(diff.reg, b.reg, a.reg, aSize);
            diff.sign = Integer::NEGATIVE;
        }
    }
    else if (aSize > bSize)
    {
        word borrow = Subtract(diff.reg, a.reg, b.reg, bSize);
        CopyWords(diff.reg+bSize, a.reg+bSize, aSize-bSize);
        borrow = Decrement(diff.reg+bSize, aSize-bSize, borrow);
        assert(!borrow);
        diff.sign = Integer::POSITIVE;
    }
    else
    {
        word borrow = Subtract(diff.reg, b.reg, a.reg, aSize);
        CopyWords(diff.reg+aSize, b.reg+aSize, bSize-aSize);
        borrow = Decrement(diff.reg+aSize, bSize-aSize, borrow);
        assert(!borrow);
        diff.sign = Integer::NEGATIVE;
    }
}

Integer operator+(const Integer &a, const Integer& b)
{
    Integer sum((word)0, STDMAX(a.reg.size, b.reg.size));
    if (a.NotNegative())
    {
        if (b.NotNegative())
            PositiveAdd(sum, a, b);
        else
            PositiveSubtract(sum, a, b);
    }
    else
    {
        if (b.NotNegative())
            PositiveSubtract(sum, b, a);
        else
        {
            PositiveAdd(sum, a, b);
            sum.sign = Integer::NEGATIVE;
        }
    }
    return sum;
}

Integer& Integer::operator+=(const Integer& t)
{
    reg.CleanGrow(t.reg.size);
    if (NotNegative())
    {
        if (t.NotNegative())
            PositiveAdd(*this, *this, t);
        else
            PositiveSubtract(*this, *this, t);
    }
    else
    {
        if (t.NotNegative())
            PositiveSubtract(*this, t, *this);
        else
        {
            PositiveAdd(*this, *this, t);
            sign = Integer::NEGATIVE;
        }
    }
    return *this;
}

Integer operator-(const Integer &a, const Integer& b)
{
    Integer diff((word)0, STDMAX(a.reg.size, b.reg.size));
    if (a.NotNegative())
    {
        if (b.NotNegative())
            PositiveSubtract(diff, a, b);
        else
            PositiveAdd(diff, a, b);
    }
    else
    {
        if (b.NotNegative())
        {
            PositiveAdd(diff, a, b);
            diff.sign = Integer::NEGATIVE;
        }
        else
            PositiveSubtract(diff, b, a);
    }
    return diff;
}

Integer& Integer::operator-=(const Integer& t)
{
    reg.CleanGrow(t.reg.size);
    if (NotNegative())
    {
        if (t.NotNegative())
            PositiveSubtract(*this, *this, t);
        else
            PositiveAdd(*this, *this, t);
    }
    else
    {
        if (t.NotNegative())
        {
            PositiveAdd(*this, *this, t);
            sign = Integer::NEGATIVE;
        }
        else
            PositiveSubtract(*this, t, *this);
    }
    return *this;
}

Integer& Integer::operator<<=(unsigned int n)
{
    const unsigned int wordCount = WordCount();
    const unsigned int shiftWords = n / WORD_BITS;
    const unsigned int shiftBits = n % WORD_BITS;

    reg.CleanGrow(RoundupSize(wordCount+bitsToWords(n)));
    ShiftWordsLeftByWords(reg, wordCount + shiftWords, shiftWords);
    ShiftWordsLeftByBits(reg+shiftWords, wordCount+bitsToWords(shiftBits), shiftBits);
    return *this;
}

Integer& Integer::operator>>=(unsigned int n)
{
    const unsigned int wordCount = WordCount();
    const unsigned int shiftWords = n / WORD_BITS;
    const unsigned int shiftBits = n % WORD_BITS;

    ShiftWordsRightByWords(reg, wordCount, shiftWords);
    if (wordCount > shiftWords)
        ShiftWordsRightByBits(reg, wordCount-shiftWords, shiftBits);
    if (IsNegative() && WordCount()==0)   // avoid -0
        *this = Zero();
    return *this;
}

void PositiveMultiply(Integer &product, const Integer &a, const Integer &b)
{
    unsigned aSize = RoundupSize(a.WordCount());
    unsigned bSize = RoundupSize(b.WordCount());

    product.reg.CleanNew(RoundupSize(aSize+bSize));
    product.sign = Integer::POSITIVE;

    SecWordBlock workspace(aSize + bSize);
    AsymmetricMultiply(product.reg, workspace, a.reg, aSize, b.reg, bSize);
}

void Multiply(Integer &product, const Integer &a, const Integer &b)
{
    PositiveMultiply(product, a, b);

    if (a.NotNegative() != b.NotNegative())
        product.Negate();
}

Integer operator*(const Integer &a, const Integer &b)
{
    Integer product;
    Multiply(product, a, b);
    return product;
}

/*
void PositiveDivide(Integer &remainder, Integer &quotient,
                   const Integer &dividend, const Integer &divisor)
{
    remainder.reg.CleanNew(divisor.reg.size);
    remainder.sign = Integer::POSITIVE;
    quotient.reg.New(0);
    quotient.sign = Integer::POSITIVE;
    unsigned i=dividend.BitCount();
    while (i--)
    {
        word overflow = ShiftWordsLeftByBits(remainder.reg, remainder.reg.size, 1);
        remainder.reg[0] |= dividend[i];
        if (overflow || remainder >= divisor)
        {
            Subtract(remainder.reg, remainder.reg, divisor.reg, remainder.reg.size);
            quotient.SetBit(i);
        }
    }
}
*/

void PositiveDivide(Integer &remainder, Integer &quotient,
                   const Integer &a, const Integer &b)
{
    unsigned aSize = a.WordCount();
    unsigned bSize = b.WordCount();

    if (!bSize)
    {
#ifdef THROW_EXCEPTIONS
        throw Integer::DivideByZero();
#else
        return;
#endif
    }

    if (a.PositiveCompare(b) == -1)
    {
        remainder = a;
        remainder.sign = Integer::POSITIVE;
        quotient = Integer::Zero();
        return;
    }

    aSize += aSize%2;   // round up to next even number
    bSize += bSize%2;

    remainder.reg.CleanNew(RoundupSize(bSize));
    remainder.sign = Integer::POSITIVE;
    quotient.reg.CleanNew(RoundupSize(aSize-bSize+2));
    quotient.sign = Integer::POSITIVE;

    SecWordBlock T(aSize+2*bSize+4);
    Divide(remainder.reg, quotient.reg, T, a.reg, aSize, b.reg, bSize);
}

void Integer::Divide(Integer &remainder, Integer &quotient, const Integer &dividend, const Integer &divisor)
{
    PositiveDivide(remainder, quotient, dividend, divisor);

    if (dividend.IsNegative())
    {
        quotient.Negate();
        if (!!remainder)
        {
            --quotient;
            remainder = divisor.AbsoluteValue() - remainder;
        }
    }

    if (divisor.IsNegative())
        quotient.Negate();
}

Integer operator/(const Integer &a, const Integer &b)
{
    Integer remainder, quotient;
    Integer::Divide(remainder, quotient, a, b);
    return quotient;
}

Integer operator%(const Integer &a, const Integer &b)
{
    Integer remainder, quotient;
    Integer::Divide(remainder, quotient, a, b);
    return remainder;
}

word Integer::ShortDivide(Integer &quotient, const Integer &dividend, word divisor)
{
#ifdef THROW_EXCEPTIONS
    if (!divisor)
        throw Integer::DivideByZero();
#endif

    assert(divisor);

    if ((divisor & (divisor-1)) == 0)   // divisor is a power of 2
    {
        quotient = dividend >> (BitPrecision(divisor)-1);
        return dividend.reg[(unsigned int)0] & (divisor-1);
    }

    unsigned int i = dividend.WordCount();
    quotient.reg.CleanNew(RoundupSize(i));
    word remainder = 0;
    while (i--)
    {
        quotient.reg[i] = word(MAKE_DWORD(dividend.reg[i], remainder) / divisor);
        remainder = word(MAKE_DWORD(dividend.reg[i], remainder) % divisor);
    }

    if (dividend.NotNegative())
        quotient.sign = POSITIVE;
    else
    {
        quotient.sign = NEGATIVE;
        if (remainder)
        {
            --quotient;
            remainder = divisor - remainder;
        }
    }

    return remainder;
}

Integer operator/(const Integer &a, word b)
{
    Integer quotient;
    Integer::ShortDivide(quotient, a, b);
    return quotient;
}

word operator%(const Integer &dividend, word divisor)
{
#ifdef THROW_EXCEPTIONS
    if (!divisor)
        throw Integer::DivideByZero();
#endif

    assert(divisor);

    word remainder;

    if ((divisor & (divisor-1)) == 0)   // divisor is a power of 2
        remainder = dividend.reg[(unsigned int)0] & (divisor-1);
    else
    {
        unsigned int i = dividend.WordCount();

        if (divisor <= 5)
        {
            dword sum=0;
            while (i--)
                sum += dividend.reg[i];
            remainder = word(sum%divisor);
        }
        else
        {
            remainder = 0;
            while (i--)
                remainder = word(MAKE_DWORD(dividend.reg[i], remainder) % divisor);
        }
    }

    if (dividend.IsNegative() && remainder)
        remainder = divisor - remainder;

    return remainder;
}

void Integer::Negate()
{
    if (!!(*this))  // don't flip sign if *this==0
        sign = Sign(1-sign);
}

int Integer::PositiveCompare(const Integer& t) const
{
    unsigned size = WordCount(), tSize = t.WordCount();

    if (size == tSize)
        return ::Compare(reg, t.reg, size);
    else
        return size > tSize ? 1 : -1;
}

int Integer::Compare(const Integer& t) const
{
    if (NotNegative())
    {
        if (t.NotNegative())
            return PositiveCompare(t);
        else
            return 1;
    }
    else
    {
        if (t.NotNegative())
            return -1;
        else
            return -PositiveCompare(t);
    }
}

Integer Integer::SquareRoot() const
{
    // overestimate square root
    Integer x, y = Power2((BitCount()+1)/2);
    assert(y*y >= *this);

    do
    {
        x = y;
        y = (x + *this/x) >> 1;
    } while (y<x);

    return x;
}

bool Integer::IsSquare() const
{
    Integer r = SquareRoot();
    return *this == r.Squared();
}

bool Integer::IsUnit() const
{
    return (WordCount() == 1) && (reg[(unsigned int)0] == 1);
}

Integer Integer::MultiplicativeInverse() const
{
    return IsUnit() ? *this : Zero();
}

Integer a_times_b_mod_c(const Integer &x, const Integer& y, const Integer& m)
{
    return x*y%m;
}

Integer a_exp_b_mod_c(const Integer &x, const Integer& e, const Integer& m)
{
    if (m.IsEven())
    {
        ModularArithmetic mr(m);
        return mr.ConvertOut(mr.Exponentiate(mr.ConvertIn(x), e));
    }
    else
    {
        MontgomeryRepresentation mr(m);
        return mr.ConvertOut(mr.Exponentiate(mr.ConvertIn(x), e));
    }
}

Integer Integer::Gcd(const Integer &a, const Integer &b)
{
    return EuclideanDomainOf<Integer>().Gcd(a, b);
}

Integer Integer::InverseMod(const Integer &m) const
{
    assert(m.NotNegative());

    if (IsNegative() || *this>=m)
        return (*this%m).InverseMod(m);

    if (m.IsEven())
    {
        if (!m || IsEven())
            return Zero();  // no inverse
        if (*this == One())
            return One();

        Integer u = m.InverseMod(*this);
        return !u ? Zero() : (m*(*this-u)+1)/(*this);
    }

    SecBlock<word> T(m.reg.size * 4);
    Integer r((word)0, m.reg.size);
    unsigned k = AlmostInverse(r.reg, T, reg, reg.size, m.reg, m.reg.size);
    DivideByPower2Mod(r.reg, r.reg, k, m.reg, m.reg.size);
    return r;
}

// ********************************************************

Integer ModularArithmetic::Add(const Integer &a, const Integer &b) const
{
    if (a.reg.size==modulus.reg.size && b.reg.size==modulus.reg.size)
    {
        if (::Add(result.reg.ptr, a.reg, b.reg, a.reg.size)
            || Compare(result.reg, modulus.reg, a.reg.size) >= 0)
        {
            ::Subtract(result.reg.ptr, result.reg, modulus.reg, a.reg.size);
        }
        return result;
    }
    else
        {Integer r=a+b; if (r>=modulus) r-=modulus; return r;}
}

Integer& ModularArithmetic::Accumulate(Integer &a, const Integer &b) const
{
    if (a.reg.size==modulus.reg.size && b.reg.size==modulus.reg.size)
    {
        if (::Add(a.reg, a.reg, b.reg, a.reg.size)
            || Compare(a.reg, modulus.reg, a.reg.size) >= 0)
        {
            ::Subtract(a.reg, a.reg, modulus.reg, a.reg.size);
        }
    }
    else
        {a+=b; if (a>=modulus) a-=modulus;}

    return a;
}

Integer ModularArithmetic::Subtract(const Integer &a, const Integer &b) const
{
    if (a.reg.size==modulus.reg.size && b.reg.size==modulus.reg.size)
    {
        if (::Subtract(result.reg.ptr, a.reg, b.reg, a.reg.size))
            ::Add(result.reg.ptr, result.reg, modulus.reg, a.reg.size);
        return result;
    }
    else
        return Add(a, Inverse(b));
}

Integer& ModularArithmetic::Reduce(Integer &a, const Integer &b) const
{
    if (a.reg.size==modulus.reg.size && b.reg.size==modulus.reg.size)
    {
        if (::Subtract(a.reg, a.reg, b.reg, a.reg.size))
            ::Add(a.reg, a.reg, modulus.reg, a.reg.size);
    }
    else
        Accumulate(a, Inverse(b));

    return a;
}

Integer ModularArithmetic::Inverse(const Integer &a) const
{
    if (!a)
        return a;

    CopyWords(result.reg.ptr, modulus.reg, modulus.reg.size);
    if (::Subtract(result.reg.ptr, result.reg, a.reg, a.reg.size))
        Decrement(result.reg.ptr+a.reg.size, 1, modulus.reg.size-a.reg.size);

    return result;
}

Integer ModularArithmetic::MultiplicativeInverse(const Integer &a) const
{
    return a.InverseMod(modulus);
}

Integer ModularArithmetic::Exponentiate(const Integer &a, const Integer &e) const
{
    if (modulus.IsOdd())
    {
        MontgomeryRepresentation dr(modulus);
        return dr.ConvertOut(dr.Exponentiate(dr.ConvertIn(a), e));
    }
    else
        return AbstractRing<Integer>::Exponentiate(a, e);
}

Integer ModularArithmetic::CascadeExponentiate(const Integer &x, const Integer &e1, const Integer &y, const Integer &e2) const
{
    if (modulus.IsOdd())
    {
        MontgomeryRepresentation dr(modulus);
        return dr.ConvertOut(dr.CascadeExponentiate(dr.ConvertIn(x), e1, dr.ConvertIn(y), e2));
    }
    else
        return AbstractRing<Integer>::CascadeExponentiate(x, e1, y, e2);
}

MontgomeryRepresentation::MontgomeryRepresentation(const Integer &m)    // modulus must be odd
    : ModularArithmetic(m),
      u((word)0, modulus.reg.size),
      workspace(5*modulus.reg.size)
{
    assert(modulus.IsOdd());
    RecursiveInverseModPower2(u.reg, workspace, modulus.reg, modulus.reg.size);
}

Integer MontgomeryRepresentation::Multiply(const Integer &a, const Integer &b) const
{
    word *const T = workspace.ptr;
    word *const R = result.reg.ptr;
    const unsigned int N = modulus.reg.size;
    assert(a.reg.size<=N && b.reg.size<=N);

    AsymmetricMultiply(T, T+2*N, a.reg, a.reg.size, b.reg, b.reg.size);
    SetWords(T+a.reg.size+b.reg.size, 0, 2*N-a.reg.size-b.reg.size);
    MontgomeryReduce(R, T+2*N, T, modulus.reg, u.reg, N);
    return result;
}

Integer MontgomeryRepresentation::Square(const Integer &a) const
{
    word *const T = workspace.ptr;
    word *const R = result.reg.ptr;
    const unsigned int N = modulus.reg.size;
    assert(a.reg.size<=N);

    RecursiveSquare(T, T+2*N, a.reg, a.reg.size);
    SetWords(T+2*a.reg.size, 0, 2*N-2*a.reg.size);
    MontgomeryReduce(R, T+2*N, T, modulus.reg, u.reg, N);
    return result;
}

Integer MontgomeryRepresentation::ConvertOut(const Integer &a) const
{
    word *const T = workspace.ptr;
    word *const R = result.reg.ptr;
    const unsigned int N = modulus.reg.size;
    assert(a.reg.size<=N);

    CopyWords(T, a.reg, a.reg.size);
    SetWords(T+a.reg.size, 0, 2*N-a.reg.size);
    MontgomeryReduce(R, T+2*N, T, modulus.reg, u.reg, N);
    return result;
}

Integer MontgomeryRepresentation::MultiplicativeInverse(const Integer &a) const
{
//    return (EuclideanMultiplicativeInverse(a, modulus)<<(2*WORD_BITS*modulus.reg.size))%modulus;
    word *const T = workspace.ptr;
    word *const R = result.reg.ptr;
    const unsigned int N = modulus.reg.size;
    assert(a.reg.size<=N);

    CopyWords(T, a.reg, a.reg.size);
    SetWords(T+a.reg.size, 0, 2*N-a.reg.size);
    MontgomeryReduce(R, T+2*N, T, modulus.reg, u.reg, N);
    unsigned k = AlmostInverse(R, T, R, N, modulus.reg, N);

//  cout << "k=" << k << " N*32=" << 32*N << endl;

    if (k>N*WORD_BITS)
        DivideByPower2Mod(R, R, k-N*WORD_BITS, modulus.reg, N);
    else
        MultiplyByPower2Mod(R, R, N*WORD_BITS-k, modulus.reg, N);

    return result;
}

HalfMontgomeryRepresentation::HalfMontgomeryRepresentation(const Integer &m)    // modulus must be odd
    : ModularArithmetic(m),
      v((modulus.reg.CleanGrow(4), Integer::Power2(3*WORD_BITS*modulus.reg.size/2)%modulus)),
      u((word)0, modulus.reg.size/2),
      workspace(4*modulus.reg.size)
{
    assert(modulus.IsOdd());

    result.reg.Grow(4);
    v.reg.CleanGrow(modulus.reg.size);
    RecursiveInverseModPower2(u.reg, workspace, modulus.reg, modulus.reg.size/2);
}

Integer HalfMontgomeryRepresentation::Multiply(const Integer &a, const Integer &b) const
{
    word *const T = workspace.ptr;
    word *const R = result.reg.ptr;
    const unsigned int N = modulus.reg.size;
    assert(a.reg.size<=N && b.reg.size<=N);

    AsymmetricMultiply(T, T+2*N, a.reg, a.reg.size, b.reg, b.reg.size);
    SetWords(T+a.reg.size+b.reg.size, 0, 2*N-a.reg.size-b.reg.size);
    HalfMontgomeryReduce(R, T+2*N, T, modulus.reg, u.reg, v.reg, N);
    return result;
}

Integer HalfMontgomeryRepresentation::Square(const Integer &a) const
{
    word *const T = workspace.ptr;
    const unsigned int N = modulus.reg.size;
    word *const R = result.reg.ptr;
    assert(a.reg.size<=N);

    RecursiveSquare(T, T+2*N, a.reg, a.reg.size);
    SetWords(T+2*a.reg.size, 0, 2*N-2*a.reg.size);
    HalfMontgomeryReduce(R, T+2*N, T, modulus.reg, u.reg, v.reg, N);
    return result;
}

Integer HalfMontgomeryRepresentation::ConvertOut(const Integer &a) const
{
    word *const T = workspace.ptr;
    word *const R = result.reg.ptr;
    const unsigned int N = modulus.reg.size;
    assert(a.reg.size<=N);

    CopyWords(T, a.reg, a.reg.size);
    SetWords(T+a.reg.size, 0, 2*N-a.reg.size);
    HalfMontgomeryReduce(R, T+2*N, T, modulus.reg, u.reg, v.reg, N);
    return result;
}

Integer HalfMontgomeryRepresentation::MultiplicativeInverse(const Integer &a) const
{
    return (a.InverseMod(modulus)<<(WORD_BITS*modulus.reg.size))%modulus;
}
