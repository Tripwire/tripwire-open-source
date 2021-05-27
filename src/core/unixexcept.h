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
// unixexcept.h
//
// unix api exception class
//
#ifndef __unixexcept_H
#define __unixexcept_H

#ifndef __ERROR_H
#include "error.h"
#endif

TSS_BEGIN_EXCEPTION(eUnix, eError)

eUnix(const TCHAR* functionName, const TCHAR* objectName, bool displayErrorNumber = true);
// construct one of these to indicate a unix api failure. both functionName and
// objectName can be NULL. This method will construct a string to pass to eError
// that contains all of the above information plus whatever errno returns if the dusplayErrorNumber
// parameter is true.


TSS_END_EXCEPTION()


#endif //__unixexcept_H
