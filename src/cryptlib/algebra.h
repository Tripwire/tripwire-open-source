#ifndef ALGEBRA_H
#define ALGEBRA_H

class Integer;

template <class T> class AbstractGroup
{
public:
    typedef T Element;

    virtual ~AbstractGroup() {}

    virtual bool Equal(const Element &a, const Element &b) const =0;
    virtual Element Zero() const =0;
    virtual Element Add(const Element &a, const Element &b) const =0;
    virtual Element Inverse(const Element &a) const =0;

    virtual Element Double(const Element &a) const;
    virtual Element Subtract(const Element &a, const Element &b) const;
    virtual Element& Accumulate(Element &a, const Element &b) const;
    virtual Element& Reduce(Element &a, const Element &b) const;

    virtual Element IntMultiply(const Element &a, const Integer &e) const;
    virtual Element CascadeIntMultiply(const Element &x, const Integer &e1, const Element &y, const Integer &e2) const;
};

template <class T> class AbstractRing : public AbstractGroup<T>
{
public:
    typedef T Element;

    virtual bool IsUnit(const Element &a) const =0;
    virtual Element One() const =0;
    virtual Element Multiply(const Element &a, const Element &b) const =0;
    virtual Element MultiplicativeInverse(const Element &a) const =0;

    virtual Element Square(const Element &a) const;
    virtual Element Divide(const Element &a, const Element &b) const;

    virtual Element Exponentiate(const Element &a, const Integer &e) const;
    virtual Element CascadeExponentiate(const Element &x, const Integer &e1, const Element &y, const Integer &e2) const;
};

// ********************************************************

// VC50 workaround: no member template support yet
template <class Element, class Iterator> Element GeneralCascadeMultiplication(const AbstractGroup<Element> &group, Iterator begin, Iterator end);
#if defined(BREAK_GCC34)
template <class Element, class Iterator> Element GeneralCascadeExponentiation(const AbstractRing<Element> &ring, Iterator begin, Iterator end);
#endif /* BREAK_GCC34 */

// ********************************************************

template <class T> class AbstractField : public AbstractRing<T>
{
public:
    bool IsUnit(const typename T::Element &a) const
        {return !this->Equal(a, this->Zero());}
};

template <class T> class AbstractEuclideanDomain : public AbstractRing<T>
{
public:
    typedef T Element;

    virtual void DivisionAlgorithm(Element &r, Element &q, const Element &a, const Element &d) const =0;

    virtual Element Mod(const Element &a, const Element &b) const;
    virtual Element Gcd(const Element &a, const Element &b) const;
};

// ********************************************************

template <class T> class MultiplicativeGroup : public AbstractGroup<typename T::Element>
{
public:
    typedef T Ring;

    MultiplicativeGroup(const Ring &m_ring)
        : m_ring(m_ring) {}

    const Ring & GetRing() const
        {return m_ring;}

    bool Equal(const typename T::Element &a, const typename T::Element &b) const
        {return m_ring.Equal(a, b);}

    typename T::Element Zero() const
        {return m_ring.One();}

    typename T::Element Add(const typename T::Element &a, const typename T::Element &b) const
        {return m_ring.Multiply(a, b);}

    typename T::Element& Accumulate(typename T::Element &a, const typename T::Element &b) const
        {return a = m_ring.Multiply(a, b);}

    typename T::Element Inverse(const typename T::Element &a) const
        {return m_ring.MultiplicativeInverse(a);}

    typename T::Element Subtract(const typename T::Element &a, const typename T::Element &b) const
        {return m_ring.Divide(a, b);}

    typename T::Element& Reduce(typename T::Element &a, const typename T::Element &b) const
        {return a = m_ring.Divide(a, b);}

    typename T::Element Double(const typename T::Element &a) const
        {return m_ring.Square(a);}

protected:
    const Ring &m_ring;
};

template <class T> class EuclideanDomainOf : public AbstractEuclideanDomain<T>
{
public:
    typedef T Element;

    EuclideanDomainOf() {}

    bool Equal(const Element &a, const Element &b) const
        {return a==b;}

    Element Zero() const
        {return Element::Zero();}

    Element Add(const Element &a, const Element &b) const
        {return a+b;}

    Element& Accumulate(Element &a, const Element &b) const
        {return a+=b;}

    Element Inverse(const Element &a) const
        {return -a;}

    Element Subtract(const Element &a, const Element &b) const
        {return a-b;}

    Element& Reduce(Element &a, const Element &b) const
        {return a-=b;}

    Element Double(const Element &a) const
        {return a.Doubled();}

    Element One() const
        {return Element::One();}

    Element Multiply(const Element &a, const Element &b) const
        {return a*b;}

    Element Square(const Element &a) const
        {return a.Squared();}

    bool IsUnit(const Element &a) const
        {return a.IsUnit();}

    Element MultiplicativeInverse(const Element &a) const
        {return a.MultiplicativeInverse();}

    Element Divide(const Element &a, const Element &b) const
        {return a/b;}

    Element Mod(const Element &a, const Element &b) const
        {return a%b;}

    void DivisionAlgorithm(Element &r, Element &q, const Element &a, const Element &d) const
        {Element::Divide(r, q, a, d);}
};

template <class T> class QuotientRing : public AbstractRing<typename T::Element>
{
public:
    typedef T EuclideanDomain;

    QuotientRing(const EuclideanDomain &domain, const typename T::Element &modulus)
        : m_domain(domain), m_modulus(modulus) {}

    const EuclideanDomain & GetDomain() const
        {return m_domain;}

    const typename T::Element & GetModulus() const
        {return m_modulus;}

    bool Equal(const typename T::Element &a, const typename T::Element &b) const
        {return m_domain.Equal(m_domain.Mod(m_domain.Subtract(a, b), m_modulus), m_domain.Zero());}

    typename T::Element Zero() const
        {return m_domain.Zero();}

    typename T::Element Add(const typename T::Element &a, const typename T::Element &b) const
        {return m_domain.Add(a, b);}

    typename T::Element& Accumulate(typename T::Element &a, const typename T::Element &b) const
        {return m_domain.Accumulate(a, b);}

    typename T::Element Inverse(const typename T::Element &a) const
        {return m_domain.Inverse(a);}

    typename T::Element Subtract(const typename T::Element &a, const typename T::Element &b) const
        {return m_domain.Subtract(a, b);}

    typename T::Element& Reduce(typename T::Element &a, const typename T::Element &b) const
        {return m_domain.Reduce(a, b);}

    typename T::Element Double(const typename T::Element &a) const
        {return m_domain.Double(a);}

    bool IsUnit(const typename T::Element &a) const
        {return m_domain.IsUnit(m_domain.Gcd(a, m_modulus));}

    typename T::Element One() const
        {return m_domain.One();}

    typename T::Element Multiply(const typename T::Element &a, const typename T::Element &b) const
        {return m_domain.Mod(m_domain.Multiply(a, b), m_modulus);}

    typename T::Element Square(const typename T::Element &a) const
        {return m_domain.Mod(m_domain.Square(a), m_modulus);}

    typename QuotientRing<T>::Element MultiplicativeInverse(const typename T::Element &a) const;

protected:
    const EuclideanDomain &m_domain;
    typename T::Element m_modulus;
};

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// BEGIN TEMPLATE DEFINITIONS

#include <vector>
//USING_NAMESPACE(std)

template <class T> T AbstractGroup<T>::Double(const Element &a) const
{
    return Add(a, a);
}

template <class T> T AbstractGroup<T>::Subtract(const Element &a, const Element &b) const
{
    return Add(a, Inverse(b));
}

template <class T> T& AbstractGroup<T>::Accumulate(Element &a, const Element &b) const
{
    return a = Add(a, b);
}

template <class T> T& AbstractGroup<T>::Reduce(Element &a, const Element &b) const
{
    return a = Subtract(a, b);
}

template <class T> T AbstractRing<T>::Square(const Element &a) const
{
    return Multiply(a, a);
}

template <class T> T AbstractRing<T>::Divide(const Element &a, const Element &b) const
{
    return Multiply(a, MultiplicativeInverse(b));
}

template <class T> T AbstractEuclideanDomain<T>::Mod(const Element &a, const Element &b) const
{
    Element r, q;
    DivisionAlgorithm(r, q, a, b);
    return r;
}

template <class T> T AbstractEuclideanDomain<T>::Gcd(const Element &a, const Element &b) const
{
    Element g[3]={b, a};
    unsigned int i0=0, i1=1, i2=2;

    while (!this->Equal(g[i1], this->Zero()))
    {
        g[i2] = Mod(g[i0], g[i1]);
        unsigned int t = i0; i0 = i1; i1 = i2; i2 = t;
    }

    return g[i0];
}

template <class T> typename QuotientRing<T>::Element QuotientRing<T>::MultiplicativeInverse(const typename T::Element &a) const
{
    typename T::Element g[3]={m_modulus, a};
    typename T::Element v[3]={m_domain.Zero(), m_domain.One()};
    typename T::Element y;
    unsigned int i0=0, i1=1, i2=2;

    while (!this->Equal(g[i1], Zero()))
    {
        // y = g[i0] / g[i1];
        // g[i2] = g[i0] % g[i1];
        m_domain.DivisionAlgorithm(g[i2], y, g[i0], g[i1]);
        // v[i2] = v[i0] - (v[i1] * y);
        v[i2] = m_domain.Subtract(v[i0], m_domain.Multiply(v[i1], y));
        unsigned int t = i0; i0 = i1; i1 = i2; i2 = t;
    }

    return m_domain.IsUnit(g[i0]) ? m_domain.Divide(v[i0], g[i0]) : m_domain.Zero();
}

template <class T> T AbstractGroup<T>::IntMultiply(const Element &base, const Integer &exponent) const
{
    unsigned int expLen = exponent.BitCount();
    if (expLen==0)
        return Zero();

    unsigned powerTableSize = (expLen <= 17 ? 1 : (expLen <= 24 ? 2 : (expLen <= 70 ? 4 : (expLen <= 197 ? 8 : (expLen <= 539 ? 16 : (expLen <= 1434 ? 32 : 64))))));
    std::vector<Element> powerTable(powerTableSize);

    powerTable[0] = base;
    if (powerTableSize > 1)
    {
        Element temp = Double(base);
        for (unsigned i=1; i<powerTableSize; i++)
            powerTable[i] = Add(temp, powerTable[i-1]);
    }

    Element result;
    unsigned power = 0, prevPosition = expLen-1;
    bool firstTime = true;

    for (int i = expLen-1; i>=0; i--)
    {
        power = 2*power + exponent.GetBit(i);

        if (i==0 || power >= powerTableSize)
        {
            unsigned squaresBefore = prevPosition-i;
            unsigned squaresAfter = 0;
            prevPosition = i;
            while (power && power%2 == 0)
            {
                power /= 2;
                squaresBefore--;
                squaresAfter++;
            }
            if (firstTime)
            {
                result = powerTable[(power-1)/2];
                firstTime = false;
            }
            else
            {
                while (squaresBefore--)
                    result = Double(result);
                if (power)
                    result = Add(powerTable[(power-1)/2], result);
            }
            while (squaresAfter--)
                result = Double(result);
            power = 0;
        }
    }
    return result;
}

template <class T> T AbstractGroup<T>::CascadeIntMultiply(const Element &x, const Integer &e1, const Element &y, const Integer &e2) const
{
    const unsigned expLen = STDMAX(e1.BitCount(), e2.BitCount());
    if (expLen==0)
        return Zero();

    const unsigned w = (expLen <= 46 ? 1 : (expLen <= 260 ? 2 : 3));
    const unsigned tableSize = 1<<w;
    std::vector<Element> powerTable(tableSize << w);

    powerTable[1] = x;
    powerTable[tableSize] = y;
    if (w==1)
        powerTable[3] = Add(x,y);
    else
    {
        powerTable[2] = Double(x);
        powerTable[2*tableSize] = Double(y);

        unsigned i, j;

        for (i=3; i<tableSize; i+=2)
            powerTable[i] = Add(powerTable[i-2], powerTable[2]);
        for (i=1; i<tableSize; i+=2)
            for (j=i+tableSize; j<(tableSize<<w); j+=tableSize)
                powerTable[j] = Add(powerTable[j-tableSize], y);

        for (i=3*tableSize; i<(tableSize<<w); i+=2*tableSize)
            powerTable[i] = Add(powerTable[i-2*tableSize], powerTable[2*tableSize]);
        for (i=tableSize; i<(tableSize<<w); i+=2*tableSize)
            for (j=i+2; j<i+tableSize; j+=2)
                powerTable[j] = Add(powerTable[j-1], x);
    }

    Element result;
    unsigned power1 = 0, power2 = 0, prevPosition = expLen-1;
    bool firstTime = true;

    for (int i = expLen-1; i>=0; i--)
    {
        power1 = 2*power1 + e1.GetBit(i);
        power2 = 2*power2 + e2.GetBit(i);

        if (i==0 || 2*power1 >= tableSize || 2*power2 >= tableSize)
        {
            unsigned squaresBefore = prevPosition-i;
            unsigned squaresAfter = 0;
            prevPosition = i;
            while ((power1 || power2) && power1%2 == 0 && power2%2==0)
            {
                power1 /= 2;
                power2 /= 2;
                squaresBefore--;
                squaresAfter++;
            }
            if (firstTime)
            {
                result = powerTable[(power2<<w) + power1];
                firstTime = false;
            }
            else
            {
                while (squaresBefore--)
                    result = Double(result);
                if (power1 || power2)
                    result = Add(powerTable[(power2<<w) + power1], result);
            }
            while (squaresAfter--)
                result = Double(result);
            power1 = power2 = 0;
        }
    }
    return result;
}

template <class Element, class Iterator> Element GeneralCascadeMultiplication(const AbstractGroup<Element> &group, Iterator begin, Iterator end)
{
    if (end-begin == 1)
        return group.IntMultiply((*begin).second, (*begin).first);
    else if (end-begin == 2)
        return group.CascadeIntMultiply((*begin).second, (*begin).first, (*(begin+1)).second, (*(begin+1)).first);
    else
    {
        Integer q, r;
        Iterator last = end;
        --last;

        make_heap(begin, end);
        pop_heap(begin, end);

        while (!!(*begin).first)
        {
            // (*last).first is largest exponent, (*begin).first is next largest
            Integer::Divide(r, q, (*last).first, (*begin).first);

            if (q == Integer::One())
                group.Accumulate((*begin).second, (*last).second);  // avoid overhead of GeneralizedMultiplication()
            else
                group.Accumulate((*begin).second, group.IntMultiply((*last).second, q));

            (*last).first = r;

            push_heap(begin, end);
            pop_heap(begin, end);
        }

        return group.IntMultiply((*last).second, (*last).first);
    }
}

template <class T> T AbstractRing<T>::Exponentiate(const Element &base, const Integer &exponent) const
{
    return MultiplicativeGroup<AbstractRing<T> >(*this).IntMultiply(base, exponent);
}

template <class T> T AbstractRing<T>::CascadeExponentiate(const Element &x, const Integer &e1, const Element &y, const Integer &e2) const
{
    return MultiplicativeGroup<AbstractRing<T> >(*this).CascadeIntMultiply(x, e1, y, e2);
}

#if defined(BREAK_GCC34)
template <class Element, class Iterator> Element GeneralCascadeExponentiation(const AbstractRing<Element> &ring, Iterator begin, Iterator end)
{
    MultiplicativeGroup<AbstractRing<Element> > mg(field);
    return GeneralCascadeMultiplication<Element>(mg, begin, end);
}
#endif /* BREAK_GCC34 */

#endif
