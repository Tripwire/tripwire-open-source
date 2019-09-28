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
////////////////////////////////////////////////////////////////////////
// unixfsservices.h
//
// cUnixFSServices implements iFSServices, which abstacts out all
// system dependent filesystem calls.

#ifndef __UNIXFSSERVICES_H
#define __UNIXFSSERVICES_H

//=========================================================================
// INCLUDES
//=========================================================================

#ifndef __FSSERVICES_H
#include "core/fsservices.h"
#endif

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

class cUnixFSServices : public iFSServices
{
    ///////////////////////////////////////////////////////////////
    // MEMBER FUNCTIONS
    ///////////////////////////////////////////////////////////////
public:
    cUnixFSServices();
    virtual ~cUnixFSServices();


    ////////////////////////////////////////
    // platform specific functions
    ////////////////////////////////////////
    virtual bool IsCaseSensitive() const;
    // returns true if the file system is case sensitive
    virtual const TCHAR* GetStandardBackupExtension() const;
    // returns normal string to append to backup files for this os.
    // (e.g. "~" for unix and ".bak" for winos)
    virtual TCHAR GetPathSeparator() const;

    ////////////////////////////////////////
    // process functions
    ////////////////////////////////////////
    virtual void Sleep(int nSeconds) const;
    // makes the current process sleep for the specified number of seconds

    ////////////////////////////////////////
    // major filesystem functions
    ////////////////////////////////////////
    virtual void Stat(const TSTRING& strFileName, cFSStatArgs& pStat) const;
    // fills out the cFSStatArgs structure with the stat info for the named file

    virtual void GetTempDirName(TSTRING& strName) const;
    // makes directory if it doesn't exist already.  Dirname will end with a delimiter ( '/' )

    virtual void SetTempDirName(TSTRING& tmpName);
    // set the default dir name which GetTempDirName will use...

    virtual TSTRING& MakeTempFilename(TSTRING& strName) const;
    // create temporary file
    //      strName must have the form ("baseXXXXXX"), where the X's are replaced with
    //      characters to make it a unique file.  There must be at least 6 Xs.


    ////////////////////////////////////////
    // minor filesystem functions
    ////////////////////////////////////////
    virtual void GetHostID(TSTRING& name) const;

    virtual void GetMachineName(TSTRING& name) const;

    virtual void GetMachineNameFullyQualified(TSTRING& name) const;

    virtual bool GetCurrentUserName(TSTRING& tstrName) const;

    virtual bool GetIPAddress(uint32_t& uiIPAddress);


    ////////////////////////////////////////
    // directory specific functions
    ////////////////////////////////////////
    virtual void ReadDir(const TSTRING& strName, std::vector<TSTRING>& vDirContents, bool bFullPaths = true) const;
    // puts the contents of the specified directory, except for . and .., into the supplied vector.
    virtual void GetCurrentDir(TSTRING& strCurDir) const;
    // returns the current working directory


    ////////////////////////////////////////
    // file specific functions
    ////////////////////////////////////////
    virtual bool FileDelete(const TSTRING& name) const;

    ////////////////////////////////////////
    // directory and file functions
    ////////////////////////////////////////
    virtual bool Rename(const TSTRING& strOldName, const TSTRING& strNewName, bool fOverWrite = true) const;
    // rename a file

    virtual bool GetUserName(uid_t user_id, TSTRING& tstrUser) const;
    virtual bool GetGroupName(gid_t group_id, TSTRING& tstrGroup) const;

    //Set whether we try to resolve uid/gid to a name, since Linux static binaries can
    //have trouble (read: segfaulting) with name resolution given the right nsswitch.conf setup.
    //This defaults to true if not specified.
    virtual void SetResolveNames(bool resolve);

    ////////////////////////////////////////
    // miscellaneous utility functions
    ////////////////////////////////////////
    virtual void ConvertModeToString(uint64_t perm, TSTRING& tstrPerm) const;
    // takes a int64_t permission (from stat) and changes it to look like UNIX's 'ls -l' (e.g. drwxrwxrwx)
    virtual bool FullPath(TSTRING& fullPath, const TSTRING& relPath, const TSTRING& pathRelFrom = _T("")) const;
    // converts relPath into a fully qualified path, storing it in FullPath. If this
    // fails, false is returned.  if the path to which relPath is relative is not CWD, put it in pathRelFrom.
    virtual bool GetExecutableFilename(TSTRING& strFullPath, const TSTRING& strFilename) const;
    // get the path to the current executable file
    virtual bool IsRoot(const TSTRING& strPath) const;
    // returns true if strPath is all '/'s

    ////////////////////////////////////////
    // error functions
    ////////////////////////////////////////
    virtual TSTRING GetErrString() const;

private:
    TSTRING mTempPath;
    bool    mResolveNames;
};

#endif //__UNIXFSSERVICES_H
