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
// serializer.cpp -- The object that does the serialization work.
///////////////////////////////////////////////////////////////////////////////
#include "stdcore.h"
#include "serializer.h"

// TODO: May localizable strings need to be moved to string table

//#############################################################################
// eSerializer
//#############################################################################

///////////////////////////////////////////////////////////////////////////////
// eSerializer::GetMsg -- Overloaded method for returning an error message.
/*virtual*/ TSTRING eSerializer::GetMsg() const
{
    TSTRING ret;

    if (!eSerializer::mDataSource.empty())
    {
        ret = mMsg;

        switch (eSerializer::mSourceType)
        {
        case TY_UNDEFINED:
            ret.append(mDataSource);
            break;
        case TY_FILE:
            ret.append(_T("\nFile: "));
            ret.append(mDataSource);
            break;
        case TY_TEMPFILE:
            ret.append(_T("\nTemporary File: "));
            ret.append(mDataSource);
            break;
        case TY_MEMORY:
            ret.append(_T("\nMemory Block: ")); //uhhh...??
            ret.append(mDataSource);
            break;
        case TY_PIPE:
            ret.append(_T("\nNamed Pipe: "));
            ret.append(mDataSource);
            break;
        case TY_SOCKET:
            ret.append(_T("\nNetwork Socket: "));
            ret.append(mDataSource);
            break;
        default:
            ret.append(_T("\n"));
            ret.append(mDataSource);
            break;
        }
    }
    else
    {
        // Just use the base class method...
        ret = eError::GetMsg();
    }

    return ret;
}

/*
eSerializer::eSerializer(ErrorNum errNum, const TSTRING& msg) :
    eError(errNum, msg)
{
}

const TSTRING&  eSerializer::GetMsg() const
{
    if((mErrorNum < 0) || (mErrorNum >= E_NUMITEMS))
    {
        // I don't know what this error number is; just use the base class implementation
        return eError::GetMsg();
    }

    static TSTRING message;
    static const TCHAR* reasonStrings[] = { _T("The serializer encountered an unknown type"),
                                            _T("Invlaid input stream format for serializer"),
                                            _T("Archive error"),
                                            _T("Version Mismatch"),
                                            _T("Invalid Reason") };

    message = reasonStrings[mErrorNum];
    message += _T(" : ");
    message += mMsg;

    return message;

}

*/
