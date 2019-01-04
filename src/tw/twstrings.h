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
// Name....: twstrings.h
// Date....: 5/11/99
// Creator.: Matthew Brinkley (brinkley)
//
//

#ifndef __TWSTRINGS_H
#define __TWSTRINGS_H

#include "tw.h"

TSS_BeginStringIds(tw) STR_ERR_MISSING_LOCAL_KEY, STR_ERR_MISSING_SITE_KEY, STR_ERR_MISSING_POLICY, STR_ERR_MISSING_DB,
    STR_ERR_MISSING_REPORT, STR_ERR_MISSING_EDITOR, STR_GENERATING_DB, STR_ERR_WRONG_PASSPHRASE_LOCAL,
    STR_ERR_WRONG_PASSPHRASE_SITE, STR_DEF_CFG_FILENAME, STR_GET_HELP, STR_VERSION, STR_VERSION_LONG, STR_NEWLINE,
    STR_ERR_TWCFG_CANT_READ, STR_ERR_NO_MODE, STR_UNKNOWN_MODE_SPECIFIED, STR_ERR2_BAD_MODE_SWITCH, STR_PARSING_POLICY,
    STR_ERR_REPORT_READ, STR_REPORT_NOT_ENCRYPTED, STR_ERR_DB_READ, STR_CUR_LINE, STR_INIT_SUCCESS,
    STR_REPORTEVENT_FAILED, STR_SYSLOG_FAILED,

    STR_ENTER_LOCAL_PASSPHRASE, STR_ENTER_SITE_PASSPHRASE, STR_ENTER_PROVIDED_PASSPHRASE,
    STR_DB_NOT_UPDATED, // db update not performed due to secure mode
    STR_IGNORE_PROPS,   // ignoring properties
    STR_NOT_IMPLEMENTED, STR_REPORT_EMPTY, STR_FILE_WRITTEN, STR_FILE_OPEN, STR_FILE_ENCRYPTED, STR_OPEN_KEYFILE,
    STR_OPEN_CONFIG_FILE, STR_OPEN_DB_FILE, STR_OPEN_REPORT_FILE, STR_OPEN_POLICY_FILE, STR_WRITE_POLICY_FILE,
    STR_WRITE_DB_FILE, STR_WRITE_REPORT_FILE, STR_WRITE_CONFIG_FILE,

    STR_REPORT_TITLE, STR_R_GENERATED_BY, STR_R_CREATED_ON, STR_DB_CREATED_ON, STR_DB_LAST_UPDATE, STR_R_SUMMARY,
    STR_HOST_NAME, STR_HOST_IP, STR_HOST_ID, STR_POLICY_FILE_USED, STR_CONFIG_FILE_USED, STR_DB_FILE_USED,
    STR_CMD_LINE_USED, STR_SEVERITY_LEVEL, STR_RULE_NAME, STR_ALL_RULE_NAMES, STR_TOTAL_NUM_RULES, STR_START_POINT,
    STR_TOTAL_VIOLATIONS, STR_OBJECTS_SCANNED, STR_SECTION, STR_ATTRIBUTES, STR_OBSERVED, STR_EXPECTED, STR_DB_UPDATE,
    STR_RULE_SUMMARY, STR_OBJECT_SUMMARY, STR_OBJECT_DETAIL, STR_ADDED, STR_REMOVED, STR_CHANGED, STR_ADDED_FILES,
    STR_REMOVED_FILES, STR_CHANGED_FILES, STR_TOTAL_NUM_ADDED_FILES, STR_TOTAL_NUM_REMOVED_FILES,
    STR_TOTAL_NUM_CHANGED_FILES, STR_OBJECT_NAME, STR_ADDED_FILE_NAME, STR_REMOVED_FILE_NAME, STR_CHANGED_FILE_NAME,
    STR_ERROR_REPORT, STR_REPORT_NO_ERRORS, STR_NO_VIOLATIONS_IN_SECTION, STR_ADD_X, STR_UPDATE_ALL,
    STR_END_OF_REPORT, STR_NONE, STR_NEVER, STR_OBJECTS, STR_TRIPWIRE_REPORT_SHORT, STR_VIOLATIONS_SHORT,
    STR_MAX_SEV_SHORT, STR_ADDED_SHORT, STR_REMOVED_SHORT, STR_CHANGED_SHORT,

    // database strings
    STR_DBPRINT_TITLE, STR_DB_GENERATED_BY, STR_TOTAL_NUM_FILES, STR_END_OF_DB, STR_DB_SUMMARY, STR_ATTR_VALUE,

    // twutil
    STR_IP_UNKNOWN,

    // twutil
    STR_ERR_NOT_FULL_PATH, STR_ERR_OBJ_NOT_IN_DB, STR_ERR_DB_DOESNT_CONTAIN_GENRE, STR_ERR2_CFG_KEY_MISMATCH1,
    STR_ERR2_CFG_KEY_MISMATCH2, STR_ERR2_CFG_KEY_MISMATCH3, STR_ERR_CFG_UNENCRYPTED, STR_ERR_POL_UNENCRYPTED,
    STR_ERR_BACKUP_FAILED, STR_ERR2_BACKUP_FAILED1, STR_ERR2_BACKUP_FAILED2, STR_ERR2_BACKUP_FAILED3,

    // used for reporting errors when linked with console.exe or reportviewer.exe
    STR_MEMORY_MAPPED_FILENAME,

    STR_COPYRIGHT,
    STR_TSS_PRODUCT_NAME

    TSS_EndStringIds(tw)

// Get strings like this:
// TSS_GetString( cTW, tw::STR_NOTIFY_GEN_SIG ).c_str(),

#endif //__TWSTRINGS_H
