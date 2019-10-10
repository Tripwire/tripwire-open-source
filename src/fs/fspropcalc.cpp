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
// fspropcalc.cpp

#include "stdfs.h"
#include "core/debug.h"
#include "core/errorbucket.h"
#include "core/errorbucket.h"
#include "fco/fconame.h"
#include "fco/fconametranslator.h"
#include "fco/twfactory.h"
#include "fspropcalc.h"
#include "fsobject.h"

#include <unistd.h>
#include <errno.h>

cFSPropCalc::cFSPropCalc() : mCollAction(iFCOPropCalc::PROP_LEAVE), mCalcFlags(0), mpErrorBucket(0)
{
}

cFSPropCalc::~cFSPropCalc()
{
}

///////////////////////////////////////////////////////////////////////////////
// NeedsStat -- returns true if any properties are in the vector that require
//      a stat() call
///////////////////////////////////////////////////////////////////////////////
static bool NeedsStat(const cFCOPropVector& v)
{
    return (v.ContainsItem(cFSPropSet::PROP_DEV) || v.ContainsItem(cFSPropSet::PROP_RDEV) ||
            v.ContainsItem(cFSPropSet::PROP_INODE) || v.ContainsItem(cFSPropSet::PROP_MODE) ||
            v.ContainsItem(cFSPropSet::PROP_NLINK) || v.ContainsItem(cFSPropSet::PROP_UID) ||
            v.ContainsItem(cFSPropSet::PROP_GID) || v.ContainsItem(cFSPropSet::PROP_SIZE) ||
            v.ContainsItem(cFSPropSet::PROP_ATIME) || v.ContainsItem(cFSPropSet::PROP_MTIME) ||
            v.ContainsItem(cFSPropSet::PROP_CTIME) || v.ContainsItem(cFSPropSet::PROP_BLOCK_SIZE) ||
            v.ContainsItem(cFSPropSet::PROP_BLOCKS) || v.ContainsItem(cFSPropSet::PROP_FILETYPE) ||
            v.ContainsItem(cFSPropSet::PROP_GROWING_FILE));
}

///////////////////////////////////////////////////////////////////////////////
// GetSymLinkStr -- returns a string that represents what a symbolic link is
//      pointing at. If this returns false, call iFSServices::GetErrnoString()
//      to get the error message.
///////////////////////////////////////////////////////////////////////////////


bool cFSPropCalc::GetSymLinkStr(const TSTRING& strName, cArchive& arch, size_t size)
{
#if SUPPORTS_SYMLINKS
    std::vector<char> data(size + 1);
    char*             buf = &data[0];

#if defined(O_PATH) // A Linuxism that lets us read symlinks w/o bumping the access time.
    int fd  = open(strName.c_str(), (O_PATH | O_NOFOLLOW | O_NOATIME));
    int rtn = readlinkat(fd, 0, buf, size);
    close(fd);
#else
    int rtn = readlink(strName.c_str(), buf, size);
#endif

    if (rtn < 0)
    {
        // Some OSes (like HP-UX) return ERANGE if buffer is too small.
        // This is nonstandard but better than the usual truncate-and-say-you-succeeded
        //
        if (ERANGE == errno)
            return GetSymLinkStr(strName, arch, size * 2);

        return false;
    }

    //Sadly if buf isn't big enough readlink 'succeeds' by truncating the string, so the only
    // clue your buffer might be too small is if you maxed it out.  So we try again, within reason.
#if READLINK_NULL_TERMINATES
    if ((size_t)rtn >= size - 1) //SkyOS wants space to null terminate the string it hands back, which is nice, I guess.
#else
    if ((size_t)rtn == size)
#endif
    {
        if (size < 128 * TW_PATH_SIZE)
            return GetSymLinkStr(strName, arch, size * 2);

        return false;
    }

    // the return value is the number of characters written.
    arch.WriteBlob(buf, rtn);

    return true;
#else
    return false;
#endif
}

void cFSPropCalc::AddPropCalcError(const eError& e)
{
    if (mpErrorBucket)
        mpErrorBucket->AddError(e);
}

bool cFSPropCalc::DoStat(const TSTRING& strName, cFSStatArgs& statArgs)
{
    cDebug d("cFSPropCalc::DoStat");

    try
    {
        d.TraceDetail("---Performing Stat()\n");
        iFSServices::GetInstance()->Stat(strName, statArgs);
    }
    catch (eError& e)
    {
        d.TraceError("Error getting stat info for %s : %s\n", strName.c_str(), e.GetMsg().c_str());
        e.SetFatality(false);
        AddPropCalcError(e);
        return false;
    }
    catch (std::exception& e)
    {
        d.TraceError("Error getting stat info for %s : %s\n", strName.c_str(), e.what());
        AddPropCalcError(eFSPropCalc(strName, e.what(), eError::NON_FATAL));
        return false;
    }
    catch (...)
    {
        d.TraceError("Unknown error getting stat info for %s\n", strName.c_str());
        AddPropCalcError(eFSPropCalc(strName, "unknown", eError::NON_FATAL));
        return false;
    }

    return true;
}

bool cFSPropCalc::DoOpen(const TSTRING& strName, cFileArchive& arch)
{
    try
    {
        arch.OpenRead(strName.c_str(),
                      ((mCalcFlags & iFCOPropCalc::DIRECT_IO) ? cFileArchive::FA_SCANNING | cFileArchive::FA_DIRECT :
                                                                cFileArchive::FA_SCANNING));
    }
    catch (eError&)
    {
        AddPropCalcError(eArchiveOpen(strName, iFSServices::GetInstance()->GetErrString(), eError::NON_FATAL));
        return false;
    }
    catch (std::exception& e)
    {
        AddPropCalcError(eArchiveOpen(strName, e.what(), eError::NON_FATAL));
        return false;
    }
    catch (...)
    {
        AddPropCalcError(eArchiveOpen(strName, "unknown", eError::NON_FATAL));
        return false;
    }

    return true;
}

bool cFSPropCalc::DoHash(const TSTRING& strName, cBidirArchive* pTheArch, cArchiveSigGen& asg, cFileArchive& arch)
{
    cDebug d("cFSPropCalc::DoHash");
    try
    {
        pTheArch->Seek(0, cBidirArchive::BEGINNING);
        asg.CalculateSignatures(*pTheArch);
        arch.Close();
    }
    catch (eError& e)
    {
        d.TraceError("Error generating hashes for %s : %s\n", strName.c_str(), e.GetMsg().c_str());
        e.SetFatality(false);
        AddPropCalcError(e);
        return false;
    }
    catch (std::exception& e)
    {
        d.TraceError("Error generating hashes for %s : %s\n", strName.c_str(), e.what());
        AddPropCalcError(eArchiveRead(strName, e.what(), eError::NON_FATAL));
        return false;
    }
    catch (...)
    {
        d.TraceError("Unknown error generating hashes for %s\n", strName.c_str());
        AddPropCalcError(eArchiveRead(strName, "unknown", eError::NON_FATAL));
        return false;
    }

    return true;
}

void cFSPropCalc::HandleStatProperties(const cFCOPropVector& propsToCheck, const cFSStatArgs& ss, cFSPropSet& propSet)
{
    if (propsToCheck.ContainsItem(cFSPropSet::PROP_DEV))
        propSet.SetDev(ss.dev);

    if (propsToCheck.ContainsItem(cFSPropSet::PROP_RDEV))
        propSet.SetRDev(ss.rdev);

    if (propsToCheck.ContainsItem(cFSPropSet::PROP_INODE))
        propSet.SetInode(ss.ino);

    if (propsToCheck.ContainsItem(cFSPropSet::PROP_MODE))
        propSet.SetMode(ss.mode);

    if (propsToCheck.ContainsItem(cFSPropSet::PROP_NLINK))
        propSet.SetNLink(ss.nlink);

    if (propsToCheck.ContainsItem(cFSPropSet::PROP_UID))
        propSet.SetUID(ss.uid);

    if (propsToCheck.ContainsItem(cFSPropSet::PROP_GID))
        propSet.SetGID(ss.gid);

    if (propsToCheck.ContainsItem(cFSPropSet::PROP_SIZE))
        propSet.SetSize(ss.size);

    if (propsToCheck.ContainsItem(cFSPropSet::PROP_ATIME))
        propSet.SetAccessTime(ss.atime);

    if (propsToCheck.ContainsItem(cFSPropSet::PROP_MTIME))
        propSet.SetModifyTime(ss.mtime);

    if (propsToCheck.ContainsItem(cFSPropSet::PROP_CTIME))
        propSet.SetCreateTime(ss.ctime);

    if (propsToCheck.ContainsItem(cFSPropSet::PROP_BLOCK_SIZE))
        propSet.SetBlockSize(ss.blksize);

    if (propsToCheck.ContainsItem(cFSPropSet::PROP_BLOCKS))
        propSet.SetBlocks(ss.blocks);

    if (propsToCheck.ContainsItem(cFSPropSet::PROP_GROWING_FILE))
        propSet.SetGrowingFile(ss.size);

    if (propsToCheck.ContainsItem(cFSPropSet::PROP_FILETYPE))
    {
        // TODO -- It _really_ bites duplicating code here and in fsdatasource.cpp
        // *** This _has_ to be remedied somehow!
        // set the file type
        switch (ss.mFileType)
        {
        case cFSStatArgs::TY_FILE:
            propSet.SetFileType(cFSPropSet::FT_FILE);
            break;
        case cFSStatArgs::TY_DIR:
            propSet.SetFileType(cFSPropSet::FT_DIR);
            break;
        case cFSStatArgs::TY_BLOCKDEV:
            propSet.SetFileType(cFSPropSet::FT_BLOCKDEV);
            break;
        case cFSStatArgs::TY_CHARDEV:
            propSet.SetFileType(cFSPropSet::FT_CHARDEV);
            break;
        case cFSStatArgs::TY_SYMLINK:
            propSet.SetFileType(cFSPropSet::FT_SYMLINK);
            break;
        case cFSStatArgs::TY_FIFO:
            propSet.SetFileType(cFSPropSet::FT_FIFO);
            break;
        case cFSStatArgs::TY_SOCK:
            propSet.SetFileType(cFSPropSet::FT_SOCK);
            break;
        case cFSStatArgs::TY_DOOR:
            propSet.SetFileType(cFSPropSet::FT_DOOR);
            break;
        case cFSStatArgs::TY_PORT:
            propSet.SetFileType(cFSPropSet::FT_PORT);
            break;
        case cFSStatArgs::TY_NAMED:
            propSet.SetFileType(cFSPropSet::FT_NAMED);
            break;
        case cFSStatArgs::TY_NATIVE:
            propSet.SetFileType(cFSPropSet::FT_NATIVE);
            break;
        case cFSStatArgs::TY_MESSAGE_QUEUE:
            propSet.SetFileType(cFSPropSet::FT_MESSAGE_QUEUE);
            break;
        case cFSStatArgs::TY_SEMAPHORE:
            propSet.SetFileType(cFSPropSet::FT_SEMAPHORE);
            break;
        case cFSStatArgs::TY_SHARED_MEMORY:
            propSet.SetFileType(cFSPropSet::FT_SHARED_MEMORY);
            break;
        default:
            // set it to invalid
            propSet.SetFileType(cFSPropSet::FT_INVALID);
        }
    }
}

void cFSPropCalc::HandleHashes(const cFCOPropVector& propsToCheck, const TSTRING& strName, cFSPropSet& propSet)
{
    bool hash_success = false;

    // if the file type is not a regular file, we will
    // not try to open the file for signature generation
    ASSERT(propSet.GetValidVector().ContainsItem(cFSPropSet::PROP_FILETYPE));

    if (   propSet.GetFileType() == cFSPropSet::FT_FILE
        || propSet.GetFileType() == cFSPropSet::FT_SYMLINK
#if (SUPPORTS_NATIVE_OBJECTS)
        || propSet.GetFileType() == cFSPropSet::FT_NATIVE
#endif
     )
    {
        if ( // if we need to open the file
            propsToCheck.ContainsItem(cFSPropSet::PROP_CRC32) || propsToCheck.ContainsItem(cFSPropSet::PROP_MD5) ||
            propsToCheck.ContainsItem(cFSPropSet::PROP_SHA)   || propsToCheck.ContainsItem(cFSPropSet::PROP_HAVAL))
        {
            cFileArchive   arch;
            cMemoryArchive memArch;
            cBidirArchive* pTheArch;
            hash_success = true;

            if (propSet.GetFileType() == cFSPropSet::FT_SYMLINK)
            {
                pTheArch = &memArch;
                if (!GetSymLinkStr(strName, memArch))
                {
                    // add it to the bucket...
                    AddPropCalcError(
                        eArchiveOpen(strName, iFSServices::GetInstance()->GetErrString(), eError::NON_FATAL));
                    hash_success = false;
                }
            }
            else
            {
                pTheArch     = &arch;
                hash_success = DoOpen(strName, arch);
            }

            //
            // if we have successfully initialized the archive
            //
            if (hash_success)
            {
                cArchiveSigGen asg;

                if (propsToCheck.ContainsItem(cFSPropSet::PROP_CRC32))
                {
                    propSet.SetDefinedCRC32(true);
                    asg.AddSig(propSet.GetCRC32());
                }

                if (propsToCheck.ContainsItem(cFSPropSet::PROP_MD5))
                {
                    propSet.SetDefinedMD5(true);
                    asg.AddSig(propSet.GetMD5());
                }

                if (propsToCheck.ContainsItem(cFSPropSet::PROP_SHA))
                {
                    propSet.SetDefinedSHA(true);
                    asg.AddSig(propSet.GetSHA());
                }

                if (propsToCheck.ContainsItem(cFSPropSet::PROP_HAVAL))
                {
                    propSet.SetDefinedHAVAL(true);
                    asg.AddSig(propSet.GetHAVAL());
                }

                //
                // calculate the signatures
                //
                hash_success = DoHash(strName, pTheArch, asg, arch);
            }
        }
    }

    if (!hash_success)
    {
        // We can't calculate signatures, set them to undefined
        if (propsToCheck.ContainsItem(cFSPropSet::PROP_CRC32))
            propSet.SetDefinedCRC32(false);

        if (propsToCheck.ContainsItem(cFSPropSet::PROP_MD5))
            propSet.SetDefinedMD5(false);

        if (propsToCheck.ContainsItem(cFSPropSet::PROP_SHA))
            propSet.SetDefinedSHA(false);

        if (propsToCheck.ContainsItem(cFSPropSet::PROP_HAVAL))
            propSet.SetDefinedHAVAL(false);
    }
}

///////////////////////////////////////////////////////////////////////////////
// VisitFSObject -- this is the workhorse method that actually fills out the
//      passed in FSObject' properties
///////////////////////////////////////////////////////////////////////////////
void cFSPropCalc::VisitFSObject(cFSObject& obj)
{
    cDebug d("cFSPropCalc::VisitFSObject");
    d.TraceDetail(_T("Visiting %s\n"), obj.GetName().AsString().c_str());

    // if we are not in overwrite mode, we need to alter the
    // properties we are calculating...
    cFCOPropVector propsToCheck(mPropVector);
    if (mCollAction == iFCOPropCalc::PROP_LEAVE)
    {
        cFCOPropVector inBoth = propsToCheck;
        inBoth &= obj.GetPropSet()->GetValidVector();
        propsToCheck ^= inBoth;
    }

#ifdef DEBUG
    d.TraceDetail("----->Collision Action = %s\n", mCollAction == iFCOPropCalc::PROP_LEAVE ? "Leave" : "Replace");
    d.TraceDetail("----->Object's valid properties (a):\n");
    obj.GetPropSet()->GetValidVector().TraceContents(cDebug::D_DETAIL);
    d.TraceDetail("----->Properties to calculate: (b)\n");
    mPropVector.TraceContents(cDebug::D_DETAIL);
    d.TraceDetail("----->Properties to change in object ((a&b)^b for Leave or b for Replace):\n");
    propsToCheck.TraceContents(cDebug::D_DETAIL);
#endif //_DEBUG

    // only do the stat() if it is necessary
    cFSStatArgs ss;
    TSTRING     strName = iTWFactory::GetInstance()->GetNameTranslator()->ToStringAPI(obj.GetName());

    // get a reference to the fco's property set
    cFSPropSet& propSet = obj.GetFSPropSet();

    //
    // just return if this object is invalid
    //
    if (propSet.GetFileType() == cFSPropSet::FT_INVALID)
        return;

    if (NeedsStat(propsToCheck))
    {
        if (!DoStat(strName, ss))
            return;

        HandleStatProperties(propsToCheck, ss, propSet);
    }

    HandleHashes(propsToCheck, strName, propSet);
}

void cFSPropCalc::SetPropVector(const cFCOPropVector& pv)
{
    mPropVector = pv;
}

const cFCOPropVector& cFSPropCalc::GetPropVector() const
{
    return mPropVector;
}

iFCOVisitor* cFSPropCalc::GetVisitor()
{
    return this;
}

const iFCOVisitor* cFSPropCalc::GetVisitor() const
{
    return this;
}

void cFSPropCalc::SetErrorBucket(cErrorBucket* pBucket)
{
    mpErrorBucket = pBucket;
}

const cErrorBucket* cFSPropCalc::GetErrorBucket() const
{
    return mpErrorBucket;
}

iFCOPropCalc::CollisionAction cFSPropCalc::GetCollisionAction() const
{
    return mCollAction;
}

void cFSPropCalc::SetCollisionAction(CollisionAction a)
{
    ASSERT((a == iFCOPropCalc::PROP_OVERWRITE) || (a == PROP_LEAVE));
    mCollAction = a;
}
