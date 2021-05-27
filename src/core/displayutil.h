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
// Name....: displayutil.h
// Date....: 7/12/99
// Creator.: Brian McFeely (bmcfeely)
//

#ifndef __DISPLAYUTIL_H
#define __DISPLAYUTIL_H

class cDisplayUtil
{
public:
    static TSTRING FormatMultiLineString(const TSTRING& str, int nOffset, int nWidth);
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Way too verbose here, but it was the best I could do.  I know, I know.
// If it's too complicated to explain, there's an easier way to do it.
// Soooo, I'll just put a TODO:BAM here
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// So, there's this problem with outputting a string with newlines in it when you want
// to format the output stream.  For instance:
//
// TCHAR szTenSpaceMargin[] = { _T("          ") };
// TCHAR szMessage[] = { _T("First Line\nSecond Line\n") };
//
// TCOUT << szTenSpaceMargin << setw(15) << szMessage;
//
// What happens is that you get output like:
// "               First Line"
//  ^        ^^             ^
//  |--10 sp-||---15 width--|
// "Second Line"
//  ^
//  | 0 sp, 0 width
//
// When you really wanted:
// "               First Line"
//  ^        ^^             ^
//  |--10 sp-||---15 width--|
// "              Second Line"
//  ^        ^^             ^
//  |--10 sp-||---15 width--|
//
//
// Sooooo, what this little beauty does is change the string
// "First Line\nSecond Line\n" into ""First Line\n          Second Line\n"
// ( adds nOffset spaces after each '\n', and makes sure that each line is
// in a output width specified by nWidth )
//
// So you can output a multiline string and have each line be formatted
// like the first one was.
//
// So, for the preceeding example,
//
// change:
// TCOUT << szTenSpaceMargin << setw(15) << szMessage;
//
// to:
// TCOUT << szTenSpaceMargin << setw(15)
//       << cDisplayUtil::FormatMultiLineString( szMessage, 10, 15 );
//
// Comprende?  It's crazy but it works.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif // #ifndef __DISPLAYUTIL_H
