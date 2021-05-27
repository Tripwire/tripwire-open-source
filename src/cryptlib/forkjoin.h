#ifndef FORKJOIN_H
#define FORKJOIN_H

#include "cryptlib.h"
#include "filters.h"
#include "queue.h"

class Fork : public BufferedTransformation
{
public:
    Fork(int number_of_outports, BufferedTransformation *const *outports = NULL);

    void SelectOutPort(int portNumber);

    bool Attachable() {return true;}
    void Detach(BufferedTransformation *newOut = NULL);
    void Attach(BufferedTransformation *newOut);
    virtual void Close();

    unsigned long MaxRetrieveable()
        {return outPorts[currentPort]->MaxRetrieveable();}
//    virtual void InputFinished()
//        {outPorts[currentPort]->InputFinished();}

    unsigned int Get(uint8_t &outByte)
        {return outPorts[currentPort]->Get(outByte);}
    unsigned int Get(uint8_t *outString, unsigned int getMax)
        {return outPorts[currentPort]->Get(outString, getMax);}
    unsigned int Peek(uint8_t &outByte) const
        {return outPorts[currentPort]->Peek(outByte);}

    virtual void Put(uint8_t inByte);
    virtual void Put(const uint8_t *inString, unsigned int length);

protected:
    unsigned int NumberOfPorts() const {return numberOfPorts;}
    BufferedTransformation& AccessPort(unsigned int i) {return *outPorts[i];}

private:
    Fork(const Fork &); // no copying allowed

    unsigned int numberOfPorts, currentPort;
    vector_member_ptrs<BufferedTransformation> outPorts;
};

class Join;

class Interface : public BufferedTransformation
{
public:
    Interface(Join &p, ByteQueue &b, int i)
        : parent(p), bq(b), id(i) {}

    unsigned long MaxRetrieveable();
    void Close();
    bool Attachable() {return true;}
    void Detach(BufferedTransformation *bt);
    void Attach(BufferedTransformation *bt);

    void Put(uint8_t inByte);
    void Put(const uint8_t *inString, unsigned int length);
    unsigned int Get(uint8_t &outByte);
    unsigned int Get(uint8_t *outString, unsigned int getMax);
    unsigned int Peek(uint8_t &outByte) const;

private:
    Join &parent;
    ByteQueue &bq;
    const int id;
};

class Join : public Filter
{
public:
    Join(unsigned int number_of_inports, BufferedTransformation *outQ = NULL);

    // Note that ReleaseInterface is similar but not completely compatible 
    // with SelectInterface of version 2.0.  ReleaseInterface can be called
    // only once for each interface, and if an interface is released,
    // the caller will be responsible for deleting it.
    Interface *ReleaseInterface(unsigned int i);

    virtual void NotifyInput(unsigned int interfaceId, unsigned int length);
    virtual void NotifyClose(unsigned int interfaceId);

    void Put(uint8_t inByte) {outQueue->Put(inByte);}
    void Put(const uint8_t *inString, unsigned int length)
        {outQueue->Put(inString, length);}

protected:
    unsigned int NumberOfPorts() const {return numberOfPorts;}
    ByteQueue& AccessPort(unsigned int i) {return *inPorts[i];}
    unsigned int InterfacesOpen() const {return interfacesOpen;}

private:
    Join(const Join &); // no copying allowed

    unsigned int numberOfPorts;
    vector_member_ptrs<ByteQueue> inPorts;
    unsigned int interfacesOpen;
    vector_member_ptrs<Interface> interfaces;
};

#endif
