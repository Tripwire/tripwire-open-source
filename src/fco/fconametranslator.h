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
// fconametranslator.h
//

#ifndef __FCONAMETRANSLATOR_H
#define __FCONAMETRANSLATOR_H

//=========================================================================
// INCLUDES
//=========================================================================

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

class cFCOName;

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

class iFCONameTranslator
{
public:
    //
    // basic functionality
    //
    virtual TSTRING ToStringRaw(const cFCOName& name) const = 0;
    // provides a "raw" representation of the fco name as a string. This uses only the
    // information contained in the cFCOName to construct the string (ie -- no genre specific
    // info)

    virtual TSTRING ToStringAPI(const cFCOName& name) const = 0;
    // returns a string representation of the name that can be passed to system API calls. Note
    // that this is not possible to do in all genres ( for example, the NT registry )

    virtual TSTRING ToStringDisplay(const cFCOName& name, bool bUnique = false) const = 0;
    // this function converts an fconame into a string suitable for printing to user.
    // Every character in the string will be able to be converted to a multi-byte character
    //
    // If unique is set, the string will also have the following attributes:
    // The returned string will have leading and trailing quotes.
    // The string returned must be able to be unambiguously translatable back into an FCO name
    // equal to the one that created the string. To use mathematical language, the function that
    // maps cFCONames to TSTRINGS must be "one to one" and "onto".

    virtual bool DisplayStringToFCOName(const TSTRING& str, cFCOName& name) const = 0;
    // this function converts strings from FCONameToDisplayString back into an fconame
    // returns false if can't convert.

    virtual ~iFCONameTranslator()
    {
    }
};

#endif //__FCONAMETRANSLATOR_H
