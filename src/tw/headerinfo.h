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
//headerinfo.h : a base class for database and report header objects.
#ifndef __HEADERINFO_H
#define __HEADERINFO_H

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif
#ifndef __TYPES_H
#include "core/types.h"
#endif
#ifndef __SERIALIZABLE_H
#include "core/serializable.h"
#endif

class iFCOPropDisplayer;

///////////////////////////////////////////////////////////////////////////////
// cHeaderInfo
///////////////////////////////////////////////////////////////////////////////
class cHeaderInfo : public iTypedSerializable
{
    DECLARE_TYPEDSERIALIZABLE()

public:
    cHeaderInfo();
    virtual ~cHeaderInfo();
    virtual void Clear();
    //Clears all data in a header to 0 or NULL equivalents.

    //Functions for getting & setting header values:
    const TSTRING& GetPolicyFilename() const;
    const TSTRING& GetConfigFilename() const;
    const TSTRING& GetDBFilename() const;
    const TSTRING& GetSystemName() const;
    const TSTRING& GetCommandLineParams() const;
    const TSTRING& GetIPAddress() const;
    const TSTRING& GetCreator() const;
    const TSTRING& GetHostID() const;
    int64_t        GetCreationTime() const;
    int64_t        GetLastDBUpdateTime() const;
    //Set:
    void SetPolicyFilename(const TSTRING&);
    void SetConfigFilename(const TSTRING&);
    void SetDBFilename(const TSTRING&);
    void SetSystemName(const TSTRING&);
    void SetCommandLineParams(const TSTRING&);
    void SetIPAddress(const TSTRING&);
    void SetCreator(const TSTRING&);
    void SetHostID(const TSTRING&);
    void SetCreationTime(int64_t);
    void SetLastDBUpdateTime(int64_t);

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

private:
    cHeaderInfo& operator=(cHeaderInfo&);

    //Header Data:
    TSTRING         tstr_PolicyFilename;
    TSTRING         tstr_ConfigFilename;
    TSTRING         tstr_DBFilename;
    TSTRING         tstr_SystemName;
    TSTRING         tstr_CommandLineParams;
    TSTRING         tstr_IPAddress;
    TSTRING         tstr_CreatedBy;
    TSTRING         tstr_HostID;
    int64_t         i64_CreationTime;
    mutable int64_t i64_LastDBUpdateTime;
};

///////////////////////////////////////////////////////////////////////////////
// cGenreHeaderInfo -- for genres
///////////////////////////////////////////////////////////////////////////////
class cGenreHeaderInfo : public iTypedSerializable
{
    DECLARE_TYPEDSERIALIZABLE()

public:
    cGenreHeaderInfo();
    cGenreHeaderInfo(const cGenreHeaderInfo&);
    virtual ~cGenreHeaderInfo();

    void Clear();

    // the prop displayer ... once this is set, this object owns the destruction of it.
    void                     SetPropDisplayer(iFCOPropDisplayer* pPropDisplayer);
    iFCOPropDisplayer*       GetPropDisplayer();
    const iFCOPropDisplayer* GetPropDisplayer() const;

    void                     SetObjectsScanned(int32_t);
    int32_t                  GetObjectsScanned() const;

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)
private:
    cGenreHeaderInfo& operator=(cGenreHeaderInfo&);

    iFCOPropDisplayer* mpPropDisplayer;
    int32_t            i32_ObjectsScanned;
};

///////////////////////////////////////////////////////////////////////////////
// cFCODbGenreHeader -- for database genres
//      TODO -- once this actually has data, move it to a seperate file
///////////////////////////////////////////////////////////////////////////////
class cFCODbGenreHeader : public cGenreHeaderInfo
{
    DECLARE_TYPEDSERIALIZABLE()
public:
    cFCODbGenreHeader(){};
    cFCODbGenreHeader(const cFCODbGenreHeader& rhs) : cGenreHeaderInfo(rhs){};

private:
    cFCODbGenreHeader& operator=(cFCODbGenreHeader&);
};

///////////////////////////////////////////////////////////////////////////////
// cFCOReportGenreHeader -- for report genres
//      TODO -- once this actually has data, move it to a seperate file
///////////////////////////////////////////////////////////////////////////////
class cFCOReportGenreHeader : public cGenreHeaderInfo
{
public:
    cFCOReportGenreHeader(){};
    cFCOReportGenreHeader(const cFCOReportGenreHeader& rhs) : cGenreHeaderInfo(rhs){};
    DECLARE_TYPEDSERIALIZABLE()
private:
    cFCOReportGenreHeader& operator=(cFCOReportGenreHeader&);
};


///////////////////////////////////////////////////////////////////////////////
// cFCODbHeader -- specialization of header info for databases
//      TODO -- once this actually has data, move it to a seperate file
///////////////////////////////////////////////////////////////////////////////
class cFCODbHeader : public cHeaderInfo
{
    DECLARE_TYPEDSERIALIZABLE()
public:
    typedef cHeaderInfo inherited;

private:
    cFCODbHeader& operator=(cFCODbHeader&);
};

///////////////////////////////////////////////////////////////////////////////
// cFCOReportHeader -- specialization of header info for reports
//      TODO -- once this actually has data, move it to a seperate file
///////////////////////////////////////////////////////////////////////////////
class cFCOReportHeader : public cHeaderInfo
{
    typedef cHeaderInfo inherited;
    DECLARE_TYPEDSERIALIZABLE()
private:
    cFCOReportHeader& operator=(cFCOReportHeader&);
};


//-----------------------------------------------------------------------------
// inline implementation
//-----------------------------------------------------------------------------

inline const TSTRING& cHeaderInfo::GetPolicyFilename() const
{
    return tstr_PolicyFilename;
};
inline const TSTRING& cHeaderInfo::GetConfigFilename() const
{
    return tstr_ConfigFilename;
};
inline const TSTRING& cHeaderInfo::GetDBFilename() const
{
    return tstr_DBFilename;
};
inline const TSTRING& cHeaderInfo::GetSystemName() const
{
    return tstr_SystemName;
};
inline const TSTRING& cHeaderInfo::GetCommandLineParams() const
{
    return tstr_CommandLineParams;
};
inline const TSTRING& cHeaderInfo::GetCreator() const
{
    return tstr_CreatedBy;
};
inline const TSTRING& cHeaderInfo::GetIPAddress() const
{
    return tstr_IPAddress;
};
inline const TSTRING& cHeaderInfo::GetHostID() const
{
    return tstr_HostID;
};
inline int64_t cHeaderInfo::GetCreationTime() const
{
    return i64_CreationTime;
};
inline int64_t cHeaderInfo::GetLastDBUpdateTime() const
{
    return i64_LastDBUpdateTime;
};

inline void cHeaderInfo::SetPolicyFilename(const TSTRING& tstr)
{
    tstr_PolicyFilename = tstr;
};
inline void cHeaderInfo::SetConfigFilename(const TSTRING& tstr)
{
    tstr_ConfigFilename = tstr;
};
inline void cHeaderInfo::SetDBFilename(const TSTRING& tstr)
{
    tstr_DBFilename = tstr;
};
inline void cHeaderInfo::SetSystemName(const TSTRING& name)
{
    tstr_SystemName = name;
};
inline void cHeaderInfo::SetCommandLineParams(const TSTRING& tstr)
{
    tstr_CommandLineParams = tstr;
};
inline void cHeaderInfo::SetCreator(const TSTRING& tstr)
{
    tstr_CreatedBy = tstr;
};
inline void cHeaderInfo::SetHostID(const TSTRING& tstr)
{
    tstr_HostID = tstr;
};
inline void cHeaderInfo::SetIPAddress(const TSTRING& tstr)
{
    tstr_IPAddress = tstr;
};
inline void cHeaderInfo::SetCreationTime(int64_t i)
{
    i64_CreationTime = i;
};
inline void cHeaderInfo::SetLastDBUpdateTime(int64_t i)
{
    i64_LastDBUpdateTime = i;
};

inline void cGenreHeaderInfo::SetPropDisplayer(iFCOPropDisplayer* pPropDisplayer)
{
    mpPropDisplayer = pPropDisplayer;
}
inline iFCOPropDisplayer* cGenreHeaderInfo::GetPropDisplayer()
{
    return mpPropDisplayer;
}
inline const iFCOPropDisplayer* cGenreHeaderInfo::GetPropDisplayer() const
{
    return mpPropDisplayer;
}
inline void cGenreHeaderInfo::SetObjectsScanned(int32_t i)
{
    i32_ObjectsScanned = i;
};
inline int32_t cGenreHeaderInfo::GetObjectsScanned() const
{
    return i32_ObjectsScanned;
};


#endif //__HEADERINFO_H
