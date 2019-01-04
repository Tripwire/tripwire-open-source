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
// coreerrors.cpp
//
// Registers all error strings in the core package
//

#include "stdcore.h"
#include "coreerrors.h"

#include "errorgeneral.h"
#include "archive.h"
#include "error.h"
#include "unixexcept.h"
#include "fsservices.h"
#include "serializer.h"
#include "cmdlineparser.h"
#include "twlocale.h"
#include "codeconvert.h"
#include "ntmbs.h"
#include "displayencoder.h"

TSS_BEGIN_ERROR_REGISTRATION(core)

/// Internal

TSS_REGISTER_ERROR(eInternal(), _T("Internal error."))


/// General

TSS_REGISTER_ERROR(eErrorGeneral(), _T("General Error"));
TSS_REGISTER_ERROR(eOpen(), _T("File could not be opened."));
TSS_REGISTER_ERROR(eOpenRead(), _T("File could not be opened for reading."));
TSS_REGISTER_ERROR(eOpenWrite(), _T("File could not be opened for writing."));
TSS_REGISTER_ERROR(eBadModeSwitch(), _T("Unknown mode specified."));
TSS_REGISTER_ERROR(eBadCmdLine(), _T("Command line error."));


/// Archive

TSS_REGISTER_ERROR(eArchive(), _T("Archive error."))
TSS_REGISTER_ERROR(eArchiveOpen(), _T("File could not be opened."))
TSS_REGISTER_ERROR(eArchiveWrite(), _T("File could not be written."))
TSS_REGISTER_ERROR(eArchiveRead(), _T("File could not be read."))
TSS_REGISTER_ERROR(eArchiveEOF(), _T("End of file reached."))
TSS_REGISTER_ERROR(eArchiveSeek(), _T("File seek failed."))
TSS_REGISTER_ERROR(eArchiveMemmap(), _T("Memory mapped archive file invalid."))
TSS_REGISTER_ERROR(eArchiveOutOfMem(), _T("Archive ran out of memory."))
TSS_REGISTER_ERROR(eArchiveInvalidOp(), _T("Archive logic error."))
TSS_REGISTER_ERROR(eArchiveFormat(), _T("Archive file format invalid."))
TSS_REGISTER_ERROR(eArchiveNotRegularFile(), _T("File is not a regular file."))
TSS_REGISTER_ERROR(eArchiveCrypto(), _T("File could not be decrypted."))
TSS_REGISTER_ERROR(eArchiveStringTooLong(), _T("String was too long."))


/// File

TSS_REGISTER_ERROR(eFile(), _T("File error."))
TSS_REGISTER_ERROR(eFileOpen(), _T("File could not be opened."))
TSS_REGISTER_ERROR(eFileWrite(), _T("File could not be written."))
TSS_REGISTER_ERROR(eFileRead(), _T("File could not be read."))
TSS_REGISTER_ERROR(eFileEOF(), _T("End of file reached."))
TSS_REGISTER_ERROR(eFileSeek(), _T("File seek failed."))
TSS_REGISTER_ERROR(eFileInvalidOp(), _T("File logic error."))
TSS_REGISTER_ERROR(eFileTrunc(), _T("File could not be truncated."))
TSS_REGISTER_ERROR(eFileClose(), _T("File could not be closed."))
TSS_REGISTER_ERROR(eFileFlush(), _T("File could not be flushed."))
TSS_REGISTER_ERROR(eFileRewind(), _T("File could not be rewound."))


/// General API failures
TSS_REGISTER_ERROR(eUnix(), _T("Unix API failure."))


/// FSServices

TSS_REGISTER_ERROR(eFSServices(), _T("File system error."))
TSS_REGISTER_ERROR(eFSServicesGeneric(), _T("File system error."))


/// Serializer

TSS_REGISTER_ERROR(eSerializerUnknownType(),
                   _T("Unknown type encountered in file.\nFile format may not be valid for this platform."))
TSS_REGISTER_ERROR(eSerializerInputStreamFmt(), _T("Invalid input stream format."))
TSS_REGISTER_ERROR(eSerializerOutputStreamFmt(), _T("Invalid output stream format."))
TSS_REGISTER_ERROR(eSerializerInputStremTypeArray(), _T("A bad index was encountered in file."))
TSS_REGISTER_ERROR(eSerializerArchive(), _T("File read encountered an archive error."))
TSS_REGISTER_ERROR(eSerializerVersionMismatch(), _T("File version mismatch."))
TSS_REGISTER_ERROR(eSerializerEncryption(), _T("File encryption error."))
TSS_REGISTER_ERROR(eSerializer(), _T("File format error."))


/// Command Line

TSS_REGISTER_ERROR(eCmdLine(), _T("Command line parsing error."))
TSS_REGISTER_ERROR(eCmdLineInvalidArg(), _T("Invalid argument passed on command line."))
TSS_REGISTER_ERROR(eCmdLineBadArgParam(), _T("Incorrect number of parameters to a command line argument."))
TSS_REGISTER_ERROR(eCmdLineBadParam(), _T("Incorrect number of parameters on command line."))
TSS_REGISTER_ERROR(eCmdLineBadSwitchPos(), _T("Switch appears after final command line parameter."))
TSS_REGISTER_ERROR(eCmdLineMutEx(), _T("Specified command line switches are mutually exclusive."))
TSS_REGISTER_ERROR(eCmdLineDependency(), _T("Command line parameter missing."))
TSS_REGISTER_ERROR(eCmdLineMultiArg(), _T("Command line argument specified more than once."))


/// TWLocale

TSS_REGISTER_ERROR(eTWLocale(), _T("Localization error."))
TSS_REGISTER_ERROR(eTWLocaleBadNumFormat(), _T("Bad number format."))


/// Character Handling (defined in ntmbs.h)

TSS_REGISTER_ERROR(eCharacter(), _T("General Character Handling Error."))
TSS_REGISTER_ERROR(eCharacterEncoding(), _T("Character Encoding Error."))


/// Character Conversion Handling (defined in <codeconvert.h>)

TSS_REGISTER_ERROR(eConverter(), _T("General conversion error."))
TSS_REGISTER_ERROR(eConverterReset(), _T("Converter handle could not be reset."))
TSS_REGISTER_ERROR(eConverterFatal(), _T("Catastrophic conversion error."))
TSS_REGISTER_ERROR(eConverterUnsupportedConversion(), _T("Unsupported character conversion."))
TSS_REGISTER_ERROR(eConverterUnknownCodepage(), _T("Could not identify code page."))

//
// Display Encoder
//

TSS_REGISTER_ERROR(eEncoder(), _T("Display encoder error."))
TSS_REGISTER_ERROR(eBadDecoderInput(), _T("Bad input to display encoder."))
TSS_REGISTER_ERROR(eBadHexConversion(), _T("Bad hex conversion in display encoder."))
TSS_REGISTER_ERROR(eUnknownEscapeEncoding(), _T("Unknown encoding in display encoder input."))


TSS_END_ERROR_REGISTRATION()
