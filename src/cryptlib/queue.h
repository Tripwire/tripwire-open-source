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

    void Put(uint8_t inByte);
    void Put(const uint8_t *inString, unsigned int length);

    // both functions returns the number of bytes actually retrived
    unsigned int Get(uint8_t &outByte);
    unsigned int Get(uint8_t *outString, unsigned int getMax);

    unsigned int Peek(uint8_t &outByte) const;

    void CopyTo(BufferedTransformation &target) const;
    void CopyTo(uint8_t *target) const;

    ByteQueue & operator=(const ByteQueue &rhs);
    bool operator==(const ByteQueue &rhs) const;
    uint8_t operator[](unsigned long i) const;

private:
    void CopyFrom(const ByteQueue &copy);
    void Destroy();

    unsigned int nodeSize;
    ByteQueueNode *head, *tail;
};

#endif
