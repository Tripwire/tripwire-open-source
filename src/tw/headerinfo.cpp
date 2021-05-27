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
//////////////////////////////////////////////////////////////////////////////
//headerinfo.cpp : implementation for base class that stores and returns info.
//      for database and report header objects.

#include "stdtw.h"
#include "headerinfo.h"
#include "core/debug.h"
#include "core/serializer.h"
#include "fco/fcopropdisplayer.h"
#include "core/errorutil.h"

IMPLEMENT_TYPEDSERIALIZABLE(cHeaderInfo, _T("cHeaderInfo"), 0, 1);
IMPLEMENT_TYPEDSERIALIZABLE(cFCODbHeader, _T("cFCODbHeader"), 0, 1);
IMPLEMENT_TYPEDSERIALIZABLE(cFCOReportHeader, _T("cFCOReportHeader"), 0, 1);

IMPLEMENT_TYPEDSERIALIZABLE(cGenreHeaderInfo, _T("cGenreHeaderInfo"), 0, 1);
IMPLEMENT_TYPEDSERIALIZABLE(cFCODbGenreHeader, _T("cFCODbGenreHeader"), 0, 1);
IMPLEMENT_TYPEDSERIALIZABLE(cFCOReportGenreHeader, _T("cFCOReportGenreHeader"), 0, 1);


//////////////////////////////////////////////////////////////////////////////
// Ctor, Dtor: Ctor intializes the Header data to zero or a NULL equivalent.
cHeaderInfo::cHeaderInfo()
{
    Clear();
}

cHeaderInfo::~cHeaderInfo()
{
}


void cHeaderInfo::Clear()
{
    tstr_SystemName        = _T("");
    tstr_PolicyFilename    = _T("");
    tstr_ConfigFilename    = _T("");
    tstr_DBFilename        = _T("");
    tstr_CommandLineParams = _T("");
    tstr_CreatedBy         = _T("");
    tstr_IPAddress         = _T("");
    tstr_HostID            = _T("");
    i64_CreationTime       = 0;
    i64_LastDBUpdateTime   = 0;
}


void cHeaderInfo::Read(iSerializer* pSerializer, int32_t version) // throw (eSerializer, eArchive)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("cHeaderInfo Read")));

    pSerializer->ReadString(tstr_SystemName);
    pSerializer->ReadString(tstr_PolicyFilename);
    pSerializer->ReadString(tstr_ConfigFilename);
    pSerializer->ReadString(tstr_DBFilename);
    pSerializer->ReadString(tstr_CommandLineParams);
    pSerializer->ReadString(tstr_CreatedBy);
    pSerializer->ReadString(tstr_IPAddress);
    pSerializer->ReadString(tstr_HostID);
    pSerializer->ReadInt64(i64_CreationTime);
    pSerializer->ReadInt64(i64_LastDBUpdateTime);
}


void cHeaderInfo::Write(iSerializer* pSerializer) const // throw (eSerializer, eArchive)
{
    pSerializer->WriteString(tstr_SystemName);
    pSerializer->WriteString(tstr_PolicyFilename);
    pSerializer->WriteString(tstr_ConfigFilename);
    pSerializer->WriteString(tstr_DBFilename);
    pSerializer->WriteString(tstr_CommandLineParams);
    pSerializer->WriteString(tstr_CreatedBy);
    pSerializer->WriteString(tstr_IPAddress);
    pSerializer->WriteString(tstr_HostID);
    pSerializer->WriteInt64(i64_CreationTime);
    pSerializer->WriteInt64(i64_LastDBUpdateTime);
}


//////////////////////////////////////////////////////////////////////////////
// Ctor, Dtor: Ctor intializes the Header data to zero or a NULL equivalent.
cGenreHeaderInfo::cGenreHeaderInfo() : mpPropDisplayer(0), i32_ObjectsScanned(0)
{
}

cGenreHeaderInfo::~cGenreHeaderInfo()
{
    delete mpPropDisplayer;
}

cGenreHeaderInfo::cGenreHeaderInfo(const cGenreHeaderInfo& rhs) : mpPropDisplayer(0)
{
    if (this == &rhs)
        return;

    if (rhs.mpPropDisplayer)
        mpPropDisplayer = rhs.GetPropDisplayer()->Clone();

    i32_ObjectsScanned = rhs.GetObjectsScanned();
}

void cGenreHeaderInfo::Clear()
{
    delete mpPropDisplayer;
    mpPropDisplayer    = 0;
    i32_ObjectsScanned = 0;
}

void cGenreHeaderInfo::Read(iSerializer* pSerializer, int32_t version) // throw (eSerializer, eArchive)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("cHeaderInfo Read")));

    // read the prop displayer
    ASSERT(mpPropDisplayer == 0);

    int32_t fMakePD;
    pSerializer->ReadInt32(fMakePD);

    if (fMakePD == 1)
        mpPropDisplayer = static_cast<iFCOPropDisplayer*>(pSerializer->ReadObjectDynCreate());

    pSerializer->ReadInt32(i32_ObjectsScanned);
}


void cGenreHeaderInfo::Write(iSerializer* pSerializer) const // throw (eSerializer, eArchive)
{
    // write out the prop displayer
    if (mpPropDisplayer)
    {
        pSerializer->WriteInt32(1);
        pSerializer->WriteObjectDynCreate(mpPropDisplayer);
    }
    else
        pSerializer->WriteInt32(0);

    pSerializer->WriteInt32(i32_ObjectsScanned);
}
