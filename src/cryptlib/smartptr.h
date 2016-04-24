#ifndef SMARTPTR_H
#define SMARTPTR_H

#include <algorithm>

template<class T> class member_ptr
{
public:
    explicit member_ptr(T *p = 0) : m_p(p) {}

    ~member_ptr();

    const T& operator*() const { return *m_p; }
    T& operator*() { return *m_p; }

    const T* operator->() const { return m_p; }
    T* operator->() { return m_p; }

    const T* get() const { return m_p; }
    T* get() { return m_p; }

    T* release()
    {
        T *old_p = m_p;
        m_p = 0;
        return old_p;
    } 

    void reset(T *p = 0);

private:
    member_ptr(const member_ptr<T>& rhs);       // copy not allowed
    void operator=(const member_ptr<T>& rhs);   // assignment not allowed

    T *m_p;
};

template <class T> member_ptr<T>::~member_ptr() {delete m_p;}
template <class T> void member_ptr<T>::reset(T *p) {delete m_p; m_p = p;}

// ********************************************************

template<class T> class counted_ptr
{
public:
    explicit counted_ptr(T *p = 0);
    counted_ptr(const counted_ptr<T>& rhs);

    ~counted_ptr();

    const T& operator*() const { return *m_p; }
    T& operator*() { return *m_p; }

    const T* operator->() const { return m_p; }
    T* operator->() { return m_p; }

    const T* get() const { return m_p; }
    T* get() { return m_p; }

    counted_ptr<T> & operator=(const counted_ptr<T>& rhs);

private:
    T *m_p;
};

template <class T> counted_ptr<T>::counted_ptr(T *p)
    : m_p(p) 
{
    if (m_p)
        m_p->m_referenceCount = 1;
}

template <class T> counted_ptr<T>::counted_ptr(const counted_ptr<T>& rhs)
    : m_p(rhs.m_p)
{
    if (m_p)
        m_p->m_referenceCount++;
}

template <class T> counted_ptr<T>::~counted_ptr()
{
    if (m_p && --m_p->m_referenceCount == 0)
        delete m_p;
}

template <class T> counted_ptr<T> & counted_ptr<T>::operator=(const counted_ptr<T>& rhs)
{
    if (m_p && --m_p->m_referenceCount == 0)
        delete m_p;
    m_p = rhs.m_p;
    if (m_p)
        m_p->m_referenceCount++;
    return *this;
}

// ********************************************************

template <class T> class vector_member_ptrs
{
public:
    vector_member_ptrs(unsigned int size=0)
        : _size(size) {ptr = new member_ptr<T>[_size];}
    ~vector_member_ptrs()
        {delete [] ptr;}

    member_ptr<T>& operator[](unsigned int index)
        {assert(index<_size); return ptr[index];}
    const member_ptr<T>& operator[](unsigned int index) const
        {assert(index<_size); return ptr[index];}

    unsigned int size() const {return _size;}
    void resize(unsigned int newSize)
    {
        member_ptr<T> *newPtr = new member_ptr<T>[newSize];
        for (unsigned int i=0; i<STDMIN(_size, newSize); i++)
            newPtr[i].reset(ptr[i].release());
        delete [] ptr;
        _size = newSize;
        ptr = newPtr;
    }

private:
    vector_member_ptrs(const vector_member_ptrs<T> &c); // copy not allowed
    void operator=(const vector_member_ptrs<T> &x);     // assignment not allowed

    unsigned int _size;
    member_ptr<T> *ptr;
};

#endif
