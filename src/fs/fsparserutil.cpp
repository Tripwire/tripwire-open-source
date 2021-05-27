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
// fsparserutil.cpp
//

//=========================================================================
// INCLUDES
//=========================================================================

#include "stdfs.h"

#include "fsparserutil.h"
#include "core/fsservices.h"
#include "core/file.h"
#include "fspropset.h"
#include "fsstrings.h"
#include "fco/fcopropvector.h"
#include "fco/fconame.h"
#include "fco/fcostrings.h"

//=========================================================================
// PUBLIC METHOD CODE
//=========================================================================

bool cFSParserUtil::MapStringToProperty(const TSTRING& str, int& propIndex) const
{
    bool fMappedChar = true; // assume we'll recognize this char
                             // and set false if we don't

    // for short names
    if (str.length() == 1)
    {
        switch (str[0])
        {
        case 'p':
            propIndex = cFSPropSet::PROP_MODE;
            break;
        case 'i':
            propIndex = cFSPropSet::PROP_INODE;
            break;
        case 'n':
            propIndex = cFSPropSet::PROP_NLINK;
            break;
        case 'u':
            propIndex = cFSPropSet::PROP_UID;
            break;
        case 'g':
            propIndex = cFSPropSet::PROP_GID;
            break;
        case 's':
            propIndex = cFSPropSet::PROP_SIZE;
            break;
        case 't':
            propIndex = cFSPropSet::PROP_FILETYPE;
            break;
        case 'd':
            propIndex = cFSPropSet::PROP_DEV;
            break;
        case 'r':
            propIndex = cFSPropSet::PROP_RDEV;
            break;
        case 'b':
            propIndex = cFSPropSet::PROP_BLOCKS;
            break;
        case 'a':
            propIndex = cFSPropSet::PROP_ATIME;
            break;
        case 'm':
            propIndex = cFSPropSet::PROP_MTIME;
            break;
        case 'c':
            propIndex = cFSPropSet::PROP_CTIME;
            break;
        case 'C':
            propIndex = cFSPropSet::PROP_CRC32;
            break;
        case 'M':
            propIndex = cFSPropSet::PROP_MD5;
            break;
        case 'S':
            propIndex = cFSPropSet::PROP_SHA;
            break;
        case 'H':
            propIndex = cFSPropSet::PROP_HAVAL;
            break;
        case 'l':
            propIndex = cFSPropSet::PROP_GROWING_FILE;
            break;
        default:
            fMappedChar = false;
            break;
        }
    }
    else
        fMappedChar = false;

    /* Leaving this here in case we ever want to implement long property names
 
    else
    {
        if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_MODE ) ) )
            propIndex = cFSPropSet::PROP_MODE;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_INODE ) ) )
            propIndex = cFSPropSet::PROP_INODE;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_UID ) ) )
            propIndex = cFSPropSet::PROP_UID;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_GID ) ) )
            propIndex = cFSPropSet::PROP_GID;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_SIZE ) ) )
            propIndex = cFSPropSet::PROP_SIZE;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_FILETYPE ) ) )
            propIndex = cFSPropSet::PROP_FILETYPE;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_DEV ) ) )
            propIndex = cFSPropSet::PROP_DEV;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_RDEV ) ) )
            propIndex = cFSPropSet::PROP_RDEV;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_BLOCKS ) ) )
            propIndex = cFSPropSet::PROP_BLOCKS;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_ATIME ) ) )
            propIndex = cFSPropSet::PROP_ATIME;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_MTIME ) ) )
            propIndex = cFSPropSet::PROP_MTIME;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_CTIME ) ) )
            propIndex = cFSPropSet::PROP_CTIME;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_CRC32 ) ) )
            propIndex = cFSPropSet::PROP_CRC32;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_MD5 ) ) )
            propIndex = cFSPropSet::PROP_MD5;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_SHA ) ) )
            propIndex = cFSPropSet::PROP_SHA;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_HAVAL ) ) )
            propIndex = cFSPropSet::PROP_HAVAL;
        else if( 0 == str.compare( TSS_GetString( cFS, fs::STR_PARSER_PROP_GROWING_FILE ) ) )
            propIndex = cFSPropSet::PROP_GROWING_FILE;
        else
            fMappedChar = false;
    }
*/

    return (fMappedChar);
}

void cFSParserUtil::AddSubTypeProps(cFCOPropVector& v) const
{
    v.AddItemAndGrow(cFSPropSet::PROP_FILETYPE);
}

static inline void trim_leading_whitespace(std::string& str)
{
// C++17 removes std::ptr_fun and deprecates std::not1,
// so just use a lambda where available
#if !USE_LAMBDAS
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
#else
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int c) {return !std::isspace(c);} ));
#endif
}

void cFSParserUtil::InterpretFCOName(const std::list<TSTRING>& l, cFCOName& nameOut) const
{
    TSTRING strT;
    for (std::list<TSTRING>::const_iterator i = l.begin(); i != l.end(); ++i)
        strT += *i;

#if USES_DEVICE_PATH
    strT = cDevicePath::AsPosix(strT);
#endif

    //Trim any remaining whitespace before actual path, for cases like " /foo",
    // otherwise it'll be flagged as a relative path.
    //Don't trim trailing whitespace, since that could potentially be a valid path.
    if (strT[0] != '/')
        trim_leading_whitespace(strT);

    // let cFCOName handle interpretation
    nameOut = strT;
}

bool cFSParserUtil::EnumPredefinedVariables(int index, TSTRING& sName, TSTRING& sValue) const
{
    switch (index)
    {
    case 0:
        sName  = TSS_GetString(cFS, fs::STR_PARSER_READONLY);
        sValue = TSS_GetString(cFS, fs::STR_FS_PARSER_READONLY_VAL);
        return true;
    case 1:
        sName  = TSS_GetString(cFS, fs::STR_PARSER_DYNAMIC);
        sValue = TSS_GetString(cFS, fs::STR_FS_PARSER_DYNAMIC_VAL);
        return true;
    case 2:
        sName  = TSS_GetString(cFS, fs::STR_PARSER_GROWING);
        sValue = TSS_GetString(cFS, fs::STR_FS_PARSER_GROWING_VAL);
        return true;
    case 3:
        sName  = TSS_GetString(cFS, fs::STR_PARSER_IGNOREALL);
        sValue = TSS_GetString(cFS, fs::STR_FS_PARSER_IGNOREALL_VAL);
        return true;
    case 4:
        sName  = TSS_GetString(cFS, fs::STR_PARSER_IGNORENONE);
        sValue = TSS_GetString(cFS, fs::STR_FS_PARSER_IGNORENONE_VAL);
        return true;
    case 5:
        sName  = TSS_GetString(cFS, fs::STR_PARSER_DEVICE);
        sValue = TSS_GetString(cFS, fs::STR_FS_PARSER_DEVICE_VAL);
        return true;
    case 6:
        sName = TSS_GetString(cFS, fs::STR_PARSER_HOSTNAME);
        try
        {
            iFSServices::GetInstance()->GetMachineName(sValue);
            if (sValue.empty())
                sValue = TSS_GetString(cFS, fs::STR_FS_PARSER_HOSTNAME_VAL);
        }
        catch (eFSServices&)
        {
            sValue = TSS_GetString(cFS, fs::STR_FS_PARSER_HOSTNAME_VAL);
        }
        return true;
    default:
        return false;
    }

    ASSERT(false); // unreachable code
}

bool cFSParserUtil::IsAbsolutePath(const TSTRING& strPath) const
{
#if USES_DEVICE_PATH
    return cDevicePath::IsAbsolutePath(strPath);
#else
    // IF there's a first character AND it's a '/', it's absolute.
    return (strPath.size() > 0 && (_T('/') == strPath[0]));
#endif
}
