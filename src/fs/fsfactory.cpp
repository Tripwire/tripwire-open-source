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
// fsfactory.cpp

#include "stdfs.h"

#include "fsfactory.h"
#include "fspropcalc.h"
#include "fco/fcospecimpl.h"
#include "fspropdisplayer.h"
#include "fsobject.h"
#include "fsparserutil.h"
#include "fsdatasourceiter.h"
#include "fco/fconameinfo.h"
#include "fsnametranslator.h"

//-----------------------------------------------------------------------------
// cUnixFCONameInfo
//-----------------------------------------------------------------------------
class cUnixNameInfo : public iFCONameInfo
{
public:
    virtual bool IsCaseSensitive() const
    {
        return true;
    }

    virtual TCHAR GetDelimitingChar() const
    {
        return _T('/');
    }

    virtual ~cUnixNameInfo()
    {
    }
};


iFCOPropCalc* cFSFactory::CreatePropCalc() const
{
    return new cFSPropCalc();
}

iFCOSpec* cFSFactory::CreateSpec(const TSTRING& name, iFCOSpecHelper* pHelper) const
{
    return new cFCOSpecImpl(name, NULL, pHelper);
}

iFCOPropDisplayer* cFSFactory::CreatePropDisplayer() const
{
    static iFCOPropDisplayer* gPropDisplayer = 0;

    if (!gPropDisplayer)
        gPropDisplayer = new cFSPropDisplayer();

    return gPropDisplayer;
}

iSerRefCountObj::CreateFunc cFSFactory::GetCreateFunc() const
{
    return cFSObject::Create;
}

iParserGenreUtil* cFSFactory::CreateParserGenreUtil() const
{
    return new cFSParserUtil();
}

iFCODataSourceIter* cFSFactory::CreateDataSourceIter() const
{
    return new cFSDataSourceIter();
}


iFCONameInfo* cFSFactory::GetNameInfo() const
{
    static cUnixNameInfo gUnixNameInfo;
    return &gUnixNameInfo;
}

cFCOPropVector cFSFactory::GetLooseDirMask() const
{
    static cFCOPropVector vec;
    static bool           bInit = false;

    if (!bInit)
    {
        vec.AddItem(cFSPropSet::PROP_SIZE);
        vec.AddItem(cFSPropSet::PROP_NLINK);
        vec.AddItem(cFSPropSet::PROP_ATIME);
        vec.AddItem(cFSPropSet::PROP_CTIME);
        vec.AddItem(cFSPropSet::PROP_MTIME);
        vec.AddItem(cFSPropSet::PROP_BLOCKS);
        vec.AddItem(cFSPropSet::PROP_GROWING_FILE);
        vec.AddItem(cFSPropSet::PROP_CRC32);
        vec.AddItem(cFSPropSet::PROP_MD5);
        vec.AddItem(cFSPropSet::PROP_SHA);
        vec.AddItem(cFSPropSet::PROP_HAVAL);

        bInit = true;
    }

    return vec;
}

iFCONameTranslator* cFSFactory::GetNameTranslator() const
{
    static cFSNameTranslator gFSNameTranslator;
    return (&gFSNameTranslator);
}
