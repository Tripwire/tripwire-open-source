//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000 Tripwire,
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
// file_t.cpp : A test harness for cFile, a class for abstracting
//		file operations between different platforms.

#include "core/stdcore.h"
#include "core/file.h"
#include "twtest/test.h"
#include <stdio.h>

void TestFile()
{ 

    TSTRING fileName = TEMP_DIR;
    fileName += _T("/file_test.bin");

	//Create a temporary file for testing:
	FILE* testStream;
	testStream = _tfopen( fileName.c_str(), _T("w+b"));
	TSTRING testString( _T("This is a test") );
	int iTestStringLength = testString.length();

	//Write some data to the stream...
	fwrite( testString.c_str(), sizeof(TCHAR), iTestStringLength, testStream );
	fclose( testStream );

	//Open the file again, for reading only this time.
	testStream = _tfopen( fileName.c_str(), _T("rb") );

	cFile fTempFile;
	//Try attaching one of our file objects to the stream.
//TODO: fTempFile.AttachRead( testStream );

	//Try reading something from the file object
	TCHAR buffer[40];
	TCHAR buffer2[40];

	fTempFile.Read( buffer, sizeof(TCHAR) * iTestStringLength );
	fTempFile.Close();

	testStream = _tfopen( fileName.c_str(), _T("a+b") );
//TODO:	fTempFile.AttachReadWrite( testStream ); 

	//Now try writing something to the stream.
	fTempFile.Write( testString.c_str(), sizeof(TCHAR) * iTestStringLength );
	fTempFile.Rewind();
	fTempFile.Read( buffer2, sizeof(TCHAR) * iTestStringLength * 2 );

}

