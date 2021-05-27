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
// tripwirestrings.h
//
#ifndef __TRIPWIRESTRINGS_H
#define __TRIPWIRESTRINGS_H

#include "tripwire.h"

TSS_BeginStringIds(tripwire)

    STR_TRIPWIRE_VERSION,
    STR_EMBEDDED_VERSION, STR_TRIPWIRE_HELP_INIT, STR_TRIPWIRE_HELP_CHECK, STR_TRIPWIRE_HELP_UPDATE,
    STR_TRIPWIRE_HELP_UPDATE_POLICY, STR_TRIPWIRE_HELP_TEST, STR_TRIPWIRE_USAGE_SUMMARY, STR_INTEGRITY_CHECK,
    STR_IC_SUCCESS, STR_IC_FILES, STR_CHECKING_SPECS_NAMED, STR_CHECKING_GENRE, STR_IC_SEVERITY_LEVEL,
    STR_IC_IGNORING_SEVERITY, STR_IC_IGNORING_SEV_NUM, STR_IC_IGNORING_SEV_NAME, STR_IC_IGNORING_RULE_NAME,
    STR_IC_IGNORING_GENRE_NAME, STR_IC_NOEMAIL_SENT, STR_NO_EMAIL_RECIPIENTS, STR_PU_PROCESSING_GENRE,
    STR_PU_INTEGRITY_CHECK, STR_PU_UPDATE_DB, STR_PU_PRUNING, STR_PU_ADDING_GENRE, STR_PU_BAD_PROPS,
    STR_PROCESSING_GENRE,

    STR_ERR2_MAIL_MESSAGE_SERVER, STR_ERR2_MAIL_MESSAGE_SERVER_RETURNED_ERROR, STR_ERR2_MAIL_MESSAGE_COMMAND,

    //
    // Tripwire Command Line
    //
    STR_ERR_IC_EMAIL_AND_FILES, STR_ERR_NO_MAIL_METHOD, STR_ERR_INVALID_MAIL_METHOD, STR_ERR_MISSING_MAILPROGRAM,
    STR_ERR_NO_TEST_MODE, STR_GENERATING_DB, STR_INIT_SUCCESS, STR_ERR2_DIFFERENT_USERS1, STR_ERR2_DIFFERENT_USERS2,
    STR_ERR_IC_NO_SPECS_LEFT, STR_ERR_BAD_PARAM, STR_ERR_UPDATE_ED_LAUNCH, STR_REPORT_EMPTY, STR_DB_NOT_UPDATED,
    STR_ERR_POL_UPDATE, STR_IGNORE_PROPS, STR_ERR_ILLEGAL_MODE_HELP, STR_SYSLOG_IDENT, STR_SYSLOG_INIT_MSG,
    STR_SYSLOG_IC_MSG, STR_SYSLOG_UP_MSG, STR_SYSLOG_POLUP_MSG, STR_SYSLOG_INIT_FAIL_MSG, STR_SYSLOG_IC_FAIL_MSG,
    STR_SYSLOG_UP_FAIL_MSG, STR_SYSLOG_POLUP_FAIL_MSG,

    //
    // email testing
    //
    STR_TEST_EMAIL_TO, STR_TEST_EMAIL_SUBJECT, STR_TEST_EMAIL_BODY,
    //
    // email reporting
    //
    STR_EMAIL_SUBJECT_REPORT_1, STR_EMAIL_SUBJECT_REPORT_2, STR_EMAIL_SUBJECT_REPORT_3, STR_EMAIL_REPORT_TO,
    STR_EMAIL_FROM, STR_EMAIL_BEGIN, STR_EMAIL_OK_BODY, STR_EMAIL_NO_RECIPIENTS, STR_EMAIL_OK_TO, STR_EMAIL_SUBJECT_OK,
    STR_ERR_EMAIL_REPORT, STR_ERR_EMAIL_TEST,

    STR_NOTIFY_PROCESSING, STR_NOTIFY_CHECKING, STR_NOTIFY_CHECKING_RULE, STR_NOTIFY_DB_CHANGING, STR_NOTIFY_DB_ADDING,
    STR_NOTIFY_DB_REMOVING,
    STR_NOTIFY_GEN_SIG

    TSS_EndStringIds(tripwire)


#endif
