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
//////////////////////////////////////////////////////////////////////
// tripwireerrors.cpp
//
// Registers all error strings in the tripwire package
//

#include "stdtripwire.h"
#include "tripwireerrors.h"

#include "mailmessage.h"
#include "policyupdate.h"
#include "updatedb.h"
#include "integritycheck.h"
#include "twcmdline.h"
#include "twcmdlineutil.h"

TSS_BEGIN_ERROR_REGISTRATION(tripwire)

// MailMessage
TSS_REGISTER_ERROR(eMailMessageError(), _T("Mail message error."));

// SMTP
TSS_REGISTER_ERROR(eMailSMTPWinsockDidNotStart(), _T("An internet connection could not be found."));
TSS_REGISTER_ERROR(eMailSMTPWinsockUnloadable(), _T("WSOCK32.DLL could not be loaded."));
TSS_REGISTER_ERROR(eMailSMTPWinsockCleanup(), _T("The internet connection could not be terminated properly."));
TSS_REGISTER_ERROR(eMailSMTPIPUnresolvable(), _T("The IP address could not be resolved: "));
TSS_REGISTER_ERROR(eMailSMTPSocket(), _T("Socket for an SMTP connection could not be created."));
TSS_REGISTER_ERROR(eMailSMTPOpenConnection(), _T("The SMTP connection could not be established."));
TSS_REGISTER_ERROR(eMailSMTPCloseConnection(), _T("The SMTP connection could not be properly closed."));
TSS_REGISTER_ERROR(eMailSMTPServer(), _T("The SMTP server returned an error."));
TSS_REGISTER_ERROR(eMailSMTPNotSupported(), _T("SMTP email is not supported on this platform"));
// Piped
TSS_REGISTER_ERROR(eMailPipedOpen(), _T("Problem opening mail pipe."));
TSS_REGISTER_ERROR(eMailPipedWrite(), _T("Problem writing to mail pipe."));
TSS_REGISTER_ERROR(eMailPipedCmdFailed(), _T("Execution of mail program failed."));

// Policy Update
TSS_REGISTER_ERROR(ePolicyUpdate(), _T("Policy Update"));
TSS_REGISTER_ERROR(
    ePolicyUpdateAddedFCO(),
    _T("Policy Update Added Object.\nAn object has been added since the database was last updated.\nObject name: "));
TSS_REGISTER_ERROR(ePolicyUpdateRemovedFCO(),
                   _T("Policy Update Removed Object.\nAn object has been removed since the database was last ")
                   _T("updated.\nObject name: "));
TSS_REGISTER_ERROR(ePolicyUpdateChangedFCO(),
                   _T("Policy Update Changed Object.\nAn object has been changed since the database was last ")
                   _T("updated.\nObject name: "));

// Update Db
TSS_REGISTER_ERROR(eUpdateDb(), _T("Database Update"));
TSS_REGISTER_ERROR(eUpdateDbAddedFCO(),
                   _T("Database Update Addition.\nThe report may be out of sync with current database.\nThe report ")
                   _T("indicates an object has been added but the object is already in the database.\nObject name: "));
TSS_REGISTER_ERROR(
    eUpdateDbRemovedFCO(),
    _T("Database Update Object Not Found.\nThe report may be out of sync with current database.\nThe report indicates ")
    _T("that an object has been removed but the object is not in the database.\nObject name: "));
TSS_REGISTER_ERROR(
    eUpdateDbChangedFCO(),
    _T("Database Update Old Property Mismatch.\nThe report may be out of sync with current database.\nThe old ")
    _T("properties for an object do not match the properites stored in the database.\nObject name: "));

// IC
TSS_REGISTER_ERROR(eIC(), _T("Integrity Check"));
TSS_REGISTER_ERROR(eICFCONotInSpec(),
                   _T("Integrity Check Internal Error.\nObject to be checked is not in any database rules.\n"));
TSS_REGISTER_ERROR(eICFCONotInDb(), _T("Integrity Check Internal Error.\nObject to be checked is not in database.\n"));
TSS_REGISTER_ERROR(eICFCOCreate(),
                   _T("Integrity Check Internal Error.\nObject to be checked could not be created in the database.\n"));
TSS_REGISTER_ERROR(eICBadPol(), _T("Policy file does not match policy used to create database."));
TSS_REGISTER_ERROR(eICUnknownGenre(), _T("Invalid section specified on the command line."));
TSS_REGISTER_ERROR(eICBadSevVal(), _T("Invalid severity level specified on the command line."));
TSS_REGISTER_ERROR(eICInappropriateGenre(), _T("Inappropriate section specified on the command line."));
TSS_REGISTER_ERROR(eICDifferentUsers(), _T("Current user differs from one who created database."));
TSS_REGISTER_ERROR(eICMismatchingParams(), _T("Mismatch in specified command line parameters: "));
TSS_REGISTER_ERROR(eTWInvalidConfigFileKey(), _T("Configuration file contains an invalid variable."));
TSS_REGISTER_ERROR(eTWPassForUnencryptedDb(), _T("Passphrase specified for an unencrypted database file."));

// General TW
TSS_REGISTER_ERROR(eTWHelpInvalidModeSwitch(), _T("Invalid mode parameter to help: "));
TSS_REGISTER_ERROR(eTWInitialization(), _T("Tripwire initialization error."));
TSS_REGISTER_ERROR(eTWInvalidReportLevel(), _T("Invalid reporting level specified.\nValid levels: [0-4]\n"));
TSS_REGISTER_ERROR(eTWInvalidReportLevelCfg(),
                   _T("Invalid reporting level in configuration file\nValid levels: [0-4]\n"));
TSS_REGISTER_ERROR(eTWInvalidPortNumber(), _T("Invalid SMTP port number.\nValid ports: [0-65535]\n"));
TSS_REGISTER_ERROR(eTWInvalidTempDirectory(), _T("Cannot access temp directory."));

TSS_REGISTER_ERROR(eTWSyslogNotSupported(), _T("Syslog reporting is not supported on this platform."));
TSS_REGISTER_ERROR(eTWDirectIONotSupported(), _T("Direct I/O hashing is not supported on this platform."));

TSS_END_ERROR_REGISTRATION()
