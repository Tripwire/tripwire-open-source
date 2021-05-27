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
// fspropset.h -- file system objects' property set objects
//
// cFSPropSet -- base class with all of the common properties
#ifndef __FSPROPSET_H
#define __FSPROPSET_H

#ifndef __FCOPROPSET_H
#include "fco/fcopropset.h"
#endif

#ifndef __DEBUG_H
#include "core/debug.h"
#endif
#ifndef __FCOPROPIMPL_H
#include "fco/fcopropimpl.h"
#endif
#ifndef __SIGNATURE_H
#include "fco/signature.h"
#endif
#ifndef __PROPSET_H
#include "fco/propset.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// cFCOPropFileType -- a property that represents a file type. Is is really just
//      an int32_t that overrides the AsString() method to display the file type
///////////////////////////////////////////////////////////////////////////////
class cFCOPropFileType : public cFCOPropInt32
{
public:
    virtual TSTRING AsString() const;
};

///////////////////////////////////////////////////////////////////////////////
// cFSPropSet
///////////////////////////////////////////////////////////////////////////////
class cFSPropSet : public iFCOPropSet
{
    DECLARE_TYPEDSERIALIZABLE()

public:
    // file system property enumeration..
    enum
    {
        PROP_FILETYPE = 0,
        PROP_DEV,
        PROP_RDEV,
        PROP_INODE,
        PROP_MODE,
        PROP_NLINK,
        PROP_UID,
        PROP_GID,
        PROP_SIZE,
        PROP_ATIME,
        PROP_MTIME,
        PROP_CTIME,
        PROP_BLOCK_SIZE,
        PROP_BLOCKS,
        PROP_GROWING_FILE, // a hack to detect growing files; this holds the same thing as PROP_SIZE
        PROP_CRC32,
        PROP_MD5,
        PROP_SHA,
        PROP_HAVAL,
        PROP_ACL,

        PROP_NUMITEMS
    };

    // the type of file this is
    enum FileType
    {
        FT_INVALID = 0,
        FT_FILE,
        FT_DIR,
        FT_BLOCKDEV,
        FT_CHARDEV,
        FT_SYMLINK,
        FT_FIFO,
        FT_SOCK,
        FT_DOOR,
        FT_PORT,
        FT_NAMED,
        FT_NATIVE,
        FT_MESSAGE_QUEUE,
        FT_SEMAPHORE,
        FT_SHARED_MEMORY,
        FT_NUMITEMS
    };


    cFSPropSet();
    cFSPropSet(const cFSPropSet& rhs);
    virtual ~cFSPropSet();

    const cFSPropSet& operator=(const cFSPropSet& rhs);

    virtual const cFCOPropVector& GetValidVector() const;
    virtual int                   GetNumProps() const;
    virtual int                   GetPropIndex(const TCHAR* name) const;
    virtual TSTRING               GetPropName(int index) const;
    virtual const iFCOProp*       GetPropAt(int index) const;
    virtual iFCOProp*             GetPropAt(int index);
    virtual void                  InvalidateProp(int index);
    virtual void                  InvalidateProps(const cFCOPropVector& propsToInvalidate);
    virtual void                  InvalidateAll();
    virtual void                  CopyProps(const iFCOPropSet* pSrc, const cFCOPropVector& propsToCopy);

    static int GetNumFSProps();
    // convenience function that returns the number of properties FSPropSets have without needing
    // to create one. This is useful in creating property vectors, since you need to know the size...

    // note that file type and normalized time are redundant properties (that is, they are derived from
    // the values of other properties) but their values are dependent on the file system the fsobject
    // came from, so they must be set during property calculation

    // Get/Set functions for all the properties
    PROPERTY(cFCOPropFileType, FileType, PROP_FILETYPE) // file type enumeration above
    PROPERTY(cFCOPropUint64, Dev, PROP_DEV)             //st_dev    -- the device number of the disk
    PROPERTY(cFCOPropUint64, RDev, PROP_RDEV)           //st_rdev   -- the device number of character or block file
    PROPERTY(cFCOPropUint64, Inode, PROP_INODE)         //st_ino    -- the indode number
    PROPERTY(cFCOPropUint64,
             Mode,
             PROP_MODE) //st_mode   -- the file's mode; also indicates whether it is a directory or a file
    PROPERTY(cFCOPropInt64, NLink, PROP_NLINK)                    //st_nlink  -- number of links to this file
    PROPERTY(cFCOPropInt64, UID, PROP_UID)                        //st_uid    -- uid who owns the file
    PROPERTY(cFCOPropInt64, GID, PROP_GID)                        //st_gid    -- gid who owns the file
    PROPERTY(cFCOPropInt64, Size, PROP_SIZE)                      //st_size   -- the size of the file
    PROPERTY(cFCOPropInt64, AccessTime, PROP_ATIME)               //st_atime  -- last access time
    PROPERTY(cFCOPropInt64, ModifyTime, PROP_MTIME)               //st_mtime  -- last modify time
    PROPERTY(cFCOPropInt64, CreateTime, PROP_CTIME)               //st_ctime  -- create time
    PROPERTY(cFCOPropInt64, BlockSize, PROP_BLOCK_SIZE)           //st_blksize
    PROPERTY(cFCOPropInt64, Blocks, PROP_BLOCKS)                  //st_blocks
    PROPERTY(cFCOPropGrowingFile, GrowingFile, PROP_GROWING_FILE) //growing file property; synonym for Size above.
    PROPERTY_OBJ(cCRC32Signature, CRC32, PROP_CRC32)
    PROPERTY_OBJ(cMD5Signature, MD5, PROP_MD5)
    PROPERTY_OBJ(cSHASignature, SHA, PROP_SHA)
    PROPERTY_OBJ(cHAVALSignature, HAVAL, PROP_HAVAL)
    //PROPERTY_OBJ(cUnixACL,        ACL,            PROP_ACL)  // will eventually be implememented

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

    // debugging method
    virtual void TraceContents(int dl = -1) const;

private:
    cFCOPropVector mValidProps;     // all the properties that have been evaluated
    cFCOPropVector mUndefinedProps; // properties that have been measured but have undefined values
};

#endif
