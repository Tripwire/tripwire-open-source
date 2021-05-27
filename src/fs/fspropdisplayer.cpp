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
// fspropdisplayer.cpp
//

#include "stdfs.h"

//=========================================================================
// INCLUDES
//=========================================================================

#include "fspropdisplayer.h"
#include "fspropset.h"
#include "core/fsservices.h"
#include "core/serializer.h"
#include "core/timeconvert.h"
#include "fco/fconame.h"
#include "fco/fco.h"
#include "core/twlocale.h"
#include "core/displayencoder.h"
#include "fco/fcoundefprop.h"
#include "core/errorutil.h"

//=========================================================================
// STANDARD LIBRARY INCLUDES
//=========================================================================

#include <iomanip>

//=========================================================================
// DEFINES AND MACROS
//=========================================================================

//=========================================================================
// OTHER DIRECTIVES
//=========================================================================

#if !IS_AIX
// For some stupid linker reason, this object does not get constructed under
// AIX. The cheap fix is to move this call to fs.cpp.
IMPLEMENT_TYPEDSERIALIZABLE(cFSPropDisplayer, _T("FSPropDisplayer"), 0, 1);
#endif

//=========================================================================
// GLOBALS
//=========================================================================

//=========================================================================
// UTIL FUNCTION PROTOTYES
//=========================================================================

//=========================================================================
// METHOD CODE
//=========================================================================

cFSPropDisplayer::cFSPropDisplayer() : mbLazy(false)
{
    mpvPropsWeDisplay.AddItemAndGrow(cFSPropSet::PROP_UID);
    mpvPropsWeDisplay.AddItemAndGrow(cFSPropSet::PROP_GID);
    mpvPropsWeDisplay.AddItemAndGrow(cFSPropSet::PROP_ATIME);
    mpvPropsWeDisplay.AddItemAndGrow(cFSPropSet::PROP_MTIME);
    mpvPropsWeDisplay.AddItemAndGrow(cFSPropSet::PROP_CTIME);
    mpvPropsWeDisplay.AddItemAndGrow(cFSPropSet::PROP_MODE);

    mpvPropsWeDisplay.AddItemAndGrow(cFSPropSet::PROP_BLOCK_SIZE);
    mpvPropsWeDisplay.AddItemAndGrow(cFSPropSet::PROP_BLOCKS);
    mpvPropsWeDisplay.AddItemAndGrow(cFSPropSet::PROP_SIZE);
    mpvPropsWeDisplay.AddItemAndGrow(cFSPropSet::PROP_NLINK);
}

iFCOPropDisplayer* cFSPropDisplayer::Clone() const
{
    iFCOPropDisplayer* pNew = new cFSPropDisplayer;
    pNew->Merge(this);
    return pNew;
}

bool cFSPropDisplayer::IsMultiLineProp(int propID) const
{
    // no FS props are multiline
    return false;
}

cFSPropDisplayer::~cFSPropDisplayer()
{
    cDebug d("~cFSPropDisplayer");
    d.TraceNever("Destructor called on object %x\n", (ptr_size_type)this);
}

const cFCOPropVector& cFSPropDisplayer::GetPropsConverted() const
{
    return mpvPropsWeDisplay;
}

void cFSPropDisplayer::SetLazy(const bool bLazy /* = true */)
{
    mbLazy = bLazy;
}

bool cFSPropDisplayer::GetLazy() const
{
    return mbLazy;
}

// adds all prop display info from rhs to this
void cFSPropDisplayer::Merge(const iFCOPropDisplayer* const ppd)
{
    ASSERT(ppd != 0);

    // should only merge two prop displayers of the same type
    if (ppd->GetType() != this->GetType())
        ASSERT(false);

    const cFSPropDisplayer* const pfspd = static_cast<const cFSPropDisplayer*>(ppd);

    // merge propvectors
    mpvPropsWeDisplay |= pfspd->GetPropsConverted();


    // merge mapping info
    for (INT64_TO_STRING_MAP::const_iterator iterUID = pfspd->uidToUsername.begin();
         iterUID != pfspd->uidToUsername.end();
         ++iterUID)
    {
        AddUsernameMapping(iterUID->first, iterUID->second);
    }

    for (INT64_TO_STRING_MAP::const_iterator iterGID = pfspd->gidToGroupname.begin();
         iterGID != pfspd->gidToGroupname.end();
         ++iterGID)
    {
        AddGroupnameMapping(iterGID->first, iterGID->second);
    }

    // keep lazy flag the same
}


TSTRING& cFSPropDisplayer::GetDetailsHeader(TSTRING& strBuf, int iMargin) const
{
    static cFSPropSet set;
    TOSTRINGSTREAM    sstr;

#if !ARCHAIC_STL    
    sstr << std::left;
#endif
    
    sstr << _T(" ") << std::setw(PROP_MODE_WIDTH) << set.GetPropName(cFSPropSet::PROP_MODE);
    sstr << _T(" ") << std::setw(PROP_OWNER_WIDTH) << set.GetPropName(cFSPropSet::PROP_UID);
    sstr << _T(" ") << std::setw(PROP_SIZE_WIDTH) << set.GetPropName(cFSPropSet::PROP_SIZE);
    sstr << _T(" ") << std::setw(PROP_MTIME_WIDTH) << set.GetPropName(cFSPropSet::PROP_MTIME);
    sstr << _T("\n");
    for (int i = 0; i < iMargin; i++)
        sstr << _T(" ");
    sstr << _T(" ") << std::setw(PROP_MODE_WIDTH) << _T("------");
    sstr << _T(" ") << std::setw(PROP_OWNER_WIDTH) << _T("----------");
    sstr << _T(" ") << std::setw(PROP_SIZE_WIDTH) << _T("----------");
    sstr << _T(" ") << std::setw(PROP_MTIME_WIDTH) << _T("----------");

    tss_stream_to_string(sstr, strBuf);
    return strBuf;
}


////////////////////////////////////////////////////////////////////////
// Function name    : cFSPropDisplayer::GetDetails
// Description      : takes a TSTRING and fills it with an "ls -l" representation
//                    of the object's permission bits ( e.g. "drwxr-x--x" ), UID,
//                    size, and last access time.  Does not add the name of the FSO
//
// Return type      : TSTRING& -- reference to input buffer
//
// Argument         : TSTRING& strBuf -- buffer.  GetDesription will use it to allocate space.
//
TSTRING& cFSPropDisplayer::GetDetails(const iFCO* const pfco, TSTRING& strBuf) const
{
    TOSTRINGSTREAM       sstr;
    const cFCOPropVector pv = pfco->GetPropSet()->GetValidVector();

#if !ARCHAIC_STL    
    sstr << std::left;
#endif
    
    sstr << _T(" ") << std::setw(PROP_MODE_WIDTH);
    if (pv.ContainsItem(cFSPropSet::PROP_MODE))
        sstr << PropAsString(pfco, cFSPropSet::PROP_MODE).c_str();
    else
        sstr << _T("XXXXXXXXX");

    sstr << _T(" ") << std::setw(PROP_OWNER_WIDTH);
    if (pv.ContainsItem(cFSPropSet::PROP_UID))
        sstr << PropAsString(pfco, cFSPropSet::PROP_UID).c_str();
    else
        sstr << _T("XXXXXXXXX");

    sstr << _T(" ") << std::setw(PROP_SIZE_WIDTH);
    if (pv.ContainsItem(cFSPropSet::PROP_SIZE))
        sstr << PropAsString(pfco, cFSPropSet::PROP_SIZE).c_str();
    else
        sstr << _T("XXX");

    sstr << _T(" ") << std::setw(PROP_MTIME_WIDTH);
    if (pv.ContainsItem(cFSPropSet::PROP_MTIME))
        sstr << PropAsString(pfco, cFSPropSet::PROP_MTIME).c_str();
    else
        sstr << _T("XXXXXXXXXXXXXXXXX");

    tss_stream_to_string(sstr, strBuf);
    return strBuf;
}


void cFSPropDisplayer::InitForFCO(const iFCO* const ifco)
{
    ASSERT(ifco != 0);

    cFCOPropVector v     = ifco->GetPropSet()->GetValidVector();
    int            vSize = v.GetSize();
    for (int i = 0; i < vSize; i++)
    {
        if (v.ContainsItem(i))
        {
            InitForProp(ifco, i);
        }
    }
}


void cFSPropDisplayer::InitForProp(const iFCO* const pFCO, const int propIdx)
{
    ASSERT(pFCO != 0);

    switch (propIdx)
    {
    case cFSPropSet::PROP_UID:
    {
        const int64_t& i64UID =
            static_cast<const cFCOPropInt64*>(pFCO->GetPropSet()->GetPropAt(cFSPropSet::PROP_UID))->GetValue();

        // check if prop is in table.  if it is, then don't hit the FS
        TSTRING tstrDummy;
        if (!GetUsername(i64UID, tstrDummy))
        {
            TSTRING tstrUsername;
            if (iFSServices::GetInstance()->GetUserName(i64UID, tstrUsername))
                AddUsernameMapping(i64UID, tstrUsername);
            else
                AddUsernameMapping(
                    i64UID,
                    _T("")); // on failure, enter error value into mapping so we don't search for this value again.
        }
    }
    break;
    case cFSPropSet::PROP_GID:
    {
        const int64_t& i64GID =
            static_cast<const cFCOPropInt64*>(pFCO->GetPropSet()->GetPropAt(cFSPropSet::PROP_GID))->GetValue();

        // check if prop is in table.  if it is, then don't hit the FS
        TSTRING tstrDummy;
        if (!GetGroupname(i64GID, tstrDummy))
        {
            TSTRING tstrGroupname;
            if (iFSServices::GetInstance()->GetGroupName(i64GID, tstrGroupname))
                AddGroupnameMapping(i64GID, tstrGroupname);
            else
                AddGroupnameMapping(
                    i64GID,
                    _T("")); // on failure, enter error value into mapping so we don't search for this value again.
        }
    }
    break;
    default:
        // right now nothing else needs to be done for the other props
        break;
    }
}

TSTRING cFSPropDisplayer::PropAsString(const iFCO* const pFCO, const int propIdx, int iOffset, int iWidth)
{
    if (mpvPropsWeDisplay.ContainsItem(propIdx) && mbLazy)
    {
        InitForProp(pFCO, propIdx);
    }
    // run the const version of PropAsString
    return ((const cFSPropDisplayer*)this)->PropAsString(pFCO, propIdx, iOffset, iWidth);
}

TSTRING cFSPropDisplayer::PropAsString(const iFCO* const pFCO, const int propIdx, int iOffset, int iWidth) const
{
    TSTRING strProp = _T("");

    ASSERT(pFCO != 0);

    // get prop pointer
    const iFCOProp* pProp = pFCO->GetPropSet()->GetPropAt(propIdx);

    // if we know how to display prop
    if (pProp->GetType() != cFCOUndefinedProp::GetInstance()->GetType() && mpvPropsWeDisplay.ContainsItem(propIdx))
    {
        switch (propIdx)
        {
        case cFSPropSet::PROP_BLOCK_SIZE:
        case cFSPropSet::PROP_BLOCKS:
        case cFSPropSet::PROP_SIZE:
        case cFSPropSet::PROP_NLINK:
        {
	    // Note that supplying both consts within the static cast makes gcc emit a
	    // "type qualifiers ignored on cast result type" (-Wignored-qualifiers) warning.
            const cFCOPropInt64* const pTypedProp = static_cast<const cFCOPropInt64*>(pProp);
            cTWLocale::FormatNumber(pTypedProp->GetValue(), strProp);
        }
        break;
        case cFSPropSet::PROP_ATIME:
        case cFSPropSet::PROP_MTIME:
        case cFSPropSet::PROP_CTIME:
        {
            const cFCOPropInt64* const pTypedProp = static_cast<const cFCOPropInt64*>(pProp);
            int64_t                    i64        = pTypedProp->GetValue();
            cTWLocale::FormatTime(i64, strProp);
        }
        break;
        case cFSPropSet::PROP_MODE:
        {
            const cFCOPropUint64* const pTypedProp = static_cast<const cFCOPropUint64*>(pProp);
            ASSERT(pTypedProp != 0);

            iFSServices::GetInstance()->ConvertModeToString(pTypedProp->GetValue(), strProp);
        }
        break;
        case cFSPropSet::PROP_UID:
        {
            const cFCOPropInt64* const pTypedProp = static_cast<const cFCOPropInt64*>(pProp);
            ASSERT(pTypedProp != 0);
            if (GetUsername(pTypedProp->GetValue(), strProp))
            {
                TOSTRINGSTREAM ostr;
                //TODO: can we get rid of this cast now?
                ostr << strProp << _T(" (") << (int32_t)pTypedProp->GetValue() << _T(")");
		tss_stream_to_string(ostr, strProp);
            }
            else
                strProp = pProp->AsString();
        }
        break;
        case cFSPropSet::PROP_GID:
        {
            const cFCOPropInt64* const pTypedProp = static_cast<const cFCOPropInt64*>(pProp);
            ASSERT(pTypedProp != 0);
            if (GetGroupname(pTypedProp->GetValue(), strProp))
            {
                TOSTRINGSTREAM ostr;
                //TODO: can we get rid of this cast now?
                ostr << strProp << _T(" (") << (int32_t)pTypedProp->GetValue() << _T(")");
		tss_stream_to_string(ostr, strProp);
            }
            else
                strProp = pProp->AsString();
        }
        break;
        default:
            ASSERT(false); // mpvPropsWeDisplay should contain one of the above
            break;
        }
    }
    else // we don't know how to display prop, so let prop display itself
    {
        strProp = pProp->AsString();
    }

    return (strProp);
}

void cFSPropDisplayer::Write(iSerializer* pSerializer) const
{
    mpvPropsWeDisplay.Write(pSerializer);
    pSerializer->WriteInt32(mbLazy ? 1 : 0);


    //  stuff
    {
        // write UID mapping
        pSerializer->WriteInt32(uidToUsername.size());
        for (INT64_TO_STRING_MAP::const_iterator iterUid = uidToUsername.begin(); iterUid != uidToUsername.end();
             ++iterUid)
        {
            pSerializer->WriteInt64(iterUid->first);
            pSerializer->WriteString(iterUid->second);
        }

        // write GID mapping
        pSerializer->WriteInt32(gidToGroupname.size());
        for (INT64_TO_STRING_MAP::const_iterator iterGid = gidToGroupname.begin(); iterGid != gidToGroupname.end();
             ++iterGid)
        {
            pSerializer->WriteInt64(iterGid->first);
            pSerializer->WriteString(iterGid->second);
        }
    }
}

void cFSPropDisplayer::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("Property Displayer Read")));

    mpvPropsWeDisplay.Read(pSerializer);

    int32_t iDummy;
    pSerializer->ReadInt32(iDummy);
    mbLazy = iDummy == 0 ? false : true;

    //  stuff
    {
        int32_t nValues;
        int64_t key;
        TSTRING val;
        int     i;

        // read UID hashtable
        pSerializer->ReadInt32(nValues);
        for (i = 0; i < nValues; i++)
        {
            pSerializer->ReadInt64(key);
            pSerializer->ReadString(val);
            AddUsernameMapping(key, val);
        }

        // read GID hashtable
        pSerializer->ReadInt32(nValues);
        for (i = 0; i < nValues; i++)
        {
            pSerializer->ReadInt64(key);
            pSerializer->ReadString(val);
            AddGroupnameMapping(key, val);
        }
    }
}


//////////////////////////////////////////////
// Lookup functions
//////////////////////////////////////////////

bool cFSPropDisplayer::GetUsername(const int64_t& i64uid, TSTRING& tstrUsername) const
{
    bool fFound = false;

    INT64_TO_STRING_MAP::const_iterator iter = uidToUsername.find(i64uid);

    if (iter != uidToUsername.end())
    {
        fFound       = true;
        tstrUsername = iter->second;
    }

    return (fFound && !tstrUsername.empty());
}

bool cFSPropDisplayer::GetGroupname(const int64_t& i64gid, TSTRING& tstrGroupname) const
{
    bool fFound = false;

    INT64_TO_STRING_MAP::const_iterator iter = gidToGroupname.find(i64gid);

    if (iter != gidToGroupname.end())
    {
        fFound        = true;
        tstrGroupname = iter->second;
    }

    return (fFound && !tstrGroupname.empty());
}

//////////////////////////////////////////////
// Addition functions
//////////////////////////////////////////////

bool cFSPropDisplayer::AddUsernameMapping(const int64_t& i64uid, const TSTRING& tstrUsername)
{
    std::pair<INT64_TO_STRING_MAP::iterator, bool> ret =
        uidToUsername.insert(INT64_TO_STRING_MAP::value_type(i64uid, tstrUsername));
    return (ret.second = false); // returns true if key didn't exist before
}

bool cFSPropDisplayer::AddGroupnameMapping(const int64_t& i64gid, const TSTRING& tstrGroupname)
{
    std::pair<INT64_TO_STRING_MAP::iterator, bool> ret =
        gidToGroupname.insert(INT64_TO_STRING_MAP::value_type(i64gid, tstrGroupname));
    return (ret.second = false); // returns true if key didn't exist before
}

bool cFSPropDisplayer::operator==(const cFSPropDisplayer& rhs) const
{
    return (mpvPropsWeDisplay == rhs.mpvPropsWeDisplay && uidToUsername == rhs.uidToUsername &&
            gidToGroupname == rhs.gidToGroupname);
}
