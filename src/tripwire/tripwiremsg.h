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
// tripwiremsg.mc
//
// This file (tripwiremsg.mc) is the source for tripwire.exe messages.  You
// may be reading these comments in tripwiremsg.h, in which case you are reading
// an output file from mc.exe.
//
// This is compiled by the dev studio utility mc.exe to produce the following files:
//
//       tripwiremsg.h
//       tripwiremsg.rc
//       MSG00001.bin
//
// The tripwiremsg.rc file is included in the tripwire resource script "Script1.rc".
// This causes the file MSG00001.bin to be included as a binary resrouce in the final
// executable.
//
// All of this is done to support using Tripwire as an event source for Event Logging.
// In general .mc files such as this are used to support custom strings for the
// FormatMessage() WinAPI.  The event viewer requires this mechanism for event strings.
//
// We are sort of cheating here.  By having each string be "%1", we can have any string
// we pass to ReportEvent() be the entire Event Log message.  Microsoft discourages this
// as it makes localization difficult (you can't just provide a new rc file with
// all of the localized versions of the strings).  However, we have chosen to handle the
// localization issues by localizing the strings in the various tripwirestrings files.
//
// To recompile this files into the files listed above run:
//
//       mc tripwiremsg.mc
//
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: MSG_TRIPWIRE_GENERIC_SUCCESS
//
// MessageText:
//
//  %1
//
#define MSG_TRIPWIRE_GENERIC_SUCCESS 0x00000001L

//
// MessageId: MSG_TRIPWIRE_GENERIC_INFO
//
// MessageText:
//
//  %1
//
#define MSG_TRIPWIRE_GENERIC_INFO 0x40000001L

//
// MessageId: MSG_TRIPWIRE_GENERIC_WARNING
//
// MessageText:
//
//  %1
//
#define MSG_TRIPWIRE_GENERIC_WARNING 0x80000001L

//
// MessageId: MSG_TRIPWIRE_GENERIC_ERROR
//
// MessageText:
//
//  %1
//
#define MSG_TRIPWIRE_GENERIC_ERROR 0xC0000001L
