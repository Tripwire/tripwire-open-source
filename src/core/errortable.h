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
// File:    errortable.h
// Date:    30 April 99
// Creator: mdb
//
// cErrorTable --   singleton derived from Resource_<> that serves as the global
//                  error id to string mapping
//
#ifndef __ERRORTABLE_H
#define __ERRORTABLE_H

#ifndef __TSS_RESOURCES_H
#include "resources.h"
#endif
#ifndef __ERROR_H
#include "error.h"
#endif

class eError;
//-----------------------------------------------------------------------------
// cErrorTable
//-----------------------------------------------------------------------------
class cErrorTable : public cMessages_<uint32_t, TCHAR>
{
public:
    typedef cMessages_<uint32_t, TCHAR> inherited;

    //
    // Convenience Methods
    //
    void Put(const eError& e, const TCHAR* msg);
    //
    // Singleton Interface
    //
    static cErrorTable* GetInstance();

private:
#ifdef DEBUG
    static void AssertMsgValidity(const TCHAR* msg);
#endif
};

inline void cErrorTable::Put(const eError& e, const TCHAR* msg)
{
#ifdef DEBUG
    AssertMsgValidity(msg);
#endif

    inherited::Put(e.GetID(), msg);
}

//-----------------------------------------------------------------------------
// Convenience Macros
//-----------------------------------------------------------------------------
//
// These macros make it easy for a module to register errors with the global
//      error table. Them like this:
//
//      // animalerrors.h
//      //
//      TSS_DECLARE_ERROR_REGISTRATION( animal )
//
//      // animalerrors.cpp
//      //
//      TSS_BEGIN_ERROR_REGISTRATION( animal )
//      TSS_REGISTER_ERROR( eDog,       _T("Dog error") )
//      TSS_REGISTER_ERROR( eDogBark,   _T("Barking error") )
//      TSS_END_ERROR_REGISTRATION()
//
//      // pkg.h
//      TSS_DeclarePackage( cWorld )
//
//      // pkg.cpp
//      cWorld::cWorld()
//      {
//          TSS_REGISTER_PKG_ERRORS( animal )
//
//===================
// cpp file macros
//===================
#    define TSS_BEGIN_ERROR_REGISTRATION(pkgName)          \
        RegisterErrors##pkgName::RegisterErrors##pkgName() \
        {

#    define TSS_REGISTER_ERROR(err, str) cErrorTable::GetInstance()->Put(err, str);

#    define TSS_END_ERROR_REGISTRATION() }

//===================
// h file macros
//===================
#    define TSS_DECLARE_ERROR_REGISTRATION(pkgName) \
        struct RegisterErrors##pkgName              \
        {                                           \
            RegisterErrors##pkgName();              \
        };

//===================
// package init macros
//===================
#    define TSS_REGISTER_PKG_ERRORS(pkgName) RegisterErrors##pkgName register##pkgName;


#endif //__ERRORTABLE_H
