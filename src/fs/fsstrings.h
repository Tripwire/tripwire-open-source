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
// Name....: fsstrings.h
// Date....: 05/05/99
// Creator.: Brian McFeely (bmcfeely)
//

#ifndef __FSSTRINGS_H
#define __FSSTRINGS_H

#include "fs.h" // for: STRINGTABLE syntax

//--Message Keys

TSS_BeginStringIds(fs)

    // file types
    STR_FT_INVALID,
    STR_FT_FILE, STR_FT_DIR, STR_FT_BLOCKDEV, STR_FT_CHARDEV, STR_FT_SYMLINK, STR_FT_FIFO, STR_FT_SOCK, STR_FT_DOOR, STR_FT_PORT, STR_FT_NAMED, STR_FT_NATIVE,
  STR_FT_MESSAGE_QUEUE, STR_FT_SEMAPHORE, STR_FT_SHARED_MEMORY,
    // property names
    STR_PROP_DEV, STR_PROP_RDEV, STR_PROP_INODE, STR_PROP_MODE, STR_PROP_NLINK, STR_PROP_UID, STR_PROP_GID,
    STR_PROP_SIZE, STR_PROP_ATIME, STR_PROP_MTIME, STR_PROP_CTIME, STR_PROP_BLOCK_SIZE, STR_PROP_BLOCKS, STR_PROP_CRC32,
    STR_PROP_MD5, STR_PROP_FILETYPE, STR_PROP_GROWING_FILE, STR_PROP_SHA, STR_PROP_HAVAL, STR_PROP_ACL,

    /* Leaving these here in case we ever implement long property names
    STR_PARSER_PROP_DEV,
    STR_PARSER_PROP_RDEV,
    STR_PARSER_PROP_INODE,
    STR_PARSER_PROP_MODE,
    STR_PARSER_PROP_NLINK,
    STR_PARSER_PROP_UID,
    STR_PARSER_PROP_GID,
    STR_PARSER_PROP_SIZE,
    STR_PARSER_PROP_ATIME,
    STR_PARSER_PROP_MTIME,
    STR_PARSER_PROP_CTIME,
    STR_PARSER_PROP_BLOCK_SIZE,
    STR_PARSER_PROP_BLOCKS,
    STR_PARSER_PROP_FILETYPE,
    STR_PARSER_PROP_GROWING_FILE,
    STR_PARSER_PROP_CRC32,
    STR_PARSER_PROP_MD5,
    STR_PARSER_PROP_SHA,
    STR_PARSER_PROP_HAVAL,
*/
    STR_PARSER_READONLY, STR_PARSER_DYNAMIC, STR_PARSER_GROWING, STR_PARSER_IGNOREALL, STR_PARSER_IGNORENONE,
    STR_PARSER_DEVICE, STR_PARSER_HOSTNAME,

    STR_FS_PARSER_READONLY_VAL, STR_FS_PARSER_DYNAMIC_VAL, STR_FS_PARSER_GROWING_VAL, STR_FS_PARSER_IGNOREALL_VAL,
    STR_FS_PARSER_IGNORENONE_VAL, STR_FS_PARSER_DEVICE_VAL, STR_FS_PARSER_HOSTNAME_VAL,

    STR_DIFFERENT_FILESYSTEM

    TSS_EndStringIds(fs)


#endif //__FSSTRINGS_H
