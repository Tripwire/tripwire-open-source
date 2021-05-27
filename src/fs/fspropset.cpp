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
// fspropset.cpp

#include "stdfs.h"

#include "fspropset.h"

#include "core/debug.h"
#include "core/serializer.h"
#include "fco/fcoundefprop.h"
#include "core/errorutil.h"

// string table stuff
#include "fsstrings.h"

//#############################################################################
// cFCOPropFileType
//#############################################################################
TSTRING cFCOPropFileType::AsString() const
{
    static int fileTypes[] = {fs::STR_FT_INVALID,
                              fs::STR_FT_FILE,
                              fs::STR_FT_DIR,
                              fs::STR_FT_BLOCKDEV,
                              fs::STR_FT_CHARDEV,
                              fs::STR_FT_SYMLINK,
                              fs::STR_FT_FIFO,
                              fs::STR_FT_SOCK,
                              fs::STR_FT_DOOR,
                              fs::STR_FT_PORT,
                              fs::STR_FT_NAMED,
                              fs::STR_FT_NATIVE,
                              fs::STR_FT_MESSAGE_QUEUE,
                              fs::STR_FT_SEMAPHORE,
                              fs::STR_FT_SHARED_MEMORY};

    int32_t fileType = GetValue();
    if ((fileType > cFSPropSet::FT_INVALID) && (fileType < cFSPropSet::FT_NUMITEMS))
        return TSS_GetString(cFS, fileTypes[fileType]);
    else
        return TSS_GetString(cFS, fileTypes[cFSPropSet::FT_INVALID]);
}


//#############################################################################
// cFSPropSet
//#############################################################################

IMPLEMENT_TYPEDSERIALIZABLE(cFSPropSet, _T("cFSPropSet"), 0, 1);

///////////////////////////////////////////////////////////////////////////////
// the names of all the properties
// TODO -- put these in a class-static hash table of something of the like so that
//      name lookups are constant time -- mdb
///////////////////////////////////////////////////////////////////////////////
static int cFSPropSet_PropNames[] = {
    fs::STR_PROP_FILETYPE, fs::STR_PROP_DEV,   fs::STR_PROP_RDEV,       fs::STR_PROP_INODE,  fs::STR_PROP_MODE,
    fs::STR_PROP_NLINK,    fs::STR_PROP_UID,   fs::STR_PROP_GID,        fs::STR_PROP_SIZE,   fs::STR_PROP_ATIME,
    fs::STR_PROP_MTIME,    fs::STR_PROP_CTIME, fs::STR_PROP_BLOCK_SIZE, fs::STR_PROP_BLOCKS, fs::STR_PROP_GROWING_FILE,
    fs::STR_PROP_CRC32,    fs::STR_PROP_MD5,   fs::STR_PROP_SHA,        fs::STR_PROP_HAVAL,  fs::STR_PROP_ACL};

///////////////////////////////////////////////////////////////////////////////
// TraceContents
///////////////////////////////////////////////////////////////////////////////
void cFSPropSet::TraceContents(int dl) const
{
    if (dl < 0)
        dl = cDebug::D_DEBUG;

    cDebug d("cFSPropSet::TraceContents");

    TOSTRINGSTREAM ostr;
    ostr << _T("File Sysytem Prop Set: ");
    for (int i = 0; i < GetNumProps(); i++)
    {
        if (mValidProps.ContainsItem(i))
        {
            ostr << _T("[") << i << _T("]") << GetPropName(i) << _T(" = ") << GetPropAt(i)->AsString().c_str()
                 << _T(", ");
        }
    }

    tss_mkstr(out, ostr);
    d.Trace(dl, _T("%s\n"), out.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// CopyProps
///////////////////////////////////////////////////////////////////////////////
void cFSPropSet::CopyProps(const iFCOPropSet* pSrc, const cFCOPropVector& propsToCopy)
{
    ASSERT(pSrc->GetType() == CLASS_TYPE(cFSPropSet));

    // first, modify my valid vector...
    mValidProps |= propsToCopy;

    for (int i = 0; i < GetNumFSProps(); i++)
    {
        if (propsToCopy.ContainsItem(i))
        {
            ASSERT(pSrc->GetValidVector().ContainsItem(i));
            const iFCOProp* pProp = pSrc->GetPropAt(i);

            if (pProp->GetType() != cFCOUndefinedProp::GetInstance()->GetType())
            {
                GetPropAt(i)->Copy(pProp);
                mUndefinedProps.RemoveItem(i);
            }
            else
                mUndefinedProps.AddItem(i);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// Invalidate
///////////////////////////////////////////////////////////////////////////////
void cFSPropSet::InvalidateProp(int index)
{
    ASSERT((index >= 0) && (index < GetNumProps()));
    mValidProps.RemoveItem(index);
}

void cFSPropSet::InvalidateAll()
{
    mValidProps.Clear();
}

void cFSPropSet::InvalidateProps(const cFCOPropVector& propsToInvalidate)
{
    cFCOPropVector inBoth = mValidProps;
    inBoth &= propsToInvalidate;
    mValidProps ^= inBoth;
}

///////////////////////////////////////////////////////////////////////////////
// GetNumFSProps
///////////////////////////////////////////////////////////////////////////////
int cFSPropSet::GetNumFSProps()
{
    return PROP_NUMITEMS;
}

///////////////////////////////////////////////////////////////////////////////
// ctors. dtor, operator=
///////////////////////////////////////////////////////////////////////////////
cFSPropSet::cFSPropSet()
    : iFCOPropSet(), mValidProps(cFSPropSet::PROP_NUMITEMS), mUndefinedProps(cFSPropSet::PROP_NUMITEMS)
{
    // TODO: do I want to zero out all the property values here?
}

cFSPropSet::~cFSPropSet()
{
}

cFSPropSet::cFSPropSet(const cFSPropSet& rhs) : iFCOPropSet(), mValidProps(cFSPropSet::PROP_NUMITEMS)
{
    *this = rhs;
}

const cFSPropSet& cFSPropSet::operator=(const cFSPropSet& rhs)
{
    mValidProps     = rhs.GetValidVector();
    mUndefinedProps = rhs.mUndefinedProps;

    for (int i = 0; i < PROP_NUMITEMS; i++)
    {
        if (mValidProps.ContainsItem(i) && !mUndefinedProps.ContainsItem(i))
        {
            GetPropAt(i)->Copy(((cFSPropSet&)rhs).GetPropAt(i));
            // call non-const GetPropAt for rhs
            // don't want it to assert ContainsItem
        }
    }

    return *this;
}

const cFCOPropVector& cFSPropSet::GetValidVector() const
{
    return mValidProps;
}

int cFSPropSet::GetNumProps() const
{
    return PROP_NUMITEMS;
}

int cFSPropSet::GetPropIndex(const TCHAR* name) const
{
    for (int i = 0; i < PROP_NUMITEMS; i++)
    {
        if (_tcscmp(name, TSS_GetString(cFS, cFSPropSet_PropNames[i]).c_str()) == 0)
            return i;
    }
    return iFCOPropSet::PROP_NOT_FOUND;
}

TSTRING cFSPropSet::GetPropName(int index) const
{
    ASSERT((index >= 0) && (index < GetNumProps()));
    return TSS_GetString(cFS, cFSPropSet_PropNames[index]);
}

const iFCOProp* cFSPropSet::GetPropAt(int index) const
{
    // the specified property had better have a valid value...
    ASSERT((index >= 0) && (index < GetNumProps()));
    ASSERT(mValidProps.ContainsItem(index));

    if (mUndefinedProps.ContainsItem(index))
    {
        return cFCOUndefinedProp::GetInstance();
    }

    switch (index)
    {
    case PROP_FILETYPE:
        return &mFileType;
    case PROP_DEV:
        return &mDev;
    case PROP_RDEV:
        return &mRDev;
    case PROP_INODE:
        return &mInode;
    case PROP_MODE:
        return &mMode;
    case PROP_NLINK:
        return &mNLink;
    case PROP_UID:
        return &mUID;
    case PROP_GID:
        return &mGID;
    case PROP_SIZE:
        return &mSize;
    case PROP_ATIME:
        return &mAccessTime;
    case PROP_MTIME:
        return &mModifyTime;
    case PROP_CTIME:
        return &mCreateTime;
    case PROP_BLOCK_SIZE:
        return &mBlockSize;
    case PROP_BLOCKS:
        return &mBlocks;
    case PROP_GROWING_FILE:
        return &mGrowingFile;
    case PROP_CRC32:
        return &mCRC32;
    case PROP_MD5:
        return &mMD5;
    case PROP_SHA:
        return &mSHA;
    case PROP_HAVAL:
        return &mHAVAL;
    case PROP_ACL:
        ASSERT(false); // unimplemented
        return NULL;
    default:
    {
        // bad parameter passed to GetPropAt
        ASSERT(false);
    }
    }
    return NULL;
}

// TODO -- it sucks duplicating code like this! -- mdb
iFCOProp* cFSPropSet::GetPropAt(int index)
{
    // the specified property had better have a valid value...
    ASSERT((index >= 0) && (index < GetNumProps()));
    // don't assert for non-const GetPropAt() because we might want to get a non-valid
    // property for copying
    //ASSERT(mValidProps.ContainsItem(index));

    if (mUndefinedProps.ContainsItem(index))
    {
        return cFCOUndefinedProp::GetInstance();
    }

    switch (index)
    {
    case PROP_FILETYPE:
        return &mFileType;
    case PROP_DEV:
        return &mDev;
    case PROP_RDEV:
        return &mRDev;
    case PROP_INODE:
        return &mInode;
    case PROP_MODE:
        return &mMode;
    case PROP_NLINK:
        return &mNLink;
    case PROP_UID:
        return &mUID;
    case PROP_GID:
        return &mGID;
    case PROP_SIZE:
        return &mSize;
    case PROP_ATIME:
        return &mAccessTime;
    case PROP_MTIME:
        return &mModifyTime;
    case PROP_CTIME:
        return &mCreateTime;
    case PROP_BLOCK_SIZE:
        return &mBlockSize;
    case PROP_BLOCKS:
        return &mBlocks;
    case PROP_GROWING_FILE:
        return &mGrowingFile;
    case PROP_CRC32:
        return &mCRC32;
    case PROP_MD5:
        return &mMD5;
    case PROP_SHA:
        return &mSHA;
    case PROP_HAVAL:
        return &mHAVAL;
    case PROP_ACL:
        ASSERT(false); // unimplemented
        return NULL;
    default:
    {
        // bad parameter passed to GetPropAt
        ASSERT(false);
    }
    }
    return NULL;
}

void cFSPropSet::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("FS Property Set Read")));

    mValidProps.Read(pSerializer);
    mUndefinedProps.Read(pSerializer);

    for (int i = 0; i < PROP_NUMITEMS; i++)
    {
        if (mValidProps.ContainsItem(i) && !mUndefinedProps.ContainsItem(i))
            GetPropAt(i)->Read(pSerializer);
    }
}

void cFSPropSet::Write(iSerializer* pSerializer) const
{
    mValidProps.Write(pSerializer);
    mUndefinedProps.Write(pSerializer);

    for (int i = 0; i < PROP_NUMITEMS; i++)
    {
        if (mValidProps.ContainsItem(i) && !mUndefinedProps.ContainsItem(i))
            GetPropAt(i)->Write(pSerializer);
    }
}
