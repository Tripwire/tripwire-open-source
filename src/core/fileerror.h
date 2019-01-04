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
// fileerror.h -- file error exception class
//

#ifndef __FILEERROR_H
#define __FILEERROR_H

#ifndef __ERROR_H
#include "error.h"
#endif

//=============================================================================
// class eFileError
//
// This error class is useful for errors that need to specify a filename.
// cTWUtil::PrintErrorMsg() returns text similar to the following:
//
// ### Error: File Access denied.               <-- text associated with errorNum
// ### Filename: C:\foo\bar.txt                 <-- constructor parameter "filename"
// ### Insufficient permission to open file.    <-- constructor parameter "msg"
// ### Exiting...                               <-- appropriate third message
//=============================================================================
TSS_BEGIN_EXCEPTION_NO_CTOR(eFileError, eError)
private:
TSTRING mFilename;

public:
eFileError(const TSTRING& filename, const TSTRING& description, uint32_t flags = 0);

explicit eFileError(const eFileError& rhs) : eError(rhs)
{
    mFilename = rhs.mFilename;
}
eFileError(const TSTRING& msg, uint32_t flags = 0) : eError(msg, flags)
{
}


TSTRING         GetFilename() const;
TSTRING         GetDescription() const;
virtual TSTRING GetMsg() const;

TSS_END_EXCEPTION()

#    define TSS_FILE_EXCEPTION(except, base)                                                               \
        TSS_BEGIN_EXCEPTION(except, base)                                                                  \
        except(const TSTRING& filename, const TSTRING& msg, uint32_t flags = 0) : base(filename, msg, flags) \
        {                                                                                                  \
        }                                                                                                  \
        TSS_END_EXCEPTION()

#endif
