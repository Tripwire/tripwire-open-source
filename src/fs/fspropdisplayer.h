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
// fspropdisplayer.h
//

#ifndef __FSPROPDISPLAYER_H
#define __FSPROPDISPLAYER_H

//=========================================================================
// INCLUDES
//=========================================================================

#ifndef __FCOPROPDISPLAYER_H
#include "fco/fcopropdisplayer.h"
#endif

#ifndef __SERIALIZABLE_H
#include "serializable.h"
#endif

#ifndef __FCOPROPVECTOR_H
#include "fco/fcopropvector.h"
#endif

#ifndef __FCOPROP_H
#include "fco/fcoprop.h"
#endif

#ifndef __HASHTABLE_H
#include "core/hashtable.h"
#endif

#ifndef __FCONAME_H
#include "fco/fconame.h"
#endif

//=========================================================================
// DEFINES AND MACROS
//=========================================================================

//=========================================================================
// OTHER DIRECTIVES
//=========================================================================

//=========================================================================
// GLOBALS
//=========================================================================

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

class iFCO;

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================


//
// cFSPropDisplayer
//

class cFSPropDisplayer : public iFCOPropDisplayer
{
public:
    DECLARE_TYPEDSERIALIZABLE()

    cFSPropDisplayer();
    virtual ~cFSPropDisplayer();

    virtual iFCOPropDisplayer* Clone() const;

    virtual void                  Merge(const iFCOPropDisplayer* const ppd);
    virtual const cFCOPropVector& GetPropsConverted() const;
    virtual TSTRING  PropAsString(const iFCO* const pFCO, const int propIdx, int iOffset = 0, int iWidth = 0) const;
    virtual TSTRING  PropAsString(const iFCO* const pFCO, int const propIdx, int iOffset = 0, int iWidth = 0);
    virtual bool     IsMultiLineProp(int propID) const;
    virtual TSTRING& GetDetails(const iFCO* const pfco, TSTRING& strBuf) const;
    virtual TSTRING& GetDetailsHeader(TSTRING& strBuf, int iMargin) const;
    virtual void     InitForProp(const iFCO* const pFCO, const int propIdx);
    virtual void     InitForFCO(const iFCO* const ifco);
    virtual void     SetLazy(const bool bLazy = true);
    virtual bool     GetLazy() const;
    virtual void     Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void     Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

    bool operator==(const cFSPropDisplayer& rhs) const; // for testing

private:
    void AddMapping(const iFCOProp* const pProp, const TSTRING& tstrValue, const int propTypeEnum);
    // pass in a property value and its string representation.  for instance: ( FS::PROP_UID --> username )


    cFCOPropVector mpvPropsWeDisplay; // if a given prop in this vector is valid, then this class
                                      // can do a PropAsString that's different from iFCOProp::AsString()
    bool mbLazy;

    typedef std::map<int64_t, TSTRING> INT64_TO_STRING_MAP;

    bool GetUsername(const int64_t& i64uid, TSTRING& tstrUsername) const;
    bool GetGroupname(const int64_t& i64uid, TSTRING& tstrGroupname) const;
    bool AddUsernameMapping(const int64_t& i64ID, const TSTRING& tstrUsername);
    bool AddGroupnameMapping(const int64_t& i64ID, const TSTRING& tstrGroupname);

    // storage for conversion stuff
    INT64_TO_STRING_MAP uidToUsername;
    INT64_TO_STRING_MAP gidToGroupname;

    friend class cTestFSPropDisplayer;

    enum Widths
    {
        PROP_MODE_WIDTH  = 11,
        PROP_OWNER_WIDTH = 20,
        PROP_SIZE_WIDTH  = 10,
        PROP_MTIME_WIDTH = 10
    };
};


#endif //__FSPROPDISPLAYER_H
