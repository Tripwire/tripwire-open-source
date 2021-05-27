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
// configfile.h
//

//=========================================================================
// INCLUDES
//=========================================================================

#ifndef __CONFIGFILE_H
#define __CONFIGFILE_H

#ifndef __TCHAR_H
#include "core/tchar.h"
#endif //__TCHAR_H

#ifndef __ERROR_H
#include "core/error.h"
#endif //__ERROR_H

#ifndef __HASHTABLE_H
#include "core/hashtable.h"
#endif //__HASHTABLE_H

#ifndef __FSSERVICES_H
#include "core/fsservices.h"
#endif

#ifndef __DEBUG_H
#include "core/debug.h"
#endif //__DEBUG_H

//=========================================================================
// DEFINES AND MACROS
//=========================================================================

//=========================================================================
// GLOBALS
//=========================================================================

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

class eFSServices;
class cElGamalSigPrivateKey;
class cElGamalSigPublicKey;
class cFileHeaderID;

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

TSS_EXCEPTION(eConfigFile, eError);
TSS_EXCEPTION(eConfigFileNoEq, eConfigFile);
TSS_EXCEPTION(eConfigFileNoKey, eConfigFile);
TSS_EXCEPTION(eConfigFileAssignToPredefVar, eConfigFile);
TSS_EXCEPTION(eConfigFileUseUndefVar, eConfigFile);
TSS_EXCEPTION(eConfigFileMissReqKey, eConfigFile);
TSS_EXCEPTION(eConfigFileEmptyVariable, eConfigFile);
TSS_EXCEPTION(eConfigFileMissingRightParen, eConfigFile);

///////////////////////////////////////////////////////////////////////////////
// cConfigFile
//
// use this class read and write key/value pairs to and from a file.
// it reads ([\r\n]+)-separated "key=value" pairs from a file
// it writes pairs "Insert"ed into it
// comments are specified by lines that match /$\s*#/ -- note that this does
// not allow for comments after a name/value pair.
///////////////////////////////////////////////////////////////////////////////
class cConfigFile
{
public:
    cConfigFile();
    virtual ~cConfigFile();

    bool Lookup(const TSTRING& tstrKey, TSTRING& tstrVal) const;
    // returns true if key is found in internal container and returns its value in tstrVal.

    void Insert(const TSTRING& tstrKey, const TSTRING& tstrVal);
    // add key+value to config data. visible for unit testing.

    void WriteString(TSTRING& configText);
    // writes all key/value pairs from internal container to filename as "name=value\n"
    void ReadString(const TSTRING configText); // throw( eConfigFile );
        // reads "name=value[\r\n]+" from file and puts all pairs into internal container

    static const cFileHeaderID& GetFileHeaderID();
    // Return a cFileHeaderID for all databases

private:
    void GetKeyValuePair(const TSTRING& tstrLine, TSTRING& tstrKey, TSTRING& tstrVal); // throw (eConfigFile)
    bool IsPredefinedVar(const TSTRING& var, TSTRING& val) const;
    void DoVarSubst(TSTRING& rval) const;       //throw( eConfigFile )
    void CheckThatAllMandatoryKeyWordsExists(); // throw( eConfigFile )

    TSTRING MakeErrorString(const TSTRING& strMsg = _T(""), bool fShowLineNum = true) const;
    // conststructs an error string. Appends "Line number: X"
    // if fShowLineNum is true
    // This string can then be passed to the exception ctor.

    cHashTable<TSTRING, TSTRING> mStringHashTable;
    friend class cConfigFileIter;
    int mnLine;
};


///////////////////////////////////////////////////////////////////////////////
// cConfigFileIter -- iterate over the entries in the config file
//
// NOTE -- this class is not tested at all!
///////////////////////////////////////////////////////////////////////////////
class cConfigFileIter
{
public:
    explicit cConfigFileIter(cConfigFile& cf);
    virtual ~cConfigFileIter();

    void           SeekBegin() const;
    bool           Done() const;
    void           Next() const;
    const TSTRING& Key() const;
    TSTRING&       Val();

protected:
    cHashTableIter<TSTRING, TSTRING> mIter;
};

//-----------------------------------------------------------------------------
// inline implementation
//-----------------------------------------------------------------------------
inline cConfigFileIter::cConfigFileIter(cConfigFile& cf) : mIter(cf.mStringHashTable)
{
    mIter.SeekBegin();
}

inline cConfigFileIter::~cConfigFileIter()
{
}

inline void cConfigFileIter::SeekBegin() const
{
    mIter.SeekBegin();
}

inline bool cConfigFileIter::Done() const
{
    return mIter.Done();
}

inline void cConfigFileIter::Next() const
{
    mIter.Next();
}

inline const TSTRING& cConfigFileIter::Key() const
{
    ASSERT(!Done());
    return mIter.Key();
}

inline TSTRING& cConfigFileIter::Val()
{
    ASSERT(!Done());
    return mIter.Val();
}


#endif //__CONFIGFILE_H
