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
// fileutil.cpp
//
#include "stdutil.h"
#include "fileutil.h"

#include "core/corestrings.h"
#include "utilstrings.h"

#include "core/usernotify.h"
#include "core/fsservices.h"
#include "core/error.h"
#include "core/errorgeneral.h"
#include "core/file.h"
#include "core/errorbucketimpl.h"

#include <fstream> // for the FileExists() stuff

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

///////////////////////////////////////////////////////////////////////////////
// TestFileExists
///////////////////////////////////////////////////////////////////////////////
void cFileUtil::TestFileExists(const TSTRING& fileName)
{
    if (!cFileUtil::FileExists(fileName))
    {
        TSTRING filenameText = TSS_GetString(cCore, core::STR_ERROR_FILENAME);
        filenameText.append(fileName);
        filenameText.append(1, _T('\n'));
        filenameText.append(iFSServices::GetInstance()->GetErrString());

        throw eOpen(filenameText);
    }
}

///////////////////////////////////////////////////////////////////////////////
// TestFileWritable
///////////////////////////////////////////////////////////////////////////////
void cFileUtil::TestFileWritable(const TSTRING& fileName)
{
    if (!cFileUtil::FileWritable(fileName))
    {
        TSTRING filenameText = TSS_GetString(cCore, core::STR_ERROR_FILENAME);
        filenameText.append(fileName);
        filenameText.append(1, _T('\n'));
        filenameText.append(iFSServices::GetInstance()->GetErrString());

        throw eOpenWrite(filenameText);
    }
}

///////////////////////////////////////////////////////////////////////////////
// TestFileWritable
///////////////////////////////////////////////////////////////////////////////
void cFileUtil::TestFileReadable(const TSTRING& fileName)
{
    if (!cFileUtil::FileReadable(fileName))
    {
        TSTRING filenameText = TSS_GetString(cCore, core::STR_ERROR_FILENAME);
        filenameText.append(fileName);
        filenameText.append(1, _T('\n'));
        filenameText.append(iFSServices::GetInstance()->GetErrString());

        throw eOpenRead(filenameText);
    }
}

///////////////////////////////////////////////////////////////////////////////
// IsDir
///////////////////////////////////////////////////////////////////////////////
bool cFileUtil::IsDir(const TSTRING& fileName)
{
    cFSStatArgs s;
    try
    {
        iFSServices::GetInstance()->Stat(fileName, s);
    }
    catch (const eFSServices&)
    {
        return false;
    }

    return (s.mFileType == cFSStatArgs::TY_DIR);
}

bool cFileUtil::IsRegularFile(const TSTRING& fileName)
{
    cFSStatArgs s;
    try
    {
        iFSServices::GetInstance()->Stat(fileName, s);
    }
    catch (const eFSServices&)
    {
        return false;
    }
    return (s.mFileType == cFSStatArgs::TY_FILE);
}

///////////////////////////////////////////////////////////////////////////////
// FileExists
///////////////////////////////////////////////////////////////////////////////
bool cFileUtil::FileExists(const TSTRING& fileName)
{
    return ((_taccess(fileName.c_str(), F_OK) == 0) && !cFileUtil::IsDir(fileName));
}

///////////////////////////////////////////////////////////////////////////////
// FileReadable
///////////////////////////////////////////////////////////////////////////////
bool cFileUtil::FileReadable(const TSTRING& fileName)
{
    // TODO:BAM on Win32 systems, _taccess doesn't check DACL,
    // so all files are really readable
    return (_taccess(fileName.c_str(), R_OK) == 0);
}

///////////////////////////////////////////////////////////////////////////////
// FileWritable
///////////////////////////////////////////////////////////////////////////////
bool cFileUtil::FileWritable(const TSTRING& fileName)
{
    // if the file does not exist, but could be
    // created then return true
    if (_taccess(fileName.c_str(), F_OK) != 0)
    {
        // After some other attempts to see if we could create
        // the file, it became easier to just to create the file
        // and check for failure.
        int fh;
        if ((fh = _tcreat(fileName.c_str(), 0777)) == -1)
            return false;

        close(fh);
        if (_tunlink(fileName.c_str()) != 0)
            ASSERT(false);

        return true;
    }
    else // file exists; make sure it is writable and not a dir
    {
        return ((_taccess(fileName.c_str(), W_OK) == 0) && !cFileUtil::IsDir(fileName));
    }
}

///////////////////////////////////////////////////////////////////////////////
// BackupFile
//
// Renames the file, if it exists, to an appropiate backup name for the OS.
// Prints warning and returns false if file backup fails, otherwise returns true.
// An eFile(ERR_FILE_WRITE_FAILED) will be thrown if the file is not writable.
// This is because we don't want to overwrite read-only files, which would happen
// if we were just renamed the file to a backup filename.
///////////////////////////////////////////////////////////////////////////////
bool cFileUtil::BackupFile(const TSTRING& filename, bool printWarningOnFailure) // throw eFile()
{
    // if the file does not exist, no worries
    if (!cFileUtil::FileExists(filename))
        return true;

    // if the file is not a regular file, it is not appropriate to back it up
    if (!cFileUtil::IsRegularFile(filename))
        return true;

    // if it is not writeable, throw error
    if (!cFileUtil::FileWritable(filename))
    {
        // if this assert goes off, you didn't check that the file was writeable
        // before calling BackupFile().
        ASSERT(false);
        throw eFileWrite(filename, iFSServices::GetInstance()->GetErrString());
    }

#if USES_DOS_DEVICE_PATH
    TSTRING backup_filename = cDosPath::BackupName(cDosPath::AsNative(filename));
#else
    TSTRING backup_filename = filename;
#endif

    backup_filename += iFSServices::GetInstance()->GetStandardBackupExtension();

    // remove the backup file if it exists.  We ingore the return value from
    // _tunlink(), problems removing the file will be caught by _trename().
    _tunlink(backup_filename.c_str());

    // back up the file, preserving permissions and ownership, if possible
    if (cFileUtil::Copy(filename.c_str(), backup_filename.c_str()) == false)
    {
        if (printWarningOnFailure && iUserNotify::GetInstance()->GetVerboseLevel() >= iUserNotify::V_NORMAL)
        {
            TSTRING estr;
            estr.assign(TSS_GetString(cUtil, util::STR_ERR2_BACKUP_FAILED1));
            estr.append(filename);
            estr.append(TSS_GetString(cUtil, util::STR_ERR2_BACKUP_FAILED2));
            estr.append(backup_filename);
            estr.append(TSS_GetString(cUtil, util::STR_ERR2_BACKUP_FAILED3));

            cErrorReporter::PrintErrorMsg(eFileUtilBackup(estr, eError::NON_FATAL | eError::SUPRESS_THIRD_MSG));
        }
    }
    return true;
}

bool cFileUtil::Copy(const TSTRING& src_path, const TSTRING& dest_path)
{
    enum
    {
        BUF_SIZE = 4096
    };
    int8_t buf[BUF_SIZE];
    int  nBytesRead;

    cFile srcFile, destFile;

    srcFile.Open(src_path.c_str());
    // Create destination file. We'll fix the permissions later.
    destFile.Open(dest_path.c_str(), cFile::OPEN_WRITE | cFile::OPEN_CREATE);

    for (int i = srcFile.GetSize(); i > 0;)
    {
        nBytesRead = srcFile.Read(buf, BUF_SIZE);
        destFile.Write(buf, nBytesRead);
        i -= nBytesRead;
    }

    struct stat srcStat;
    stat(src_path.c_str(), &srcStat);

    // restore permissions and ownership
    // don't worry if it fails. it's not mission-critical.
#if HAVE_CHMOD    
    chmod(dest_path.c_str(), srcStat.st_mode);
#endif
    
#if HAVE_CHOWN    
    chown(dest_path.c_str(), srcStat.st_uid, srcStat.st_gid);
#endif

    srcFile.Close();
    destFile.Close();

    return true;
}
