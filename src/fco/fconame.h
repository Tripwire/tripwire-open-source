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
// fconame.h
//
// cFCOName -- represents the name of an FCO
#ifndef __FCONAME_H
#define __FCONAME_H

#ifndef __SERIALIZABLE_H
#include "core/serializable.h"
#endif
#ifndef __FCONAMEINFO_H
#include "fconameinfo.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// class cFCOName -- How we name a FCO.  The implementation has ended up being
//      a wrapper around a standard string.
///////////////////////////////////////////////////////////////////////////////
class cFCONameTblNode;
class cFCONameIter;
class cFCOName_i;

class cFCOName : public iTypedSerializable
{
    DECLARE_TYPEDSERIALIZABLE()

public:
    typedef cFCONameIter iterator;

    explicit cFCOName(iFCONameInfo* iNI = NULL);
    cFCOName(const cFCOName& rhs);

    explicit cFCOName(const TSTRING& rhs, iFCONameInfo* iNI = NULL);
    explicit cFCOName(const TCHAR* rhs, iFCONameInfo* iNI = NULL);
    // these ctors are explicit because it is expensive to create these things, so we don't want to
    // do it unexpectedly
    virtual ~cFCOName();

    cFCOName& operator=(const cFCOName& rhs);
    cFCOName& operator=(const TSTRING& rhs);
    cFCOName& operator=(const TCHAR* rhs);

    bool operator<(const cFCOName& rhs) const;
    // defines an arbitrary order for cFCONames. This is so that names can
    // be stored in trees and other ordered data structures in order to facilitate
    // fast lookups.

    bool operator==(const cFCOName& rhs) const;
    bool operator!=(const cFCOName& rhs) const;
    bool IsEqual(const cFCOName& rhs) const;
    // both of these are synonyms for (GetRelationship(rhs) == REL_EQUAL)

    TSTRING AsString() const;
    // return a string representation of the string

    bool IsCaseSensitive() const;
    void SetCaseSensitive(bool val);
    // determines how name comparisons are done. If two cFCONames are compared who are
    // both case insensitive, a case insensitive compare is done. If both are case sensitive,
    // a case sensitive compare is done. If one is and one isn't case sensitive (? TODO: what
    // is the right thing to do in this case? ASSERT(0)? do a case sensitive compare??)
    TCHAR GetDelimiter() const;
    void  SetDelimiter(TCHAR delimiter);
    // defines what the path-delimiting character is

    void Push(const TSTRING& str);
    // pushes the named string onto the end of the path. A delimiter is implicitely placed between the old end
    // and the new string. if debug is defined, it is asserted that the new string does not contain the
    // delimiting character
    const TCHAR* Pop();
    // removes the last part of the path from the name and returns it. ASSERTS and returns an undefined value if
    // the path is empty
    const TCHAR* PopFront();
    // removes the first part of the path from the name and returns it. ASSERTS and returns an undefined value if
    // the path is empty
    void Clear();
    // removes all elements from the name; after this, GetSize() == 0.
    const TCHAR* GetShortName() const;
    // this will return the final string in the vector. It will assert that the name is not empty.
    int GetSize() const;
    // returns the number of path items (strings between delimiters) that are in the fco name. It is always equal
    // to the number of delimiters in the path plus one.

    enum Relationship
    {
        REL_BELOW,    // an fco is below another  (/etc/passwd is below /etc)
        REL_ABOVE,    // an fco is above another  (/etc is above /etc/passwd)
        REL_EQUAL,    // the fcos are equal       (/etc/passwd is equal to /etc/passwd)
        REL_UNRELATED // the fcos are unrelated   (/var/spool/mail is unrelated to /var/log/pig.log)
    };
    Relationship GetRelationship(const cFCOName& rhs) const;
    // returns the relationship of _this_ name to the one passed in (ie -- if REL_BELOW is returned,
    // this fco name is below the one passed in)

    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

    //////////////////////////////////
    // Debugging method
    //////////////////////////////////
    static void ClearNameTable();
    // this method should _only_ be called when the program is exiting, and you are positively sure you
    // will not be using or creating any more cFCONames. Violating this rule will result in unpredictable
    // results. This clears out the cFCOName internal name table, making memory leaks easier to find.
protected:
    friend class cFCOName_i;
    friend class cFCONameIter;

    void ParseString(const TCHAR* str);
    // helper function that takes the pathname pointer to by str and fills out mpPathName
    void CopyOnModify();
    // if the fconame is modified and mpPathName is shared by more than one object, this will
    // release mpPathName and create a new one, so we don't change any other object's data
    void SetNameInfo(iFCONameInfo* pNI);
    // sets the delimiter and case sensitiveness to the name info's

    cFCOName_i* mpPathName;
    TCHAR       mDelimiter;      // the delimiting character in names (ie -- in file systems, it is '/')
    bool        mbCaseSensitive; // determines whether name compares are case sensitive of not

    // this is what type of structure holds the elements of the name
    //
    typedef std::vector<cFCONameTblNode*> ListType;

#ifdef DEBUG
    TSTRING mDebugStrName; // so we can see this guy's value in the debug window
#endif
};

//-----------------------------------------------------------------------------
// cFCONameIter
//-----------------------------------------------------------------------------
class cFCONameIter_i;
class cFCONameIter
{
public:
    explicit cFCONameIter(const cFCOName& name);
    ~cFCONameIter();

    int  GetSize() const; // returns the number of entries in the fco name
    void SeekBegin();
    void Next();
    void Prev();
    bool Done() const;

    int Index() const;
    // return the current offset from the beginning of the short name list
    void SeekTo(int index);
    // seeks to the given index. asserts that the index is appropriate for the
    // current fco name.

    const TCHAR* GetName() const;

private:
    void operator=(const cFCONameIter& rhs); // not impl
    cFCONameIter(const cFCONameIter& rhs);   // not impl

    // the data...
    //
    const cFCOName&                    mName;
    cFCOName::ListType::const_iterator mIter;
};

//#############################################################################
// inline implementation
inline bool cFCOName::IsCaseSensitive() const
{
    return mbCaseSensitive;
}
inline void cFCOName::SetCaseSensitive(bool val)
{
    mbCaseSensitive = val;
}
inline TCHAR cFCOName::GetDelimiter() const
{
    return mDelimiter;
}
inline void cFCOName::SetDelimiter(TCHAR delimiter)
{
    mDelimiter = delimiter;
}

///////////////////////////////////////////////////////////////////////////////
// IsEqual
///////////////////////////////////////////////////////////////////////////////
inline bool cFCOName::IsEqual(const cFCOName& rhs) const
{
    return (GetRelationship(rhs) == cFCOName::REL_EQUAL);
}

inline bool cFCOName::operator==(const cFCOName& rhs) const
{
    return IsEqual(rhs);
}
inline bool cFCOName::operator!=(const cFCOName& rhs) const
{
    return (!IsEqual(rhs));
}


#endif //__FCONAME_H
