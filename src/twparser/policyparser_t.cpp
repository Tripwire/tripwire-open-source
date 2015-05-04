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
// policyparser_t -- test policy files
//					TODO:
//					we should put lots of files in ./testfiles, and verify:
//					1.  correct files parse correctly
//					2.  incorrect files cause errors
//					3.  slightly incorrect files cause errors

#include "stdtwparser.h"
#include "core/debug.h"
#include "fco/fcospecimpl.h"
#include "parserhelper.h"
#include "policyparser.h"
#include "fs/fspropset.h"
#include "fco/fcospeclist.h"
#include "test/test.h"
#include <fstream>

// helper class that checks output of each fcospec
class cPolicyParserTester {
public:
	static bool			VerifyNameAndStartPoint(iFCOSpec *pfsspec, TSTRING &name);
};

void TestPolicyParser()
{
	cDebug d("TestPolicyParser()");

	//
	// file: pol.txt
	//
    try
	{
	    cDebug::AddOutTarget(cDebug::OUT_STDOUT);

        std::ifstream in;
        in.open( "../twparser/testfiles/pol.txt" );
        if( ! in.good() )
            throw eParserHelper( _T("couldn't open test file") );

		cPolicyParser parser( in );

        cGenreSpecListVector policy;
        cErrorQueue errorQ;
        cErrorReporter errorR;
        cErrorTracer errorT;

        // set up an error bucket that will spit things to stderr
	    errorT.SetChild( &errorR );
	    errorQ.SetChild( &errorT );

		parser.Execute( policy, &errorQ );

        return;
	}
	catch(eError& e)
	{
        TCERR << (int)e.GetID() << " : " << e.GetMsg().c_str() << std::endl;
		return;
	}

	return;
}
	

