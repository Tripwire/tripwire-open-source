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
//
// Name....: package.h
// Date....: 05/08/99
// Creator.: Robert DiFalco (rdifalco)
//
// These classes and macros make up the TSS package abstraction.
//
// In TSS, a package represent a single project file. This file
// can represent a LIB, DLL, EXE, or other source code "package".
// The package _classes_, work together to provide a single point
// of entry for all the resources a Package (such as a LIB of DLL
// module) wish to make available to itself and other packages.
// A typical example (in fact so typical that it is concretely
// modeled in the Package abstraction) are shared Message strings.
// Each package has a message table that can be accessed by other
// packages that cooperate in the same application.
//
// The major components of packaging are the package wrapper (or
// singleton) and the package representation. There may only be
// one copy of the package representation. Its members may be
// themselves singletons or shared. Using the package interface
// guarantees that all shared resources will be properely
// initialized before they are used. When creating a package
// representation you can declare other packages that your
// package depends on, ensuring that predicate packages and their
// resources will be initialized before the package that uses
// those resources.
//
// Contents of this File
//
//  TSS_Package( cPackage )
//  TSS_BeginPackage( cPackage )
//  TSS_EndPackage( cPackage )
//  TSS_ImplementPackage( cPackage )
//  TSS_Dependency( cPackage )
//
//  TSS_DECLARE_STRINGTABLE
//  TSS_GetString( cPackage, IDS )
//
//  TSS_BeginStringtable( cPackage )
//  TSS_StringEntry( IDS, "message" )
//  TSS_EndStringtable( cPackage )
//
//  TSS_BeginStringIds( package_namespace )
//  TSS_EndStringIds( package_namespace )
//
//  class cPackageBase_<CharT>
//  class cPackage_<cPackage>
//

#ifndef __PACKAGE_H
#define __PACKAGE_H

#include "resources.h" // for: cMessage_<KEY,CHAR>


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Macro helpers
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//--PACKAGE Helpers

#define TSS_Package(pkg) pkg::GetInstance() // Access "the" Package obj

#define TSS_Dependency(pkg) TSS_Package(pkg) // Declare a Package Depend.


#define TSS_BeginPackage(pkg)               \
    class pkg : public cPackageBase_<TCHAR> \
    {                                       \
    public:                                 \
        static pkg& GetInstance();


#define TSS_EndPackage(pkg) \
    }                       \
    ;

#define TSS_ImplementPackage(pkg)         \
    pkg& pkg::GetInstance()               \
    {                                     \
        static bool bConstructed = false; \
        static pkg  x;                    \
        if (!bConstructed)                \
        {                                 \
            bConstructed = true;          \
            x.LoadStrings();              \
        }                                 \
        return x;                         \
    }


//--STRINGTABLE Helperfs

#define TSS_GetString(pkg, id) TSS_Package(pkg).GetString(id) // Access the Message String


#define TSS_DECLARE_STRINGTABLE                                \
public:                                                        \
    Messages::String GetString(Messages::ConstKeyRef id) const \
    {                                                          \
        return m_messages.Get(id);                             \
    }                                                          \
    void LoadStrings();                                        \
                                                               \
private:                                                       \
    Messages m_messages // Decare a Stringtable


#ifdef DEBUG
#    define TSS_BeginStringtable(pkg)                                \
        void pkg::LoadStrings()                                      \
        {                                                            \
            cDebug d(#pkg "::LoadStrings()");                        \
            d.TraceDebug("Loading strings for " #pkg " package.\n"); \
            Messages::Pair astr[] = { // Define a Stringtable
#else                                 // DEBUG
#    define TSS_BeginStringtable(pkg) \
        void pkg::LoadStrings()       \
        {                             \
            Messages::Pair astr[] = { // Define a Stringtable
#endif                                // DEBUG

#define TSS_EndStringtable(pkg)                 \
    }                                           \
    ;                                           \
    m_messages.Put(astr, astr + countof(astr)); \
    } // End define Strintable


#define TSS_StringEntry(id, s) Messages::Pair(id, s) // Stringtable Entry

#define TSS_BeginStringIds(pns) \
    namespace pns               \
    {                           \
    enum                        \
    { // Define String IDs

#define TSS_EndStringIds(pns) \
    }                         \
    ;                         \
    } // End define String IDs


//=============================================================================
// cPackageBase_<CharT> -- Base class for all Package Resources
//-----------------------------------------------------------------------------
//  SYNOPSIS:
//      This class is the base class for all package representations
//      and, thus, establishes the package contract. It's interface
//      is relied on by the Package singleton wrapper, TSS_Package.
//      Since part of its contract is that there is only one package
//      instance per package, none of its data members need be static
//      allowing the client to assume that its constructor will only
//      be called once.
//
//  CONSTRAINTS:
//      A Package representation template must be instantiated with a
//      "character concept" that is a valid STDCPP NTCTS.
//      This will most often be char (for NTBS), wchar_t (for NTWCS),
//      but may also be unsigned char (for NTMBCS).
//
//  INVARIANTS:
//      m_nInstances <= 1
//
//

template<class CharT> class cPackageBase_
{
public:
    typedef cMessages_<int, CharT> Messages;

    void LoadStrings()
    {
    }
};


#endif //__PACKAGE_H
