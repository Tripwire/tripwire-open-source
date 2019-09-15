//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2019 Tripwire,
// Inc. Tripwire is a registered trademark of Tripwire, Inc.  All rights
// reserved.
//
// This program is free software.  The contents of this file are subject
// to the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.  You may redistribute it and/or modify it
// only in compliance with the GNU General Public License.
//
// This program is distributed in the hope that it will be useful.
// However, this program is distributed AS-IS WITHOUT ANY
// WARRANTY; INCLUDING THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS
// FOR A PARTICULAR PURPOSE.  Please see the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.
//
// Nothing in the GNU General Public License or any other license to use
// the code or files shall permit you to use Tripwire's trademarks,
// service marks, or other intellectual property without Tripwire's
// prior written consent.
//
// If you have any questions, please contact Tripwire, Inc. at either
// info@tripwire.org or www.tripwire.org.
//
///////////////////////////////////////////////////////////////////////////////
// bytequeue.cpp    - written and placed in the public domain by Wei Dai
//                  - modified 29 Oct 1998 mdb

#include "stdtwcrypto.h"

#include "bytequeue.h"
#include "cryptlib/queue.h"
#include "cryptlib/misc.h"

//-----------------------------------------------------------------------------
// cByteQueueNode
//      this class for use by cByteQueue only
//-----------------------------------------------------------------------------
class cByteQueueNode
{
public:
    explicit cByteQueueNode(unsigned int maxSize);

    unsigned int CurrentSize() const
    {
        return tail - head;
    }
    unsigned int UsedUp() const
    {
        return (head == MaxSize());
    }

    unsigned int Put(uint8_t inByte);
    unsigned int Put(const uint8_t* inString, unsigned int length);

    unsigned int Get(uint8_t& outByte);
    unsigned int Get(uint8_t* outString, unsigned int getMax);

    unsigned int Peek(uint8_t& outByte) const;

    void CopyTo(BufferedTransformation& target) const
    {
        target.Put(buf + head, tail - head);
    }
    void CopyTo(uint8_t* target) const
    {
        memcpy(target, buf + head, tail - head);
    }

    uint8_t operator[](unsigned int i) const
    {
        return buf[i - head];
    }

    cByteQueueNode* next;

private:
    unsigned int MaxSize() const
    {
        return buf.size;
    }

    SecByteBlock buf;
    unsigned int head, tail;
};


cByteQueueNode::cByteQueueNode(unsigned int maxSize) : buf(maxSize)
{
    head = tail = 0;
    next        = 0;
}

unsigned int cByteQueueNode::Put(uint8_t inByte)
{
    if (MaxSize() == tail)
        return 0;

    buf[tail++] = inByte;
    return 1;
}

unsigned int cByteQueueNode::Put(const uint8_t* inString, unsigned int length)
{
    unsigned int l = STDMIN(length, MaxSize() - tail);
    memcpy(buf + tail, inString, l);
    tail += l;
    return l;
}

unsigned int cByteQueueNode::Get(uint8_t& outByte)
{
    if (tail == head)
        return 0;

    outByte = buf[head++];
    return 1;
}

unsigned int cByteQueueNode::Get(uint8_t* outString, unsigned int getMax)
{
    unsigned int l = STDMIN(getMax, tail - head);
    memcpy(outString, buf + head, l);
    head += l;
    return l;
}

unsigned int cByteQueueNode::Peek(uint8_t& outByte) const
{
    if (tail == head)
        return 0;

    outByte = buf[head];
    return 1;
}

//-----------------------------------------------------------------------------
// cByteQueue
//-----------------------------------------------------------------------------

cByteQueue::cByteQueue(int nodeSize) : BufferedTransformation(), mNodeSize(nodeSize), mCurrentSize(0)
{
    head = tail = new cByteQueueNode(mNodeSize);
}

cByteQueue::cByteQueue(const cByteQueue& copy) : BufferedTransformation()
{
    CopyFrom(copy);
}

void cByteQueue::CopyFrom(const cByteQueue& copy)
{
    mNodeSize    = copy.mNodeSize;
    mCurrentSize = copy.mCurrentSize;

    head = tail = new cByteQueueNode(*copy.head);

    for (cByteQueueNode* current = copy.head->next; current; current = current->next)
    {
        tail->next = new cByteQueueNode(*current);
        tail       = tail->next;
    }

    tail->next = NULL;
}

cByteQueue::~cByteQueue()
{
    Destroy();
}

void cByteQueue::Destroy()
{
    cByteQueueNode* next;

    for (cByteQueueNode* current = head; current; current = next)
    {
        next = current->next;
        delete current;
    }

    mCurrentSize = 0;
}

///////////////////////////////////////////////////////////////////////////////
// CopyTo -- note that this does not remove anything from the queue
///////////////////////////////////////////////////////////////////////////////
void cByteQueue::CopyTo(BufferedTransformation& target) const
{
    for (cByteQueueNode* current = head; current; current = current->next)
        current->CopyTo(target);
}

void cByteQueue::CopyTo(uint8_t* target) const
{
    for (cByteQueueNode* current = head; current; current = current->next)
    {
        current->CopyTo(target);
        target += current->CurrentSize();
    }
}

unsigned long cByteQueue::CurrentSize() const
{
    return mCurrentSize;
    /*
    unsigned long size=0;

    for (cByteQueueNode *current=head; current; current=current->next)
        size += current->CurrentSize();

    return size;
    */
}

void cByteQueue::Put(uint8_t inByte)
{
    if (!tail->Put(inByte))
    {
        tail->next = new cByteQueueNode(mNodeSize);
        tail       = tail->next;
        tail->Put(inByte);
    }
    // put increases the size of the queue by one
    mCurrentSize++;
}

void cByteQueue::Put(const uint8_t* inString, unsigned int length)
{
    unsigned int l;

    // put increases the size of the queue by length
    mCurrentSize += length;

    while ((l = tail->Put(inString, length)) < length)
    {
        tail->next = new cByteQueueNode(mNodeSize);
        tail       = tail->next;
        inString += l;
        length -= l;
    }
}

unsigned int cByteQueue::Get(uint8_t& outByte)
{
    int l = head->Get(outByte);
    if (head->UsedUp())
    {
        cByteQueueNode* temp = head;
        head                 = head->next;
        delete temp;
        if (!head) // just deleted the last node
            head = tail = new cByteQueueNode(mNodeSize);
    }
    // put decreases the size of the queue by one
    mCurrentSize--;

    return l;
}

unsigned int cByteQueue::Get(uint8_t* outString, unsigned int getMax)
{
    unsigned int    getMaxSave = getMax;
    cByteQueueNode* current    = head;

    while (getMax && current)
    {
        int l = current->Get(outString, getMax);

        outString += l;
        getMax -= l;

        current = current->next;
    }

    // delete all used up nodes except the last one, to avoid the final new
    // that used to be below....
    while (head && head->UsedUp() && (head != tail))
    {
        current = head;
        head    = head->next;
        delete current;
    }

    /*
    if (!head)  // every single node has been used up and deleted
        head = tail = new cByteQueueNode(mNodeSize);
    */

    int rtn = getMaxSave - getMax;
    mCurrentSize -= rtn;

    return (rtn);
}

unsigned int cByteQueue::Peek(uint8_t& outByte) const
{
    return head->Peek(outByte);
}

cByteQueue& cByteQueue::operator=(const cByteQueue& rhs)
{
    if (this != &rhs)
    {
        Destroy();
        CopyFrom(rhs);
    }
    return *this;
}

bool cByteQueue::operator==(const cByteQueue& rhs) const
{
    const unsigned long currentSize = CurrentSize();

    if (currentSize != rhs.CurrentSize())
        return false;

    for (unsigned long i = 0; i < currentSize; i++)
        if ((*this)[i] != rhs[i])
            return false;

    return true;
}

uint8_t cByteQueue::operator[](unsigned long i) const
{
    for (cByteQueueNode* current = head; current; current = current->next)
    {
        if (i < current->CurrentSize())
            return (*current)[i];

        i -= current->CurrentSize();
    }

    // i should be less than CurrentSize(), therefore we should not be here
    assert(false);
    return 0;
}
