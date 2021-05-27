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
// serializerimpl.cpp
#include "stdcore.h"
#include "serializerimpl.h"
#include "debug.h"
#include "archive.h"
#include "ntmbs.h" // for: eCharacterEncoding
#include "crc32.h"

// static members
cSerializerImpl::SerMap         cSerializerImpl::mSerCreateMap;
cSerializerImpl::SerRefCountMap cSerializerImpl::mSerRefCountCreateMap;

///////////////////////////////////////////////////////////////////////////////
// util_GetCrc -- calculates the crc for the narrow version of the type's AsString()
///////////////////////////////////////////////////////////////////////////////
static uint32_t util_GetCRC(const cType& type)
{
    //
    // convert this to narrow...
    //

    // NOTE:RAD -- Fixed bug when going from TCHAR is Wide to Multibyte
    // 09/01/99  - Increased performance by returning byte len instead of
    //             recalculating again in the call to crcUpdate.
    //           - Increased performance by removing superflous conversion
    //             when type.AsString() is already narrow.
    //

    // We only need to count the characters
    // RAD: Yeesh! This is already done for us in cType::mString!!!
    const uint8_t* pszType = (const uint8_t*)(type.AsString());
    int            nBytes  = ::strlen((const char*)pszType);

    //ASSERT(sizeof(uint8_t) == sizeof(byte));
    ASSERT(pszType && *pszType);

    //
    // calculate the crc...
    //
    CRC_INFO crc;
    crcInit(crc);
    crcUpdate(crc, pszType, nBytes);
    crcFinit(crc);

    return crc.crc;
}

//#############################################################################
// class cSerializerImpl
//#############################################################################
cSerializerImpl::cSerializerImpl(cArchive& archive, Mode action, const TSTRING& fileName)
    : mpArchive(&archive), mMode(action), mbInit(false), mFileName(fileName)
{
}

cSerializerImpl::~cSerializerImpl()
{
}

bool cSerializerImpl::IsWriting() const
{
    return (mMode == S_WRITE);
}


///////////////////////////////////////////////////////////////////////////////
// static object registration functions; Exactly one of these should be called for
//      every object that is to be serialized.
///////////////////////////////////////////////////////////////////////////////

// TODO:dmb - following line doesn't do anything???
//std::allocator< std::pair< unsigned long, iTypedSerializable*(*)() > >;

void cSerializerImpl::RegisterSerializable(const cType& type, iTypedSerializable::CreateFunc pFunc)
{
    uint32_t crc = util_GetCRC(type);

    if (cSerializerImpl::mSerCreateMap.find(crc) != cSerializerImpl::mSerCreateMap.end())
    {
        // duplicate entry!
        //
        ASSERT(false);
        TOSTRINGSTREAM str;
        str << _T("Duplicate entry in type table: ") << type.AsString() << std::endl;
	tss_mkstr(errStr, str);
        throw eInternal(errStr);
    }
    cSerializerImpl::mSerCreateMap[crc] = pFunc;
}

void cSerializerImpl::RegisterSerializableRefCt(const cType& type, iSerRefCountObj::CreateFunc pFunc)
{
    uint32_t crc = util_GetCRC(type);

    if (cSerializerImpl::mSerRefCountCreateMap.find(crc) != cSerializerImpl::mSerRefCountCreateMap.end())
    {
        // duplicate entry!
        //
        ASSERT(false);
        TOSTRINGSTREAM str;
        str << _T("Duplicate entry in type table: ") << type.AsString() << std::ends;
	tss_mkstr(errStr, str);
        throw eInternal(errStr);
    }
    cSerializerImpl::mSerRefCountCreateMap[crc] = pFunc;
}


///////////////////////////////////////////////////////////////////////////////
// Init -- the job of init is to clear out the RefCountObj table, fill out the
//      mTypeArray, and  and write the header information
///////////////////////////////////////////////////////////////////////////////
void cSerializerImpl::Init()
{
    cDebug d("cSerializerImpl::Init");
    d.TraceDetail("Entering; IsWriting = %s\n", IsWriting() ? "true" : "false");

    mRefCtObjTbl.Clear();

    mbInit = true;
}

///////////////////////////////////////////////////////////////////////////////
// Finit
///////////////////////////////////////////////////////////////////////////////
void cSerializerImpl::Finit()
{
    cDebug d("cSerializerImpl::Finit");
    d.TraceDetail("Exiting; IsWriting = %s\n", IsWriting() ? "true" : "false");
    mbInit = false;
}

///////////////////////////////////////////////////////////////////////////////
// WriteObjectDynCreate
///////////////////////////////////////////////////////////////////////////////
void cSerializerImpl::WriteObjectDynCreate(const iTypedSerializable* pObj)
{
    ASSERT(mbInit);
    ASSERT(IsWriting());
    cDebug d("cSerializerImpl::WriteObjectDynCreate");
    // CurrentPos() only works with bidir archive
    //d.TraceDetail("Entering...  Archive Offset = %d\n", mpArchive->CurrentPos());
    d.TraceDetail(_T("             Object Type    = %s\n"), pObj->GetType().AsString());
    // first, we write out the header, which consists of the following:
    // uint32_t   crc of the object's type
    // int32_t    version of stored data
    // int32_t    size of the chunk (counting from this point; not including the previous int32
    // int32_t    index into mRefCountObjTbl, or 0 if it isn't refrence counted.
    //              if the index already exists, then no data follows; a refrence
    //              should just be added to the existing object.
    ASSERT(mpArchive != 0);

    // get the ident for this class type
    //
    uint32_t crc = util_GetCRC(pObj->GetType());

    //
    // make sure this type is registered, and figure out if it is refrence counted
    //
    SerRefCountMap::iterator i         = mSerRefCountCreateMap.find(crc);
    bool                     bRefCount = true;
    if (i == mSerRefCountCreateMap.end())
    {
        //
        // maybe it is not refrence counted...
        //
        SerMap::iterator si = mSerCreateMap.find(crc);
        if (si == mSerCreateMap.end())
        {
            d.TraceError("Attempt to serialize unregistered type : %s\n", pObj->GetType().AsString());
            ThrowAndAssert(eSerializerUnknownType(pObj->GetType().AsString(), mFileName, eSerializer::TY_FILE));
        }
        bRefCount = false;
    }

    mpArchive->WriteInt32(crc);
    mpArchive->WriteInt32(pObj->Version());
    // write a placeholder for the size; we will come back and fill in the right value later...
    mpArchive->WriteInt32(0xffffffff);

    if (bRefCount)
    {
        // see if the object has already been serialized...
        int idx;
        if ((idx = mRefCtObjTbl.Lookup(static_cast<const iSerRefCountObj*>(pObj))) == 0)
        {
            // it is not in the table yet; add it and serialize the object
            idx = mRefCtObjTbl.Add(static_cast<const iSerRefCountObj*>(pObj));
            d.TraceDetail("Object [%d] written for first time...\n", idx);
            mpArchive->WriteInt32(idx);
            pObj->Write(this);
        }
        else
        {
            // since it is already in the table, just write the index number
            d.TraceDetail("Object [%d] already serialized; incrementing refrence count.\n", idx);
            mpArchive->WriteInt32(idx);
        }
    }
    else
    {
        // not reference counted ... just write 0
        mpArchive->WriteInt32(0);
        // ... then write the object.
        pObj->Write(this);
    }
}

///////////////////////////////////////////////////////////////////////////////
// ReadObjectDynCreate
///////////////////////////////////////////////////////////////////////////////
iTypedSerializable* cSerializerImpl::ReadObjectDynCreate()
{
    cDebug d("cSerializerImpl::ReadObjectDynCreate");
    //d.TraceDetail("Entering... archive offset = %d\n", mpArchive->CurrentPos());

    int32_t  size, objIdx;
    uint32_t crc;
    // first, get the type...
    mpArchive->ReadInt32(reinterpret_cast<int32_t&>(crc));

    // read in the version
    int32_t version;
    mpArchive->ReadInt32(version);

    // read in the size and the index...
    mpArchive->ReadInt32(size);

    // Save the position so we can seek correctly later on
    //int64_t sizePos = mpArchive->CurrentPos();

    mpArchive->ReadInt32(objIdx);
    if (objIdx == 0)
    {
        // the object is not reference counted; create and read in the object
        // first, we need to create the object.
        SerMap::iterator si;
        si = mSerCreateMap.find(crc);
        if (si == mSerCreateMap.end())
        {
            // unable to find the creation function...
            d.TraceError("Unable to find creation function for non-ref counted object %d\n", crc);
            TSTRING errStr;
	    
#ifdef DEBUG
            TOSTRINGSTREAM str;	    
            // Let's only report the actual crc in debug mode
            str << (int32_t)crc << std::ends;
	    tss_mkstr(errStr, str);
#endif
            ThrowAndAssert(eSerializerUnknownType(errStr, mFileName, eSerializer::TY_FILE));
        }
        iTypedSerializable* pObj = ((*si).second)();
        d.TraceDetail("Created non-ref counted object %s(%p)\n", pObj->GetType().AsString(), pObj);
        pObj->Read(this, version);

        // seek past this object in case filepos is not correct!
        //mpArchive->Seek(sizePos + size, cBidirArchive::BEGINNING);

        return pObj;
    }
    else
    {
        // reference counted...
        iSerRefCountObj* pObj;
        pObj = mRefCtObjTbl.Lookup(objIdx);
        if (pObj == NULL)
        {
            // not in table yet...first find the creation function
            SerRefCountMap::iterator rci;
            rci = mSerRefCountCreateMap.find(crc);
            if (rci == mSerRefCountCreateMap.end())
            {
                // unable to find the creation function...
                d.TraceError("Unable to find creation function for ref counted object %d\n", crc);
                TOSTRINGSTREAM str;
                str << (int32_t)crc;;
		tss_mkstr(errStr, str);
                ThrowAndAssert(eSerializerUnknownType(errStr, mFileName, eSerializer::TY_FILE));
            }
            pObj = ((*rci).second)();
            d.TraceDetail("Creating Ref-Counted object [%d] %s(%p)\n", objIdx, pObj->GetType().AsString(), pObj);
            pObj->Read(this);
            mRefCtObjTbl.Add(pObj, objIdx);

            // seek past this object in case filepos is not correct!
            //mpArchive->Seek(sizePos + size, cBidirArchive::BEGINNING);

            return pObj;
        }
        else
        {
            // already serialized; just return this object.
            d.TraceDetail("Adding reference to previously serialized object [%d] %s(%p)\n",
                          objIdx,
                          pObj->GetType().AsString(),
                          pObj);
            pObj->AddRef();
        }

        // seek past this object in case filepos is not correct!
        //mpArchive->Seek(sizePos + size, cBidirArchive::BEGINNING);

        return pObj;
    }
}

///////////////////////////////////////////////////////////////////////////////
// WriteObect, ReadObject
///////////////////////////////////////////////////////////////////////////////
void cSerializerImpl::WriteObject(const iTypedSerializable* pObj)
{
    ASSERT(pObj != 0);
    //ASSERT(mbInit); // this isn't needed if we are not writing the type array
    ASSERT(IsWriting());
    cDebug d("cSerializerImpl::WriteObject");
    //d.TraceDetail("Entering...  Archive Offset = %d\n", mpArchive->CurrentPos());
    d.TraceDetail("             Object Type    = %s\n", pObj->GetType().AsString());
    // first, we write out the header, which consists of the following:
    // int32_t refrence into mTypeArray, indicating the object's type
    // int32_t version of stored data
    // int32_t size of the chunk (counting from this point; not including the previous int32
    // data  the object data
    ASSERT(mpArchive != 0);


    // 5Nov 98 mdb -- I am removing the read and write of type info for this method; it is never used, since
    // the object is already created when ReadObject() happens. The only good it might serve is in asserting that
    // the input stream is valid, but I can do that with the 0xffffffff size below (asserting that it is the same
    // when it is read back in)
    /*  int i = 0;
    for(; i < mTypeArray.size(); i++)
    {
        if(pObj->GetType() == *mTypeArray[i])
        {
            // aha! this is it!
            break;
        }
    }
    if(i == mTypeArray.size())
    {
        // the type was not registered; 
        // a debate exists in my mind as to whether this should be an exception or an assertion -- mdb
        d.TraceError("Attempt to serialize unregistered type : %s\n", pObj->GetType().AsString());
        ThrowAndAssert(eSerializerUnknownType(pObj->GetType().AsString()));
    }
    mpArchive->WriteInt32(i);
*/

    mpArchive->WriteInt32(0); // place holder for type array index
    mpArchive->WriteInt32(pObj->Version());
    // write a placeholder for the size; we will come back and fill in the right value later...
    //int64_t sizePos = mpArchive->CurrentPos();
    mpArchive->WriteInt32(0xffffffff);

    // write out the object!
    pObj->Write(this);

    // finally, we need to go back and patch up the size...
    //int64_t returnPos = mpArchive->CurrentPos();
    //mpArchive->Seek(sizePos, cBidirArchive::BEGINNING);
    //mpArchive->WriteInt32((int32_t)(returnPos - sizePos - sizeof(int32_t)));
    //mpArchive->Seek(returnPos, cBidirArchive::BEGINNING);
}

void cSerializerImpl::ReadObject(iTypedSerializable* pObj)
{
    cDebug d("cSerializerImpl::ReadObjectDynCreate");
    //d.TraceDetail("Entering... archive offset = %d\n", mpArchive->CurrentPos());

    // NOTE -- type index stuff is gone; see the comment in WriteObject()
    int32_t /*typeIdx,*/ size;
    // first, get the type...
    /*
    mpArchive->ReadInt32(typeIdx);
    if((typeIdx < 0) || (typeIdx >= mTypeArray.size()))
    {
        // unknown type index!
        d.TraceError("Encountered bad index into TypeArray: %d\n", typeIdx);
        ThrowAndAssert(eSerializerInputStremTypeArray());
    }
    const cType* pType = mTypeArray[typeIdx];
    */

    // read in the version
    int32_t dummy, version;
    mpArchive->ReadInt32(dummy); // old type array index
    mpArchive->ReadInt32(version);

    // read in the size and the index...
    mpArchive->ReadInt32(size);

    // use the size to assert that the file format is correct
    if (size != (int)0xffffffff)
    {
        // unknown type index!
        d.TraceError("Encountered bad size: %d\n", size);
        ThrowAndAssert(eSerializerInputStremTypeArray(_T(""), mFileName, eSerializer::TY_FILE));
    }

    // remember current position
    //int64_t sizePos = mpArchive->CurrentPos();

    // read in the object!
    pObj->Read(this, version);

    // seek past this object in case filepos is not correct!
    //mpArchive->Seek(sizePos + size, cBidirArchive::BEGINNING);
}


///////////////////////////////////////////////////////////////////////////////
// archive wrapper
///////////////////////////////////////////////////////////////////////////////
void cSerializerImpl::ReadInt16(int16_t& ret)
{
    mpArchive->ReadInt16(ret);
}

void cSerializerImpl::ReadInt32(int32_t& ret)
{
    mpArchive->ReadInt32(ret);
}

void cSerializerImpl::ReadInt64(int64_t& ret)
{
    mpArchive->ReadInt64(ret);
}

void cSerializerImpl::ReadString(TSTRING& ret)
{
    mpArchive->ReadString(ret);
}

int cSerializerImpl::ReadBlob(void* pBlob, int count)
{
    return mpArchive->ReadBlob(pBlob, count);
}

void cSerializerImpl::WriteInt16(int16_t i)
{
    mpArchive->WriteInt16(i);
}

void cSerializerImpl::WriteInt32(int32_t i)
{
    mpArchive->WriteInt32(i);
}

void cSerializerImpl::WriteInt64(int64_t i)
{
    mpArchive->WriteInt64(i);
}

void cSerializerImpl::WriteString(const TSTRING& s)
{
    mpArchive->WriteString(s);
}

void cSerializerImpl::WriteBlob(const void* pBlob, int count)
{
    mpArchive->WriteBlob(pBlob, count);
}

///////////////////////////////////////////////////////////////////////////////
// GetFileName
///////////////////////////////////////////////////////////////////////////////
TSTRING cSerializerImpl::GetFileName() const
{
    return mFileName;
}
