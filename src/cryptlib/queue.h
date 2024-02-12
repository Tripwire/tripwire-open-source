// specification file for an unlimited queue for storing bytes

#ifndef QUEUE_H
#define QUEUE_H

#include "cryptlib.h"

// The queue is implemented as a linked list of arrays, but you don't need to
// know about that.  So just ignore this next line. :)
class ByteQueueNode;

class ByteQueue : public BufferedTransformation
{
public:
    ByteQueue(unsigned int nodeSize=256);
    ByteQueue(const ByteQueue &copy);
    ~ByteQueue();

    // how many bytes currently stored
    unsigned long CurrentSize() const;
    unsigned long MaxRetrieveable()
        {return CurrentSize();}

    void Put(ibyte inByte);
    void Put(const ibyte *inString, unsigned int length);

    // both functions returns the number of bytes actually retrived
    unsigned int Get(ibyte &outByte);
    unsigned int Get(ibyte *outString, unsigned int getMax);

    unsigned int Peek(ibyte &outByte) const;

    void CopyTo(BufferedTransformation &target) const;
    void CopyTo(ibyte *target) const;

    ByteQueue & operator=(const ByteQueue &rhs);
    bool operator==(const ByteQueue &rhs) const;
    ibyte operator[](unsigned long i) const;

private:
    void CopyFrom(const ByteQueue &copy);
    void Destroy();

    unsigned int nodeSize;
    ByteQueueNode *head, *tail;
};

#endif
