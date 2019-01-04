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
// Name....: resources.h
// Date....: 05/06/99
// Creator.: Robert DiFalco (rdifalco)
//
// Abstraction for a keyed collection of resources including a
// specialization for dealing with message resources.
//

#ifndef __RESOURCES_H
#define __RESOURCES_H


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Dependencies
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include <map>     // for: Resource Table Implementation
#include "debug.h" // for: ASSERT and Trace
#include "error.h" // for: eInternal


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Classes Declared in this module
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template<class KEY, class RSC> class cResources_;
template<class KEY, class CHR> class cMessages_;


//=============================================================================
// cResources_<KEY,REZ> -- Keyed collection of resources
//-----------------------------------------------------------------------------
// SYNOPSIS:
//  Used to maintain a table of resources that are indexed by KEY
//  type values. The <REZ> type can be any valid type that is
//  assignable.
//
// CONSTRAINTS:
//  <KEY> must be a valid key type for the std::map concept
//  <REZ> must be a valid value type for the std::map concept.
//
// INVARIANTS:
//
//
template<class KeyT, class RscT> class cResources_
{
    //--Type Definitions

private:
    typedef std::map<KeyT, RscT> Table; // Storage Rep


public:
    // Type definitions

    typedef KeyT        Key; // The resource id
    typedef KeyT&       KeyRef;
    typedef const KeyT& ConstKeyRef;

    typedef RscT         Value; // The managed resource
    typedef Value&       ValueRef;
    typedef const Value& ConstValueRef;

    typedef typename Table::value_type Pair;


    //--Interface.

public:
    // Observers

    ConstValueRef Get(ConstKeyRef) const;


    // Modifiers

    void Put(const Pair*, const Pair*);
    void Put(ConstKeyRef, Value);


    //--Implementation.

protected:
    static ConstValueRef DefaultValueRef(); // NOTE:

private:
    Table m_table;
};


//=============================================================================
// cMessages_<KEY,CHAR> -- Specialization of Resources_ for message strings
//-----------------------------------------------------------------------------
//  SYNOPSIS:
//      Specialization of cResources_<KEY,REZ> to store message strings
//      instead of generalized resource objects. In the future this can
//      be refactored to use message objects so that they can come from
//      a dynamic  source (i.e., not a string-literal) such as persistant
//      storage or some algorithm based on the concatenation of elements.
//      The cMessages_<KEY,CHAR> abstraction allows a program to be
//      character type independent so that it may use ANSI, MBCS, or
//      UNICODE within the same application instance. This is achieved by
//      not allowing the client to expect a specific character sequence
//      representation, such as the near ubiquitous const char*. For
//      example:
//
//          const char* psz = messages.GetAnsi( IDS_SOMESTRING );
//
//      The above call requires the caller to be responsible for memory
//      resources in the case of a conversion. However, there is no
//      clear way to know up front how many bytes will be required.
//      Another equally unsatisfactory solution is to allocate the
//      memory within cMessages and require the caller to release the
//      the resources when they are done with the message. If we instead
//      require calls in the following form:
//
//          std::string str = messages.GetAnsi( IDS_SOMESTRING );
//
//      We can allows assume proper resource use regardless of whether
//      a simple character pointer is returned or newly allocated
//      resources used to convert the stored message.
//
//
//  CONSTRAINTS:
//      As with all classes which manipulate character sequences, CHAR
//      must be a valid character type as defined by the STDCPP standard.
//
//
template<class KeyT, class CharT> class cMessages_ : public cResources_<KeyT, const CharT*>
{
    //--Type definitions

protected:
    typedef cResources_<KeyT, const CharT*> Resources;

public:
    typedef std::basic_string<CharT> String;
    typedef const KeyT&              ConstKeyRef;


    //--Interface.

public:
    String      Get(ConstKeyRef) const;
    std::string GetAnsi(ConstKeyRef) const;
#if SUPPORTS_WCHART
    std::wstring GetWide(ConstKeyRef) const;
#endif //SUPPORTS_WCHART
};


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// IMPLEMENTATION: cResources_<KEY_TYPE, RESOURCE_TYPE>
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// NOTE: Macros to ease any changes in template arguments

#define Resource_Template template<class KeyT, class RscT>
#define Resource_Class cResources_<KeyT, RscT>


Resource_Template void Resource_Class::Put(const Pair* beg, const Pair* end)
{
    for (const Pair* at = beg; at != end; ++at)
    {
        if (!m_table.insert(*at).second)
        {
            ASSERTMSG(0, "Resource key already exists!");
            // throw eInternal( "Resource key already exists!" );
        }
    }
}

// By value allows the user to Resource_table.Put( 6, "Literal" );
Resource_Template inline void Resource_Class::Put(ConstKeyRef id, Value sResource)
{
    if (m_table.insert(Pair(id, sResource)).second == false)
    {
        ASSERTMSG(0, "Resource key already exists!");
        // throw eInternal( "Resource key already exists!" );
    }
}

Resource_Template typename Resource_Class::ConstValueRef Resource_Class::Get(ConstKeyRef id) const
{
    typename Table::const_iterator at = m_table.find(id);

    if (at == m_table.end())
    {
#ifdef DEBUG
        std::cerr << "*** Error Resource_Class::Get() [" __FILE__ ":" << __LINE__ << "]: Resource not found\n";
#endif

        return DefaultValueRef();
    }
    else
    {
        return (*at).second;
    }
}

Resource_Template typename Resource_Class::ConstValueRef Resource_Class::DefaultValueRef()
{
    static Value _default;
    return _default;
}


#undef Resource_Template //template< class KeyT, class RscT >
#undef Resource_Class    //Resources_<KeyT,RscT>


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// IMPLEMENTATION: cMessages_<KEY_TYPE, RESOURCE_TYPE>
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// NOTE: Macros to ease any changes in template arguments

#define Message_Template template<class KeyT, class CharT>
#define Message_Class cMessages_<KeyT, CharT>


Message_Template inline typename Message_Class::String Message_Class::Get(ConstKeyRef id) const
{
    typename Message_Class::Value msg = Resources::Get(id);
    return (msg == this->DefaultValueRef()) ? String() : String(msg);
}


/// NOTE: Can't have anonymous NS in Headers

namespace tss
{
namespace rsc
{
/* WARNING:RAD -- Cannot use StringUtil or CodeConvert since
        these use ErrorMessages and String Resources! */

inline void ToNTMBS(std::string& nbs, const std::string& src)
{
    nbs.assign(src);
}

#if SUPPORTS_WCHART

inline void ToNTMBS(std::string& nbs, const std::wstring& wcs)
{
    nbs.resize(MB_CUR_MAX * wcs.size());

    size_t nWrote = ::wcstombs(const_cast<char*>(nbs.c_str()), wcs.c_str(), nbs.size() + 1);

    if (nWrote == (size_t)-1)
    {
        ASSERTMSG(false, "Conversion error in cMessages_!");
        //          throw eCharacterEncoding( TSS_GetString( cCore, core::STR_ERR_BADCHAR ) );
    }

    nbs.resize(nWrote);
}

inline void ToNTWCS(std::wstring& dst, const std::wstring& src)
{
    dst.assign(src);
}

inline void ToNTWCS(std::wstring& wcs, const std::string& nbs)
{
    wcs.resize(nbs.size());

    size_t nWrote = ::mbstowcs(const_cast<wchar_t*>(wcs.c_str()), nbs.c_str(), wcs.size() + 1);

    if (nWrote == (size_t)-1)
    {
        ASSERTMSG(false, "Conversion error in cMessages_!");
        //          throw eCharacterEncoding( TSS_GetString( cCore, core::STR_ERR_BADCHAR ) );
    }

    wcs.resize(nWrote);
}
#endif //SUPPORTS_WCHART
} // namespace rsc
} // namespace tss


Message_Template inline std::string Message_Class::GetAnsi(ConstKeyRef id) const
{
    typename Message_Class::Value msg = Resources::Get(id);

    if (msg == this->DefaultValueRef())
    {
        return std::string();
    }
    else
    {
        std::string nbs;
        tss::rsc::ToNTMBS(nbs, String(msg));
        return nbs;
    }
}


#if SUPPORTS_WCHART

Message_Template inline std::wstring Message_Class::GetWide(ConstKeyRef id) const
{
    Value msg = Resources::Get(id);

    if (msg == DefaultValueRef())
    {
        return std::string();
    }
    else
    {
        std::wstring wcs;
        tss::rsc::ToNTWCS(wcs, String(msg));
        return wcs;
    }
}

#endif //SUPPORTS_WCHART


#undef Message_Template
#undef Message_Class

#endif //__RESOURCES_H


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OLD CODE * OLD CODE * OLD CODE * OLD CODE * OLD CODE * OLD CODE * OLD CODE
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef RAD_OLDCODE

//=============================================================================
// cMessages_<char & wchar_t> -- Specializations
//-----------------------------------------------------------------------------
//  SYNOPSIS:
//      MSVC does not yet support specializations. As a compromise, we fully
//      specialize on TCHR but assume a key type of "const int".
//

// for SUNPro enums are ints, not const ints
#if IS_SUNPRO
#    define ENUM_TYPE int
#else
#    define ENUM_TYPE const int
#endif

template<> class cMessages_<ENUM_TYPE, char> : public cResources_<ENUM_TYPE, const char*>
{
protected:
    typedef cResources_<ENUM_TYPE, const char*> Resources;

public:
    typedef std::string String;
    typedef int         ConstKeyRef; // Better than <const int&>


    String Get(ConstKeyRef id) const
    {
        Value msg = Resources::Get(id);
        return (msg != DefaultValueRef()) ? String(msg) : String();
    }

    String GetAnsi(ConstKeyRef id) const
    {
        return Get(id);
    }

#if SUPPORTS_WCHART
    std::wstring GetWide(ConstKeyRef id) const
    {
        Value msg = Resources::Get(id);

        if (msg == DefaultValueRef())
        {
            return std::wstring();
        }
        else
        {
            String::size_type N = String::traits_type::length(msg);

            std::wstring dst(N, 0x00);
            std::ctype<wchar_t>().widen(msg, msg + N, dst.begin());

            return dst;
        }
    }
#endif //SUPPORTS_WCHART
};

#if SUPPORTS_WCHART

template<> class cMessages_<const int, wchar_t> : public cResources_<const int, const wchar_t*>
{
protected:
    typedef cResources_<const int, const wchar_t*> Resources;

public:
    typedef std::wstring String;
    typedef int          ConstKeyRef; // Better than <const int&>


    String Get(ConstKeyRef id) const
    {
        Value msg = Resources::Get(id);
        if (msg != DefaultValueRef())
            return String(msg);
        else
            return String();
    }

    std::string GetAnsi(ConstKeyRef id) const
    {
        Value msg = Resources::Get(id);

        if (msg == DefaultValueRef())
        {
            return std::string();
        }
        else
        {
            String::size_type N = String::traits_type::length(msg);

            std::string dst(N, 0x00);
            std::ctype<CharT>().narrow(msg, msg + N, '?', dst.begin());

            return dst;
        }
    }

    String GetWide(ConstKeyRef id) const
    {
        return Get(id);
    }
};

#    endif //SUPPORTS_WCHART

#endif //RAD_OLDCODE
