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
// tripwirestrings.cpp
//
#include "stdtripwire.h"
#include "tripwirestrings.h"

#if IS_AROS
#    define VERSION_PREFIX "$VER: "
#else
#    define VERSION_PREFIX "@(#)"
#endif


TSS_BeginStringtable(cTripwire)

    TSS_StringEntry(tripwire::STR_TRIPWIRE_VERSION, _T("tripwire: File integrity assessment application.\n")),
    TSS_StringEntry(tripwire::STR_EMBEDDED_VERSION, _T(VERSION_PREFIX "tripwire " PACKAGE_VERSION)),

    TSS_StringEntry(tripwire::STR_TRIPWIRE_USAGE_SUMMARY,
                    _T("Usage:\n")
                    _T("\n")
                    _T("Database Initialization:  tripwire [-m i|--init] [options]\n")
                    _T("Integrity Checking:  tripwire [-m c|--check] [object1 [object2...]]\n")
                    _T("Database Update:  tripwire [-m u|--update]\n")
                    _T("Policy Update:  tripwire [-m p|--update-policy] policyfile.txt\n")
                    _T("Test:  tripwire [-m t|--test] --email address\n")
                    _T("\n")
                    _T("Type 'tripwire [mode] --help' OR\n")
                    _T("'tripwire --help mode [mode...]' OR\n")
                    _T("'tripwire --help all' for extended help\n")),

    TSS_StringEntry(tripwire::STR_TRIPWIRE_HELP_INIT,
                    _T("Database Initialization mode:\n")
                    _T("  -m i                 --init\n")
                    _T("  -v                   --verbose\n")
                    _T("  -s                   --silent, --quiet\n")
                    _T("  -c cfgfile           --cfgfile cfgfile\n")
                    _T("  -p polfile           --polfile polfile\n")
                    _T("  -d database          --dbfile database\n")
                    _T("  -S sitekey           --site-keyfile sitekey\n")
                    _T("  -P passphrase        --local-passphrase passphrase\n")
                    _T("  -L localkey          --local-keyfile localkey\n")
                    _T("  -e                   --no-encryption\n")
                    _T("\n")
                    _T("The -v and -s options are mutually exclusive.\n")
                    _T("The -L and -e options are mutually exclusive.\n")
                    _T("The -P and -e options are mutually exclusive.\n")
                    _T("\n")),

    TSS_StringEntry(tripwire::STR_TRIPWIRE_HELP_CHECK,
                    _T("Integrity Check mode:\n")
                    _T("  -m c                 --check\n")
                    _T("  -I                   --interactive\n")
                    _T("  -v                   --verbose\n")
                    _T("  -h                   --hexadecimal\n")
                    _T("  -s                   --silent, --quiet\n")
                    _T("  -c cfgfile           --cfgfile cfgfile\n")
                    _T("  -p polfile           --polfile polfile\n")
                    _T("  -S sitekey           --site-keyfile sitekey\n")
                    _T("  -L localkey          --local-keyfile localkey\n")
                    _T("  -d database          --dbfile database\n")
                    _T("  -r report            --twrfile report\n")
                    _T("  -P passphrase        --local-passphrase passphrase\n")
                    _T("  -n                   --no-tty-output\n")
                    _T("  -V editor            --visual editor\n")
                    _T("  -E                   --signed-report\n")
                    _T("  -R rule              --rule-name rule\n")
                    _T("  -l {level | name}    --severity {level | name}\n")
                    _T("  -x section           --section section\n")
                    _T("  -i list              --ignore list\n")
                    _T("  -M                   --email-report\n")
                    _T("  -t { 0|1|2|3|4 }     --email-report-level { 0|1|2|3|4 }\n")
                    _T("[object1 [object2...]]\n")
                    _T("\n")
                    _T("The -v and -s options are mutually exclusive.\n")
                    _T("The -l and -R options are mutually exclusive.\n")
                    _T("The -P option is only valid with -E or -I.\n")
                    _T("The -V option is only valid with -I.\n")
                    _T("The -t option is only valid with -M.\n")
                    _T("Specifying objects overrides the -l and -R options.\n")
                    _T("\n")),

    TSS_StringEntry(tripwire::STR_TRIPWIRE_HELP_UPDATE,
                    _T("Database Update mode:\n")
                    _T("  -m u                 --update\n")
                    _T("  -v                   --verbose\n")
                    _T("  -s                   --silent, --quiet\n")
                    _T("  -c cfgfile           --cfgfile cfgfile\n")
                    _T("  -p polfile           --polfile polfile\n")
                    _T("  -S sitekey           --site-keyfile sitekey\n")
                    _T("  -L localkey          --local-keyfile localkey\n")
                    _T("  -d database          --dbfile database\n")
                    _T("  -r report            --twrfile report\n")
                    _T("  -P passphrase        --local-passphrase passphrase\n")
                    _T("  -V editor            --visual editor\n")
                    _T("  -a                   --accept-all\n")
                    _T("  -Z {low | high}      --secure-mode {low | high}\n")
                    _T("\n")
                    _T("The -v and -s options are mutually exclusive.\n")
                    _T("The -a and -V options are mutually exclusive.\n")
                    _T("\n")),

    TSS_StringEntry(tripwire::STR_TRIPWIRE_HELP_UPDATE_POLICY,
                    _T("Policy Update mode:\n")
                    _T("  -m p                 --update-policy\n")
                    _T("  -v                   --verbose\n")
                    _T("  -s                   --silent, --quiet\n")
                    _T("  -c cfgfile           --cfgfile cfgfile\n")
                    _T("  -p polfile           --polfile polfile\n")
                    _T("  -S sitekey           --site-keyfile sitekey\n")
                    _T("  -L localkey          --local-keyfile localkey\n")
                    _T("  -d database          --dbfile database\n")
                    _T("  -P passphrase        --local-passphrase passphrase\n")
                    _T("  -Q passphrase        --site-passphrase passphrase\n")
                    _T("  -Z {low | high}      --secure-mode {low | high}\n")
                    _T("policyfile.txt\n")
                    _T("\n")
                    _T("The -v and -s options are mutually exclusive.\n")
                    _T("\n")),

    TSS_StringEntry(tripwire::STR_TRIPWIRE_HELP_TEST,
                    _T("Test mode:\n")
                    _T("  -m t                 --test\n")
                    _T("  -e user@domain.com   --email user@domain.com\n")
                    _T("\n")
                    _T("Only one address may be entered.\n")
                    _T("\n")),


    TSS_StringEntry(tripwire::STR_INTEGRITY_CHECK, _T("Performing integrity check...\n")),
    TSS_StringEntry(tripwire::STR_IC_SUCCESS, _T("Integrity check complete.\n")),
    TSS_StringEntry(tripwire::STR_IC_FILES, _T("Integrity checking objects specified on command line...\n")),
    TSS_StringEntry(tripwire::STR_CHECKING_SPECS_NAMED, _T("Checking rules named: ")),
    TSS_StringEntry(tripwire::STR_CHECKING_GENRE, _T("Checking section %s only.\n")),
    TSS_StringEntry(tripwire::STR_IC_SEVERITY_LEVEL, _T("Checking rules at or above severity level: ")),
    TSS_StringEntry(tripwire::STR_IC_IGNORING_SEVERITY, _T("Ignoring rule %s (severity %d)\n")),
    TSS_StringEntry(tripwire::STR_IC_IGNORING_SEV_NUM,
                    _T("Warning: Severity number specified on command line will be ignored.\n")),
    TSS_StringEntry(tripwire::STR_IC_IGNORING_SEV_NAME,
                    _T("Warning: Severity name specified on command line will be ignored.\n")),
    TSS_StringEntry(tripwire::STR_IC_IGNORING_RULE_NAME,
                    _T("Warning: Rule name specified on command line will be ignored.\n")),
    TSS_StringEntry(tripwire::STR_IC_IGNORING_GENRE_NAME,
                    _T("Warning: Section name specified on command line will be ignored.\n")),
    TSS_StringEntry(tripwire::STR_IC_NOEMAIL_SENT, _T("Nothing to report, no email sent.\n")),
    TSS_StringEntry(tripwire::STR_NO_EMAIL_RECIPIENTS, _T("No email recipients specified, no email sent.\n")),

    TSS_StringEntry(tripwire::STR_PU_PROCESSING_GENRE, _T("======== Policy Update: Processing section %s.\n")),
    TSS_StringEntry(tripwire::STR_PU_INTEGRITY_CHECK,
                    _T("======== Step 1: Gathering information for the new policy.\n")),
    TSS_StringEntry(tripwire::STR_PU_UPDATE_DB, _T("======== Step 2: Updating the database with new objects.\n")),
    TSS_StringEntry(tripwire::STR_PU_PRUNING, _T("======== Step 3: Pruning unneeded objects from the database.\n")),
    TSS_StringEntry(tripwire::STR_PU_ADDING_GENRE, _T("======== Policy Update: Adding section %s.\n")),
    TSS_StringEntry(tripwire::STR_PU_BAD_PROPS, _T("Conflicting properties for object ")),
    TSS_StringEntry(tripwire::STR_PROCESSING_GENRE, _T("*** Processing %s ***\n")),


    TSS_StringEntry(tripwire::STR_ERR2_MAIL_MESSAGE_SERVER, _T("Server: ")),
    TSS_StringEntry(tripwire::STR_ERR2_MAIL_MESSAGE_SERVER_RETURNED_ERROR, _T("Error Number:")),
    TSS_StringEntry(tripwire::STR_ERR2_MAIL_MESSAGE_COMMAND, _T("Command: ")),

    //
    // Tripwire Command Line
    //
    TSS_StringEntry(tripwire::STR_ERR_IC_EMAIL_AND_FILES,
                    _T("Email reporting cannot be specified when checking a list of objects.")),
    TSS_StringEntry(tripwire::STR_ERR_NO_MAIL_METHOD, _T("No mail method specified in configuration file.")),
    TSS_StringEntry(tripwire::STR_ERR_INVALID_MAIL_METHOD, _T("Invalid mail method in configuration file.")),
    TSS_StringEntry(tripwire::STR_ERR_MISSING_MAILPROGRAM,
                    _T("Mail method is set to SENDMAIL but MAILPROGRAM was not defined in configuration file")),
    TSS_StringEntry(tripwire::STR_ERR_NO_TEST_MODE, _T("No test area specified.")),
    TSS_StringEntry(tripwire::STR_GENERATING_DB, _T("Generating the database...\n")),
    TSS_StringEntry(tripwire::STR_INIT_SUCCESS, _T("The database was successfully generated.\n")),
    TSS_StringEntry(tripwire::STR_ERR2_DIFFERENT_USERS1, _T("Created by: ")),
    TSS_StringEntry(tripwire::STR_ERR2_DIFFERENT_USERS2, _T("\nCurrent user: ")),
    TSS_StringEntry(tripwire::STR_ERR_IC_NO_SPECS_LEFT, _T("No rules to check for section %s.\n")),
    TSS_StringEntry(tripwire::STR_ERR_BAD_PARAM, _T("Invalid parameter.")),
    TSS_StringEntry(tripwire::STR_ERR_UPDATE_ED_LAUNCH, _T("Interactive Update failed.")),
    TSS_StringEntry(tripwire::STR_REPORT_EMPTY,
                    _T("The report contains no new data, the database will not be updated.\n")),
    TSS_StringEntry(tripwire::STR_DB_NOT_UPDATED, _T("The database was not updated. ")),
    TSS_StringEntry(tripwire::STR_ERR_POL_UPDATE,
                    _T("Policy update failed; policy and database files were not altered.")),
    TSS_StringEntry(tripwire::STR_IGNORE_PROPS, _T("Ignoring properties: ")),
    TSS_StringEntry(tripwire::STR_ERR_ILLEGAL_MODE_HELP, _T("Error, mode does not exist: ")),
    TSS_StringEntry(tripwire::STR_SYSLOG_IDENT, _T("tripwire")),
    TSS_StringEntry(tripwire::STR_SYSLOG_INIT_MSG, _T("Database initialized: ")),
    TSS_StringEntry(tripwire::STR_SYSLOG_IC_MSG, _T("Integrity Check Complete: ")),
    TSS_StringEntry(tripwire::STR_SYSLOG_UP_MSG, _T("Database Update Complete: ")),
    TSS_StringEntry(tripwire::STR_SYSLOG_POLUP_MSG, _T("Policy Update Complete: ")),
    TSS_StringEntry(tripwire::STR_SYSLOG_INIT_FAIL_MSG, _T("Database initialization Failed: ")),
    TSS_StringEntry(tripwire::STR_SYSLOG_IC_FAIL_MSG, _T("Integrity Check Failed: ")),
    TSS_StringEntry(tripwire::STR_SYSLOG_UP_FAIL_MSG, _T("Database Update Failed: ")),
    TSS_StringEntry(tripwire::STR_SYSLOG_POLUP_FAIL_MSG, _T("Policy Update Failed: ")),


    //
    // email testing
    //
    TSS_StringEntry(tripwire::STR_TEST_EMAIL_TO, _T("Sending a test message to:")),
    TSS_StringEntry(tripwire::STR_TEST_EMAIL_SUBJECT, _T("Test email message from Tripwire")),
    TSS_StringEntry(tripwire::STR_TEST_EMAIL_BODY,
                    _T("If you receive this message, email notification from tripwire is working correctly.")),
    //
    // email reporting
    //
    TSS_StringEntry(tripwire::STR_EMAIL_SUBJECT_REPORT_1, _T("Tripwire report (violation count ")),
    TSS_StringEntry(tripwire::STR_EMAIL_SUBJECT_REPORT_2, _T(") (max severity ")),
    TSS_StringEntry(tripwire::STR_EMAIL_SUBJECT_REPORT_3, _T(")")),
    TSS_StringEntry(tripwire::STR_EMAIL_REPORT_TO, _T("Emailing the report to:")),
    TSS_StringEntry(tripwire::STR_EMAIL_FROM, _T("tripwire@")),
    TSS_StringEntry(tripwire::STR_EMAIL_BEGIN, _T("Beginning email reporting...")),
    TSS_StringEntry(tripwire::STR_EMAIL_OK_BODY, _T("Rules with no violations or errors:")),
    TSS_StringEntry(tripwire::STR_EMAIL_NO_RECIPIENTS,
                    _T("No email recipients specified in policy file; no email sent.")),
    TSS_StringEntry(tripwire::STR_EMAIL_OK_TO, _T("Emailing \"no violations\" message to:")),
    TSS_StringEntry(tripwire::STR_EMAIL_SUBJECT_OK, _T("Tripwire found no violations")),
    TSS_StringEntry(tripwire::STR_ERR_EMAIL_REPORT, _T("Email reporting failed.")),
    TSS_StringEntry(tripwire::STR_ERR_EMAIL_TEST, _T("Email test failed.")),

    TSS_StringEntry(tripwire::STR_NOTIFY_PROCESSING, _T("Processing: ")),
    TSS_StringEntry(tripwire::STR_NOTIFY_CHECKING, _T("Checking: ")),
    TSS_StringEntry(tripwire::STR_NOTIFY_CHECKING_RULE, _T("Checking rule: ")),
    TSS_StringEntry(tripwire::STR_NOTIFY_DB_CHANGING, _T("Database update: Modifying record ")),
    TSS_StringEntry(tripwire::STR_NOTIFY_DB_ADDING, _T("Database update: Adding record ")),
    TSS_StringEntry(tripwire::STR_NOTIFY_DB_REMOVING, _T("Database update: Removing record ")),
    TSS_StringEntry(tripwire::STR_NOTIFY_GEN_SIG, _T("Generating information for: ")),

    TSS_EndStringtable(cTripwire)
