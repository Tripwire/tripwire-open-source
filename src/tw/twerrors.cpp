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
// twerrors.cpp
//
// Registers all error strings in the tw package
//
#include "stdtw.h"
#include "core/errortable.h"

#include "twerrors.h"
#include "configfile.h"
#include "twutil.h"
#include "textreportviewer.h"
#include "filemanipulator.h"
#include "fcodatabasefile.h"

TSS_BEGIN_ERROR_REGISTRATION(tw)

//
// General
//
TSS_REGISTER_ERROR(eTWDbNotEncrypted(), _T("Database is not encrypted."));
TSS_REGISTER_ERROR(eTWDbDoesntHaveGenre(), _T("Section not found in database."));
TSS_REGISTER_ERROR(eTWDbHasNoGenres(), _T("Database file contains no sections!"));
TSS_REGISTER_ERROR(eTWBadPropsToIgnore(), _T("Invalid property mask specified on command line:"));
TSS_REGISTER_ERROR(eTWUnknownSectionName(), _T("Unknown section name specified on command line:"));
TSS_REGISTER_ERROR(eTWBadSeverityName(), _T("Unknown severity name (valid values are [low, medium, high])."));
TSS_REGISTER_ERROR(eTWCfgUnencrypted(), _T("Config file is unencrypted."));
TSS_REGISTER_ERROR(eTWPolUnencrypted(), _T("Policy file is unencrypted."));
TSS_REGISTER_ERROR(eTWSyslog(), _T("Syslog reporting failure."));


//
// Config File
//
TSS_REGISTER_ERROR(eConfigFile(), _T("Configuration file error."))
TSS_REGISTER_ERROR(eConfigFileNoEq(), _T("Configuration file contains a line with no \"=\"."))
TSS_REGISTER_ERROR(eConfigFileNoKey(), _T("Configuration file contains a line with no variable."))
TSS_REGISTER_ERROR(eConfigFileAssignToPredefVar(), _T("Configuration file assigns a value to a read only variable."))
TSS_REGISTER_ERROR(eConfigFileUseUndefVar(), _T("Configuration file uses an undefined variable."))
TSS_REGISTER_ERROR(eConfigFileMissReqKey(), _T("Configuration file does not define mandatory variable(s)."))
TSS_REGISTER_ERROR(eConfigFileEmptyVariable(), _T("Configuration file assigns an empty value to a variable."))
TSS_REGISTER_ERROR(eConfigFileMissingRightParen(), _T("Configuration file variable has no right parenthesis."))


//
// TW Util
//
TSS_REGISTER_ERROR(eTWUtil(), _T("Tripwire error."))
TSS_REGISTER_ERROR(eTWUtilNotFullPath(), _T("Object name is not fully qualified; skipping."))
TSS_REGISTER_ERROR(eTWUtilCfgKeyMismatch(), _T("Config file site key mismatch."))
TSS_REGISTER_ERROR(eTWUtilCorruptedFile(), _T("File is corrupted."))
TSS_REGISTER_ERROR(eTWUtilBadPassLocal(), _T("Incorrect local passphrase."))
TSS_REGISTER_ERROR(eTWUtilBadPassSite(), _T("Incorrect site passphrase."))
TSS_REGISTER_ERROR(eTWUtilEchoModeSet(), _T("Could not set console to no echo mode."))
TSS_REGISTER_ERROR(eTWUtilPolUnencrypted(), _T("Policy file is unencrypted."))
TSS_REGISTER_ERROR(eTWUtilObjNotInDb(), _T("Object not found in the database."))


//
// Text Report Viewer
//
TSS_REGISTER_ERROR(eTextReportViewer(), _T("Text report viewer error."))
TSS_REGISTER_ERROR(eTextReportViewerFileOpen(), _T("File could not be opened."))
TSS_REGISTER_ERROR(eTextReportViewerEditorLaunch(), _T("Editor could not be launched."))
TSS_REGISTER_ERROR(eTextReportViewerReportCorrupt(), _T("Report file could not be parsed.  Report may be corrupt."))

//
// File Manipulator
//
TSS_REGISTER_ERROR(eFileManip(), _T("File manipulation error."))
TSS_REGISTER_ERROR(eFileManipInvalidFile(), _T("Could not manipulate invalid file."))
TSS_REGISTER_ERROR(eFileManipFileRead(), _T("File could not be read."))
TSS_REGISTER_ERROR(eFileManipHeaderNotFound(), _T("File is not a Tripwire data file."))
TSS_REGISTER_ERROR(eFileManipFileNotFound(), _T("File could not be found."))
TSS_REGISTER_ERROR(eFileManipNotEncrypted(), _T("File is not encrypted."))
TSS_REGISTER_ERROR(eFileManipMissingKey(), _T("Key not supplied."))
TSS_REGISTER_ERROR(eFileManipNotWritable(), _T("File is not writable."))
TSS_REGISTER_ERROR(eFileManipUnrecognizedFileType(), _T("Unrecognized file type."))
// TODO:mdb -- why was this commented out of twstrings and the error table?

//
// Database File
//
TSS_REGISTER_ERROR(eFCODbFile(), _T("Database file error."));
TSS_REGISTER_ERROR(eFCODbFileTooBig(), _T("Database file too large."));

TSS_END_ERROR_REGISTRATION()
