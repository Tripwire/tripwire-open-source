// queue.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "queue.h"
#include "misc.h"

// this class for use by ByteQueue only
class ByteQueueNode
{
public:
    ByteQueueNode(unsigned int maxSize);

    unsigned int CurrentSize() const
        {return tail-head;}
    unsigned int UsedUp() const
        {return (head==MaxSize());}

    unsigned int Put(uint8_t inByte);
    unsigned int Put(const uint8_t *inString, unsigned int length);

    unsigned int Get(uint8_t &outByte);
    unsigned int Get(uint8_t *outString, unsigned int getMax);

    unsigned int Peek(uint8_t &outByte) const;

    void CopyTo(BufferedTransformation &target) const
        {target.Put(buf+head, tail-head);}
    void CopyTo(uint8_t *target) const
        {memcpy(target, buf+head, tail-head);}

    uint8_t operator[](unsigned int i) const
        {return buf[i-head];}

    ByteQueueNode *next;

private:
    unsigned int MaxSize() const {return buf.size;}

    SecByteBlock buf;
    unsigned int head, tail;
};


ByteQueueNode::ByteQueueNode(unsigned int maxSize)
    : buf(maxSize)
{
    head = tail = 0;
    next = 0;
}

unsigned int ByteQueueNode::Put(uint8_t inByte)
{
    if (MaxSize()==tail)
        return 0;

    buf[tail++]=inByte;
    return 1;
}

unsigned int ByteQueueNode::Put(const uint8_t *inString, unsigned int length)
{
    unsigned int l = STDMIN(length, MaxSize()-tail);
    memcpy(buf+tail, inString, l);
    tail += l;
    return l;
}

unsigned int ByteQueueNode::Get(uint8_t &outByte)
{
    if (tail==head)
        return 0;

    outByte=buf[head++];
    return 1;
}

unsigned int ByteQueueNode::Get(uint8_t *outString, unsigned int getMax)
{
    unsigned int l = STDMIN(getMax, tail-head);
    memcpy(outString, buf+head, l);
    head += l;
    return l;
}

unsigned int ByteQueueNode::Peek(uint8_t &outByte) const
{
    if (tail==head)
        return 0;

    outByte=buf[head];
    return 1;
}

// ********************************************************

ByteQueue::ByteQueue(unsigned int nodeSize)
    : nodeSize(nodeSize)
{
    head = tail = new ByteQueueNode(nodeSize);
}

ByteQueue::ByteQueue(const ByteQueue &copy)
{
    CopyFrom(copy);
}

void ByteQueue::CopyFrom(const ByteQueue &copy)
{
    nodeSize = copy.nodeSize;
    head = tail = new ByteQueueNode(*copy.head);

    for (ByteQueueNode *current=copy.head->next; current; current=current->next)
    {
        tail->next = new ByteQueueNode(*current);
        tail = tail->next;
    }

    tail->next = NULL;
}

ByteQueue::~ByteQueue()
{
    Destroy();
}

void ByteQueue::Destroy()
{
    ByteQueueNode *next;

    for (ByteQueueNode *current=head; current; current=next)
    {
        next=current->next;
        delete current;
    }
}

void ByteQueue::CopyTo(BufferedTransformation &target) const
{
    for (ByteQueueNode *current=head; current; current=current->next)
        current->CopyTo(target);
}

void ByteQueue::CopyTo(uint8_t *target) const
{
    for (ByteQueueNode *current=head; current; current=current->next)
    {
        current->CopyTo(target);
        target += current->CurrentSize();
    }
}

unsigned long ByteQueue::CurrentSize() const
{
    unsigned long size=0;

    for (ByteQueueNode *current=head; current; current=current->next)
        size += current->CurrentSize();

    return size;
}

void ByteQueue::Put(uint8_t inByte)
{
    if (!tail->Put(inByte))
    {
        tail->next = new ByteQueueNode(nodeSize);
        tail = tail->next;
        tail->Put(inByte);
    }
}

void ByteQueue::Put(const uint8_t *inString, unsigned int length)
{
    unsigned int l;

    while ((l=tail->Put(inString, length)) < length)
    {
        tail->next = new ByteQueueNode(nodeSize);
        tail = tail->next;
        inString += l;
        length -= l;
    }
}

unsigned int ByteQueue::Get(uint8_t &outByte)
{
    int l = head->Get(outByte);
    if (head->UsedUp())
    {
        ByteQueueNode *temp=head;
        head = head->next;
        delete temp;
        if (!head)  // just deleted the last node
            head = tail = new ByteQueueNode(nodeSize);
    }
    return l;
}

unsigned int ByteQueue::Get(uint8_t *outString, unsigned int getMax)
{
    unsigned int getMaxSave=getMax;
    ByteQueueNode *current=head;

    while (getMax && current)
    {
        int l=current->Get(outString, getMax);

        outString += l;
        getMax -= l;

        current = current->next;
    }

    while (head && head->UsedUp())
    {
        current=head;
        head=head->next;
        delete current;
    }

    if (!head)  // every single node has been used up and deleted
        head = tail = new ByteQueueNode(nodeSize);

    return (getMaxSave-getMax);
}

unsigned int ByteQueue::Peek(uint8_t &outByte) const
{
    return head->Peek(outByte);
}

ByteQueue & ByteQueue::operator=(const ByteQueue &rhs)
{
    Destroy();
    CopyFrom(rhs);
    return *this;
}

bool ByteQueue::operator==(const ByteQueue &rhs) const
{
    const unsigned long currentSize = CurrentSize();

    if (currentSize != rhs.CurrentSize())
        return false;

    for (unsigned long i = 0; i<currentSize; i++)
        if ((*this)[i] != rhs[i])
            return false;

    return true;
}

uint8_t ByteQueue::operator[](unsigned long i) const
{
    for (ByteQueueNode *current=head; current; current=current->next)
    {
        if (i < current->CurrentSize())
            return (*current)[i];
        
        i -= current->CurrentSize();
    }

    // i should be less than CurrentSize(), therefore we should not be here
    assert(false);
    return 0;
}
