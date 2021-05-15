//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2021 Tripwire,
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
// growheap_t.cpp
//
// test the grow heap component

#include "core/stdcore.h"
#include "core/growheap.h"
#include "twtest/test.h"
#include "core/error.h"

using namespace std;

void TestGrowHeap()
{
    const int initSize = 0x4000;
    const int growBy   = 0x4000;
    cGrowHeap gh(initSize, growBy, _T("growheap_t.cpp"));

    // report initial state
    /*    TCOUT << _T("Initial size: ") << initSize << endl;
    TCOUT << _T("Growby: ") << growBy << endl;
    TCOUT << _T("Initial Growheap memory usage: ") << gh.TotalMemUsage() << endl << endl;
*/
    const int growFactor = 5; // how much to enlarge requests each time
                              // make it odd so we can see if the growheap
                              // rounds to the alignment size
    for (size_t size = 1; size < growBy; size *= growFactor)
    {
        // allocate memory
        //TCOUT << _T("Allocing by ") << size << endl;
        void* p = gh.Malloc(size);
        TEST(p != NULL);

        // access memory with ints
        if (size > sizeof(int))
        {
            // read from memory
            //TCOUT << _T("Reading an int from memory...") << endl;
            int* pi = static_cast<int*>(p);
            int  i  = *pi;

            // write to memory
            //TCOUT << _T("Writing an int to memory...") << endl;
            *pi = i;
        }

        // access memory with doubles
        if (size > sizeof(double))
        {
            // read from memory
            //TCOUT << _T("Reading a double from memory...") << endl;
            double* pd = static_cast<double*>(p);
            double  d  = *pd;

            // write to memory
            //TCOUT << _T("Writing a double to memory...") << endl;
            *pd = d;
        }

        // report total usage
        //TCOUT << _T("Growheap memory usage: ") << gh.TotalMemUsage() << endl << endl;
    }

    TEST(gh.TotalMemUsage() > 0);

    // free memory
    gh.Clear();
    TEST(gh.TotalMemUsage() == 0);
}

void RegisterSuite_GrowHeap()
{
    RegisterTest("GrowHeap", "Basic", TestGrowHeap);
}
