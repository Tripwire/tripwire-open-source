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
// bytequeue.h
//
// cByteQueue -- specification file for an unlimited queue for storing bytes
//
// this is a superior implementation of the byte queue supplied with the crypto++ lib
#ifndef __BYTEQUEUE_H
#define __BYTEQUEUE_H

#include "cryptlib/cryptlib.h"

class cByteQueueNode;

class cByteQueue : public BufferedTransformation
{
public:
    explicit cByteQueue(int nodeSize = 1024);
    cByteQueue(const cByteQueue& copy);
    ~cByteQueue();

    // how many bytes currently stored
    unsigned long CurrentSize() const;
    unsigned long MaxRetrieveable()
    {
        return CurrentSize();
    }

    void Put(uint8_t inByte);
    void Put(const uint8_t* inString, unsigned int length);

    // both functions returns the number of bytes actually retrived
    unsigned int Get(uint8_t& outByte);
    unsigned int Get(uint8_t* outString, unsigned int getMax);

    unsigned int Peek(uint8_t& outByte) const;

    void CopyTo(BufferedTransformation& target) const;
    void CopyTo(uint8_t* target) const;

    cByteQueue& operator=(const cByteQueue& rhs);
    bool        operator==(const cByteQueue& rhs) const;
    uint8_t        operator[](unsigned long i) const;

private:
    void CopyFrom(const cByteQueue& copy);
    void Destroy();

    int             mNodeSize;
    int             mCurrentSize;
    cByteQueueNode *head, *tail;
};

#endif //__BYTEQUEUE_H
