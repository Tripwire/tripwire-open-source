#include "pch.h"
#include "eprecomp.h"
#include "algebra.h"

#include <vector>
USING_NAMESPACE(std)

template <class T> void ExponentiationPrecomputation<T>::Precompute(const Element &base, unsigned int maxExpBits)
{
    assert(storage <= maxExpBits);
    exponentBase = Integer::Power2((maxExpBits+storage-1)/storage);

    g[0] = base;
    for (unsigned i=1; i<storage; i++)
        g[i] = group.IntMultiply(g[i-1], exponentBase);
}

template <class T> typename ExponentiationPrecomputation<T>::Element ExponentiationPrecomputation<T>::Exponentiate(const Integer &exponent) const
{
    vector<pair<Integer, Element> > eb(storage);    // array of segments of the exponent and precalculated bases
    Integer temp, e = exponent;
    unsigned i;

    for (i=0; i+1<storage; i++)
    {
        Integer::Divide(eb[i].first, temp, e, exponentBase);
        swap(temp, e);
        eb[i].second = g[i];
    }
    eb[i].first = e;
    eb[i].second = g[i];

    return GeneralCascadeMultiplication<Element>(group, eb.begin(), eb.end());
}

template <class T> typename ExponentiationPrecomputation<T>::Element 
    ExponentiationPrecomputation<T>::CascadeExponentiate(const Integer &exponent, 
        const ExponentiationPrecomputation<T> &pc2, const Integer &exponent2) const
{
    vector<pair<Integer, Element> > eb(storage+pc2.storage);    // array of segments of the exponent and precalculated bases
    Integer temp, e = exponent;
    unsigned i;

    for (i=0; i+1<storage; i++)
    {
        Integer::Divide(eb[i].first, temp, e, exponentBase);
        swap(temp, e);
        eb[i].second = g[i];
    }
    eb[i].first = e;
    eb[i].second = g[i];

    e = exponent2;
    for (i=storage; i+1<storage+pc2.storage; i++)
    {
        Integer::Divide(eb[i].first, temp, e, exponentBase);
        swap(temp, e);
        eb[i].second = pc2.g[i-storage];
    }
    eb[i].first = e;
    eb[i].second = pc2.g[i-storage];

    return GeneralCascadeMultiplication<Element>(group, eb.begin(), eb.end());
}
