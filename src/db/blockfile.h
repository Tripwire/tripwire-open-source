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
// blockfile.h
//
// cBlockFile --
#ifndef __BLOCKFILE_H
#define __BLOCKFILE_H

#ifndef __TYPES_H
#include "types.h"
#endif
#ifndef __TCHAR_H
#include "core/tchar.h"
#endif
#ifndef __BLOCK_H
#include "block.h"
#endif

#ifdef DEBUG
#    undef _BLOCKFILE_DEBUG  // If we dont do this, test does not compile on unix
#    define _BLOCKFILE_DEBUG // If we dont do this, test does not compile on unix
#endif

class cBlockFile_i;
class eArchive;
class cBidirArchive;
//-----------------------------------------------------------------------------
// cBlockFile
//-----------------------------------------------------------------------------
class cBlockFile
{
public:
    enum
    {
        BLOCK_SIZE = 4096
    };
    typedef cBlock<BLOCK_SIZE> Block;

    cBlockFile();
    ~cBlockFile();

    void Open(const TSTRING& fileName, int numPages, bool bTruncate = false); //throw (eArchive)
        // opens the given file name as a block file, and uses numPages to signify the
        // number of pages to cache blocks in as they are accessed from disk.
        // if bTruncate is true, then the file is created with zero length.
    void Open(cBidirArchive* pArch, int numPages); //throw (eArchive)
        // the same as the previous Open(), except the passed in archive is used. This class will destroy
        // the archive when it is done (when Close() is called). If the archive length is zero, then
        // we create a new database. Otherwise, we assume we are opening an existing file.
    void Close(); //throw (eArchive)
        // writes all unsaved data to disk and disassociates the block file from its archive
    void Flush(); //throw (eArchive)
        // flushes all dirty data to disk

    Block* GetBlock(int blockNum); //throw (eArchive)
        // returns the specified block. This asserts that the block number is below GetNumBlocks()
        // this method takes care of paging the block into memory and returns a pointer to the block.

    Block* CreateBlock(); //throw (eArchive)
        // this creates a new block, sets its number to GetNumBlocks(), pages it into memory, and
        // returns a pointer to it

    void DestroyLastBlock();
    // this method removes the last block from the file

    int GetNumBlocks() const
    {
        return mNumBlocks;
    }
    int GetBlockSize() const
    {
        return BLOCK_SIZE;
    }

    cBidirArchive* GetArchive()
    {
        return mpArchive;
    }
    // NOTE -- be _very_ careful with this archive. It should probably not be written to
private:
    typedef cBlockImpl<BLOCK_SIZE> BlockImpl;
    typedef std::vector<BlockImpl> BlockVector;

    int            mNumPages;
    int            mNumBlocks; // the total number of blocks in the archive.
    uint32_t       mTimer;     // keeps track of the current "time"
    cBidirArchive* mpArchive;  // note: I always own the deletion of the archive
    BlockVector    mvPagedBlocks;

    void FlushBlock(BlockImpl* pBlock); //throw (eArchive)
                                        // helper function that writes a block to disk if it is dirty

    ///////////////////////////////////////////////////////////////////////////
    // Profiling / Debugging interface
    ///////////////////////////////////////////////////////////////////////////
#ifdef _BLOCKFILE_DEBUG
public:
    void TraceContents(int dl = -1) const;
    // traces out all the info we can on the current state of this class
    // dl is the debug level to trace it at; -1 means to use D_DEBUG
    void AssertValid() const;
    // ASSERTs as much as we can about the consistancy of our internal state.
    int mNumBlockWrite;
    // counts how many writes we have done
    int mNumBlockRead;
    // counts how many reads we have done
    int mNumPageFault;
    // number of times a page fault occured
    int mNumPageRequests;
    // number of page requests (useful to compare with mNumPageFault)

#endif //_BLOCKFILE_DEBUG
};


//#############################################################################
// inline implementation
//#############################################################################
inline void cBlockFile::FlushBlock(cBlockFile::BlockImpl* pBlock) //throw (eArchive)
{

    if ((pBlock->GetBlockNum() != Block::INVALID_NUM) && pBlock->IsDirty())
    {
        // mNumBlockWrite keeps track of how many block writes we do
        //
#ifdef _BLOCKFILE_DEBUG
        mNumBlockWrite++;
#endif

        pBlock->Write(*mpArchive);
    }
}


#endif //__BLOCKFILE_H
