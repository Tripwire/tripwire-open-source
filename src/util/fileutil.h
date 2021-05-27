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
// fileutil.h
//
#ifndef __FILEUTIL_H
#define __FILEUTIL_H

#include "core/error.h"

TSS_EXCEPTION(eFileUtil, eError);
TSS_EXCEPTION(eFileUtilBackup, eFileUtil);

//-----------------------------------------------------------------------------
// cFileUtil
//
// collection of static utility functions related to file manipulation
//-----------------------------------------------------------------------------
//
// TODO -- remove unnecessary #includes from the top of twutil.h
//
class cFileUtil
{
public:
    //-------------------------------------------------------------------------
    // File properties
    //-------------------------------------------------------------------------

    static void TestFileExists(const TSTRING& fileName); //throw eError();
        // tests whether or not the file exists; throws eError suitable for cTWUtil::PrintErrorMsg()
    static void TestFileWritable(const TSTRING& fileName); //throw eError();
        // tests whether or not the named file can be written to; throws eError suitable for cTWUtil::PrintErrorMsg()
    static void TestFileReadable(const TSTRING& fileName); //throw eError();
        // tests whether or not the named file can be read from; throws eError suitable for cTWUtil::PrintErrorMsg()
    static bool FileExists(const TSTRING& fileName);
    // tests whether or not the file exists
    static bool FileWritable(const TSTRING& fileName);
    // tests whether or not the named file can be written to
    // if the file does not exist, but can be created this function will return true
    static bool FileReadable(const TSTRING& fileName);
    // tests whether or not the named file can be read from
    static bool IsDir(const TSTRING& fileName);
    // returns true if file is a dir
    static bool IsRegularFile(const TSTRING& fileName);
    // returns true if file is a regular file

    //-------------------------------------------------------------------------
    // File backup mechanism
    //-------------------------------------------------------------------------

    static bool BackupFile(const TSTRING& filename, bool printWarningOnFailure = true); // throw eFile()
        // Renames the file, if it exists, to an appropiate backup name for the OS.
        // Prints warning and returns false if file backup fails, otherwise returns true.
        // An eFileWrite will be thrown if the file is not writable.
        // This is because we don't want to overwrite read-only files, which would happen
        // if we were just renamed the file to a backup filename.

    //-------------------------------------------------------------------------
    // File copy mechanism
    //-------------------------------------------------------------------------
    static bool Copy(const TSTRING& src, const TSTRING& dest); // throw eFile()
        // copies the file from src to dest, preserving permissions.
};

#endif //__FILEUTIL_H
