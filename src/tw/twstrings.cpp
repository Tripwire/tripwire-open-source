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
// Name....: twstrings.cpp
// Date....: 5/11/99
// Creator.: Matthew Brinkley (brinkley)
//
//

#include "stdtw.h"
#include "twstrings.h"
#include "buildnum.h"

#define UNICODE_STR _T("")

#ifdef DEBUG
#    define DEBUG_STR _T("d")
#else
#    define DEBUG_STR _T("")
#endif

#define TSS_PRODUCT_NAME _T("Open Source Tripwire(R) " PACKAGE_VERSION ".")

#define TSS_COPYRIGHT_NOTICE \
    _T("Open Source Tripwire 2.4 Portions copyright 2000-2019 Tripwire, Inc.  Tripwire is a registered\n\
trademark of Tripwire, Inc. This software comes with ABSOLUTELY NO WARRANTY;\n\
for details use --version. This is free software which may be redistributed\n\
or modified only under certain conditions; see COPYING for details.\n\
All rights reserved.")


#define TSS_COPYRIGHT_NOTICE_LONG \
    _T("The developer of the original code and/or files is Tripwire, Inc.  Portions \n\
created by Tripwire, Inc. are copyright 2000-2019 Tripwire, Inc.  Tripwire is a \n\
registered trademark of Tripwire, Inc.  All rights reserved.\n\
\n\
This program is free software.  The contents of this file are subject to the \n\
terms of the GNU General Public License as published by the Free Software \n\
Foundation; either version 2 of the License, or (at your option) any later \n\
version.  You may redistribute it and/or modify it only in compliance with \n\
the GNU General Public License.\n\
\n\
This program is distributed in the hope that it will be useful.  However, \n\
this program is distributed \"AS-IS\" WITHOUT ANY WARRANTY; INCLUDING THE \n\
IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  \n\
Please see the GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License along with \n\
this program; if not, write to the Free Software Foundation, Inc., \n\
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.\n\
\n\
Nothing in the GNU General Public License or any other license to use the \n\
code or files shall permit you to use Tripwire's trademarks, service marks, \n\
or other intellectual property without Tripwire's prior written consent. \n\
\n\
If you have any questions, please contact Tripwire, Inc. at either \n\
info@tripwire.org or www.tripwire.org.")


TSS_BeginStringtable(cTW)

    TSS_StringEntry(tw::STR_ERR_MISSING_LOCAL_KEY, _T("No local key specified.")),
    TSS_StringEntry(tw::STR_ERR_MISSING_SITE_KEY, _T("No site key specified.")),
    TSS_StringEntry(tw::STR_ERR_MISSING_POLICY, _T("No policy file specified.")),
    TSS_StringEntry(tw::STR_ERR_MISSING_DB, _T("No database file specified.")),
    TSS_StringEntry(tw::STR_ERR_MISSING_REPORT, _T("No report file specified.")),
    TSS_StringEntry(tw::STR_ERR_MISSING_EDITOR,
                    _T("No text editor specified in configuration file,\n")
                    _T("command line, or in VISUAL or EDITOR environment variables.")),

    TSS_StringEntry(tw::STR_ERR_WRONG_PASSPHRASE_SITE, _T("Incorrect site passphrase.")),
    TSS_StringEntry(tw::STR_ERR_WRONG_PASSPHRASE_LOCAL, _T("Incorrect local passphrase.")),

#if defined(CONFIG_DIR)
    // NOTE:mdb if we are following the FHS, then the config file lives in /etc/tripwire and not in
    // the current working directory
    TSS_StringEntry(tw::STR_DEF_CFG_FILENAME, _T(CONFIG_DIR "/tw.cfg")),
#else
    TSS_StringEntry(tw::STR_DEF_CFG_FILENAME, _T("/etc/tw.cfg")),
#endif

    TSS_StringEntry(tw::STR_GET_HELP, _T("Use --help to get help.")), TSS_StringEntry(tw::STR_NEWLINE, _T("\n")),
    TSS_StringEntry(tw::STR_ERR_TWCFG_CANT_READ, _T("Configuration file could not be read.")),
    TSS_StringEntry(tw::STR_ERR_NO_MODE, _T("No mode specified.")),
    TSS_StringEntry(tw::STR_UNKNOWN_MODE_SPECIFIED, _T("Unknown mode specified: ")),
    TSS_StringEntry(tw::STR_ERR2_BAD_MODE_SWITCH, _T("Unrecognized mode: ")),

    TSS_StringEntry(tw::STR_OPEN_POLICY_FILE, _T("Opening policy file: ")),
    TSS_StringEntry(tw::STR_PARSING_POLICY, _T("Parsing policy file: ")),
    TSS_StringEntry(tw::STR_ERR_REPORT_READ, _T("Report file could not be read.")),
    TSS_StringEntry(tw::STR_REPORT_NOT_ENCRYPTED, _T("Note: Report is not encrypted.\n")),
    TSS_StringEntry(tw::STR_ERR_DB_READ, _T("Database file could not be read.")),

    TSS_StringEntry(tw::STR_CUR_LINE, _T("Line number ")),

    TSS_StringEntry(tw::STR_ENTER_LOCAL_PASSPHRASE, _T("Please enter your local passphrase: ")),
    TSS_StringEntry(tw::STR_ENTER_SITE_PASSPHRASE, _T("Please enter your site passphrase: ")),
    TSS_StringEntry(tw::STR_ENTER_PROVIDED_PASSPHRASE, _T("Please enter passphrase: ")),

    TSS_StringEntry(tw::STR_DB_NOT_UPDATED, _T("The database was not updated. ")),
    TSS_StringEntry(tw::STR_IGNORE_PROPS, _T("Ignoring properties: ")),
    TSS_StringEntry(tw::STR_NOT_IMPLEMENTED, _T("Feature not implemented yet: ")),
    TSS_StringEntry(tw::STR_REPORT_EMPTY, _T("The report contains no new data, the database will not be updated.\n")),

    TSS_StringEntry(tw::STR_GENERATING_DB, _T("Generating the database...\n")),
    TSS_StringEntry(tw::STR_INIT_SUCCESS, _T("The database was successfully generated.\n")),
    TSS_StringEntry(tw::STR_FILE_WRITTEN, _T("File written: ")),
    TSS_StringEntry(tw::STR_FILE_OPEN, _T("Opening file: ")),
    TSS_StringEntry(tw::STR_FILE_ENCRYPTED, _T("This file is encrypted.\n")),
    TSS_StringEntry(tw::STR_OPEN_KEYFILE, _T("Opening key file: ")),
    TSS_StringEntry(tw::STR_OPEN_CONFIG_FILE, _T("Opening configuration file: ")),
    TSS_StringEntry(tw::STR_OPEN_DB_FILE, _T("Opening database file: ")),
    TSS_StringEntry(tw::STR_OPEN_REPORT_FILE, _T("Opening report file: ")),

    TSS_StringEntry(tw::STR_REPORTEVENT_FAILED, _T("Unable to add event to Event Log.  The Event Log may be full.")),
    TSS_StringEntry(tw::STR_SYSLOG_FAILED, _T("Unable to add event to Syslog.")),

    TSS_StringEntry(tw::STR_WRITE_POLICY_FILE, _T("Wrote policy file: ")),
    TSS_StringEntry(tw::STR_WRITE_DB_FILE, _T("Wrote database file: ")),
    TSS_StringEntry(tw::STR_WRITE_REPORT_FILE, _T("Wrote report file: ")),
    TSS_StringEntry(tw::STR_WRITE_CONFIG_FILE, _T("Wrote configuration file: ")),

    TSS_StringEntry(tw::STR_REPORT_TITLE, _T("Open Source Tripwire(R) " PACKAGE_VERSION " Integrity Check Report")),
    TSS_StringEntry(tw::STR_R_GENERATED_BY, _T("Report generated by: ")),
    TSS_StringEntry(tw::STR_R_CREATED_ON, _T("Report created on: ")),
    TSS_StringEntry(tw::STR_DB_CREATED_ON, _T("Database generated on: ")),
    TSS_StringEntry(tw::STR_DB_LAST_UPDATE, _T("Database last updated on: ")),
    TSS_StringEntry(tw::STR_R_SUMMARY, _T("Report Summary:")), TSS_StringEntry(tw::STR_HOST_NAME, _T("Host name:")),
    TSS_StringEntry(tw::STR_HOST_IP, _T("Host IP address:")), TSS_StringEntry(tw::STR_HOST_ID, _T("Host ID:")),
    TSS_StringEntry(tw::STR_POLICY_FILE_USED, _T("Policy file used: ")),
    TSS_StringEntry(tw::STR_CONFIG_FILE_USED, _T("Configuration file used: ")),
    TSS_StringEntry(tw::STR_DB_FILE_USED, _T("Database file used: ")),
    TSS_StringEntry(tw::STR_CMD_LINE_USED, _T("Command line used: ")),
    TSS_StringEntry(tw::STR_SEVERITY_LEVEL, _T("Severity Level")),
    TSS_StringEntry(tw::STR_TOTAL_VIOLATIONS, _T("Total violations found: ")),
    TSS_StringEntry(tw::STR_OBJECTS_SCANNED, _T("Total objects scanned: ")),
    TSS_StringEntry(tw::STR_SECTION, _T("Section")),


    TSS_StringEntry(tw::STR_ATTRIBUTES, _T("Property: ")), TSS_StringEntry(tw::STR_OBSERVED, _T("Observed")),
    TSS_StringEntry(tw::STR_EXPECTED, _T("Expected")), TSS_StringEntry(tw::STR_DB_UPDATE, _T("Database Update: ")),
    TSS_StringEntry(tw::STR_RULE_SUMMARY, _T("Rule Summary: ")),
    TSS_StringEntry(tw::STR_OBJECT_SUMMARY, _T("Object Summary: ")),
    TSS_StringEntry(tw::STR_OBJECT_DETAIL, _T("Object Detail: ")), TSS_StringEntry(tw::STR_RULE_NAME, _T("Rule Name")),
    TSS_StringEntry(tw::STR_ALL_RULE_NAMES, _T("Rules Used: ")),
    TSS_StringEntry(tw::STR_TOTAL_NUM_RULES, _T("Total number of rules used: ")),
    TSS_StringEntry(tw::STR_START_POINT, _T("Start Point: ")), TSS_StringEntry(tw::STR_ADDED, _T("Added")),
    TSS_StringEntry(tw::STR_REMOVED, _T("Removed")), TSS_StringEntry(tw::STR_CHANGED, _T("Modified")),
    TSS_StringEntry(tw::STR_ADDED_FILES, _T("Added Objects: ")),
    TSS_StringEntry(tw::STR_REMOVED_FILES, _T("Removed Objects: ")),
    TSS_StringEntry(tw::STR_CHANGED_FILES, _T("Modified Objects: ")),
    TSS_StringEntry(tw::STR_TOTAL_NUM_ADDED_FILES, _T("Total number of added objects: ")),
    TSS_StringEntry(tw::STR_TOTAL_NUM_REMOVED_FILES, _T("Total number of removed objects: ")),
    TSS_StringEntry(tw::STR_TOTAL_NUM_CHANGED_FILES, _T("Total number of modified objects: ")),
    TSS_StringEntry(tw::STR_OBJECT_NAME, _T("Object name: ")),
    TSS_StringEntry(tw::STR_ADDED_FILE_NAME, _T("Added object name: ")),
    TSS_StringEntry(tw::STR_REMOVED_FILE_NAME, _T("Removed object name: ")),
    TSS_StringEntry(tw::STR_CHANGED_FILE_NAME, _T("Modified object name: ")),
    TSS_StringEntry(tw::STR_ERROR_REPORT, _T("Error Report: ")),
    TSS_StringEntry(tw::STR_REPORT_NO_ERRORS, _T("No Errors")),
    TSS_StringEntry(tw::STR_NO_VIOLATIONS_IN_SECTION, _T("No violations.")),

    TSS_StringEntry(tw::STR_ADD_X,
                    _T("Remove the \"x\" from the adjacent box to prevent updating the database\nwith the new values ")
                    _T("for this object.")),
    TSS_StringEntry(tw::STR_UPDATE_ALL, _T("Update database with all modifications for rule: ")),

    TSS_StringEntry(tw::STR_END_OF_REPORT, _T("*** End of report ***")), TSS_StringEntry(tw::STR_NONE, _T("None")),
    TSS_StringEntry(tw::STR_NEVER, _T("Never")), TSS_StringEntry(tw::STR_OBJECTS, _T("Objects")),
    TSS_StringEntry(tw::STR_TRIPWIRE_REPORT_SHORT, _T("TWReport")), TSS_StringEntry(tw::STR_VIOLATIONS_SHORT, _T("V")),
    TSS_StringEntry(tw::STR_MAX_SEV_SHORT, _T("S")), TSS_StringEntry(tw::STR_ADDED_SHORT, _T("A")),
    TSS_StringEntry(tw::STR_REMOVED_SHORT, _T("R")), TSS_StringEntry(tw::STR_CHANGED_SHORT, _T("C")),

    TSS_StringEntry(tw::STR_DBPRINT_TITLE, _T("Open Source Tripwire(R) " PACKAGE_VERSION " Database")),
    TSS_StringEntry(tw::STR_DB_GENERATED_BY, _T("Database generated by: ")),
    TSS_StringEntry(tw::STR_TOTAL_NUM_FILES, _T("Total number of objects: ")),
    TSS_StringEntry(tw::STR_END_OF_DB, _T("*** End of database ***")),
    TSS_StringEntry(tw::STR_DB_SUMMARY, _T("Database Summary: ")), TSS_StringEntry(tw::STR_ATTR_VALUE, _T("Value: ")),

    // twutil
    TSS_StringEntry(tw::STR_IP_UNKNOWN, _T("Unknown IP")),

    // twutil
    TSS_StringEntry(tw::STR_ERR_NOT_FULL_PATH, _T("Object name is not fully qualified; skipping.")),
    TSS_StringEntry(tw::STR_ERR_OBJ_NOT_IN_DB, _T("Object not found in the database.")),
    TSS_StringEntry(tw::STR_ERR_DB_DOESNT_CONTAIN_GENRE, _T("Section not found in database.")),
    TSS_StringEntry(tw::STR_ERR2_CFG_KEY_MISMATCH1, _T("The config file \"")),
    TSS_StringEntry(tw::STR_ERR2_CFG_KEY_MISMATCH2, _T("\" is not encrypted with the current keyfile \"")),
    TSS_StringEntry(tw::STR_ERR2_CFG_KEY_MISMATCH3, _T("\".")),
    TSS_StringEntry(tw::STR_ERR_CFG_UNENCRYPTED, _T("Config file is unencrypted.")),
    TSS_StringEntry(tw::STR_ERR_POL_UNENCRYPTED, _T("Policy file is unencrypted.")),
    TSS_StringEntry(tw::STR_ERR_BACKUP_FAILED, _T("File Backup Failed.")),
    TSS_StringEntry(tw::STR_ERR2_BACKUP_FAILED1, _T("Error backing up \"")),
    TSS_StringEntry(tw::STR_ERR2_BACKUP_FAILED2, _T("\" to \"")),
    TSS_StringEntry(tw::STR_ERR2_BACKUP_FAILED3, _T("\".")),

    // used for reporting errors when linked with console.exe or reportviewer.exe
    TSS_StringEntry(tw::STR_MEMORY_MAPPED_FILENAME, _T("Memory Mapped File")),

    //
    // Version
    //
    TSS_StringEntry(tw::STR_COPYRIGHT, TSS_COPYRIGHT_NOTICE),
    TSS_StringEntry(tw::STR_TSS_PRODUCT_NAME, TSS_PRODUCT_NAME BUILD_NUM UNICODE_STR DEBUG_STR),

    TSS_StringEntry(tw::STR_VERSION,
                    TSS_PRODUCT_NAME BUILD_NUM UNICODE_STR DEBUG_STR _T(" built for " TARGET_OS "\n\n")
                        TSS_COPYRIGHT_NOTICE),
    TSS_StringEntry(tw::STR_VERSION_LONG,
                    TSS_PRODUCT_NAME BUILD_NUM UNICODE_STR DEBUG_STR _T(" built for " TARGET_OS "\n\n")
                        TSS_COPYRIGHT_NOTICE_LONG),

    TSS_EndStringtable(cTW)
