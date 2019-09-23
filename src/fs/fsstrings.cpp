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
// Name....: fsstrings.cpp
// Date....: 05/05/99
// Creator.: Brian McFeely (bmcfeely)
//

#include "stdfs.h"
#include "fs.h"
#include "fsstrings.h"

TSS_BeginStringtable(cFS)

    TSS_StringEntry(fs::STR_FT_INVALID,       _T("Invalid")),
    TSS_StringEntry(fs::STR_FT_FILE,          _T("Regular File")),
    TSS_StringEntry(fs::STR_FT_DIR,           _T("Directory")),
    TSS_StringEntry(fs::STR_FT_BLOCKDEV,      _T("Block Device")),
    TSS_StringEntry(fs::STR_FT_CHARDEV,       _T("Character Device")),
    TSS_StringEntry(fs::STR_FT_SYMLINK,       _T("Symbolic Link")),
    TSS_StringEntry(fs::STR_FT_FIFO,          _T("FIFO")),
    TSS_StringEntry(fs::STR_FT_SOCK,          _T("Socket")),
    TSS_StringEntry(fs::STR_FT_DOOR,          _T("Door")),
    TSS_StringEntry(fs::STR_FT_PORT,          _T("Event Port")),
    TSS_StringEntry(fs::STR_FT_NAMED,         _T("Named Special File")),
    TSS_StringEntry(fs::STR_FT_NATIVE,        _T("Native Object")),
    TSS_StringEntry(fs::STR_FT_MESSAGE_QUEUE, _T("Message Queue")),
    TSS_StringEntry(fs::STR_FT_SEMAPHORE,     _T("Semaphore")),
    TSS_StringEntry(fs::STR_FT_SHARED_MEMORY, _T("Shared Memory")),
  
    // property names
    TSS_StringEntry(fs::STR_PROP_DEV, _T("Device Number")),
    TSS_StringEntry(fs::STR_PROP_RDEV, _T("File Device Number")),
    TSS_StringEntry(fs::STR_PROP_INODE, _T("Inode Number")), TSS_StringEntry(fs::STR_PROP_MODE, _T("Mode")),
    TSS_StringEntry(fs::STR_PROP_NLINK, _T("Num Links")), TSS_StringEntry(fs::STR_PROP_UID, _T("UID")),
    TSS_StringEntry(fs::STR_PROP_GID, _T("GID")), TSS_StringEntry(fs::STR_PROP_SIZE, _T("Size")),
    TSS_StringEntry(fs::STR_PROP_ATIME, _T("Access Time")), TSS_StringEntry(fs::STR_PROP_MTIME, _T("Modify Time")),
    TSS_StringEntry(fs::STR_PROP_CTIME, _T("Change Time")), TSS_StringEntry(fs::STR_PROP_BLOCK_SIZE, _T("Block Size")),
    TSS_StringEntry(fs::STR_PROP_BLOCKS, _T("Blocks")), TSS_StringEntry(fs::STR_PROP_CRC32, _T("CRC32")),
    TSS_StringEntry(fs::STR_PROP_MD5, _T("MD5")), TSS_StringEntry(fs::STR_PROP_FILETYPE, _T("Object Type")),
    TSS_StringEntry(fs::STR_PROP_GROWING_FILE, _T("Growing Object Size")), TSS_StringEntry(fs::STR_PROP_SHA, _T("SHA")),
    TSS_StringEntry(fs::STR_PROP_HAVAL, _T("HAVAL")),
    TSS_StringEntry(fs::STR_PROP_ACL, _T("ACL Placeholder -- Not Implemented")),

    /*  Leaving these here in case we ever implement long property names

    TSS_StringEntry( fs::STR_PARSER_PROP_DEV,           _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_RDEV,          _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_INODE,         _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_MODE,          _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_NLINK,         _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_UID,           _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_GID,           _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_SIZE,          _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_ATIME,         _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_MTIME,         _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_CTIME,         _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_BLOCK_SIZE,    _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_BLOCKS,        _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_FILETYPE,      _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_GROWING_FILE,  _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_CRC32,         _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_MD5,           _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_SHA,           _T("unimplemented") ),
    TSS_StringEntry( fs::STR_PARSER_PROP_HAVAL,         _T("unimplemented") ),
*/
    TSS_StringEntry(fs::STR_PARSER_READONLY, _T("ReadOnly")), TSS_StringEntry(fs::STR_PARSER_DYNAMIC, _T("Dynamic")),
    TSS_StringEntry(fs::STR_PARSER_GROWING, _T("Growing")), TSS_StringEntry(fs::STR_PARSER_IGNOREALL, _T("IgnoreAll")),
    TSS_StringEntry(fs::STR_PARSER_IGNORENONE, _T("IgnoreNone")), TSS_StringEntry(fs::STR_PARSER_DEVICE, _T("Device")),
    TSS_StringEntry(fs::STR_PARSER_HOSTNAME, _T("HOSTNAME")),

    TSS_StringEntry(fs::STR_FS_PARSER_READONLY_VAL, _T("+pinugsmtdbCM-raclSH" )),
    TSS_StringEntry(fs::STR_FS_PARSER_DYNAMIC_VAL, _T("+pinugtd-rsacmblCMSH" )),
    TSS_StringEntry(fs::STR_FS_PARSER_GROWING_VAL, _T("+pinugtdl-rsacmbCMSH" )),
    TSS_StringEntry(fs::STR_FS_PARSER_IGNOREALL_VAL, _T("-pinusgamctdrblCMSH" )),
    TSS_StringEntry(fs::STR_FS_PARSER_IGNORENONE_VAL, _T("+pinusgamctdrbCMSH-l" )),
    TSS_StringEntry(fs::STR_FS_PARSER_DEVICE_VAL, _T("+pugsdr-intlbamcCMSH" )),
    TSS_StringEntry(fs::STR_FS_PARSER_HOSTNAME_VAL, _T("localhost" )),

    TSS_StringEntry(fs::STR_DIFFERENT_FILESYSTEM, _T("The object: \"%s\" is on a different file system...ignoring.\n")),

    TSS_EndStringtable(cFS)
