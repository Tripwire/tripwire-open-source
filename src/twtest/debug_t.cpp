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
// debug_t -- debug component test driver

#include "core/stdcore.h"
#include "core/debug.h"
#include "twtest/test.h"

void TestDebug()
{
    // it is amusing that we use cDebug to output the results of testing cDebug
    // "Are you insane?" ... "No, I am not."
    cDebug d("TestDebug()");
    d.TraceDebug("Entering...");

    // save the current debug level, since we will be altering it.
    int oldDebugLevel = cDebug::GetDebugLevel();

    // test debug level variation...
    d.TraceDebug("Setting debug level to Debug(%d)\n", cDebug::D_DEBUG);
    cDebug::SetDebugLevel(cDebug::D_DEBUG);
    d.TraceDebug("You should see this, as well as line 2 below, but not line 3.\n");
    d.TraceWarning("Line 2: Warning(%d)\n", cDebug::D_WARNING);
    d.TraceDetail("Line 3: Detail(%d)\n", cDebug::D_DETAIL);
    d.TraceDebug("Restoring the debug level to %d\n", oldDebugLevel);
    cDebug::SetDebugLevel(oldDebugLevel);

    // testing the output source
    int oldOutTarget = 0;
    if (cDebug::HasOutTarget(cDebug::OUT_STDOUT))
        oldOutTarget |= cDebug::OUT_STDOUT;
    if (cDebug::HasOutTarget(cDebug::OUT_TRACE))
        oldOutTarget |= cDebug::OUT_TRACE;
    if (cDebug::HasOutTarget(cDebug::OUT_FILE))
        oldOutTarget |= cDebug::OUT_FILE;
    cDebug::RemoveOutTarget(cDebug::OUT_STDOUT);
    cDebug::RemoveOutTarget(cDebug::OUT_TRACE);
    cDebug::RemoveOutTarget(cDebug::OUT_FILE);

    d.TraceDebug("You should not see this (All out targets removed)\n");
    cDebug::AddOutTarget(cDebug::OUT_STDOUT);
    d.TraceDebug("You should see this in stdout only.\n");
    cDebug::AddOutTarget(cDebug::OUT_TRACE);
    d.TraceDebug("You should see this in stdout and trace.\n");
    cDebug::RemoveOutTarget(cDebug::OUT_STDOUT);
    d.TraceDebug("You should see this in trace only.\n");


    // set up an output file...use the temp file in test.h
    std::string str = TwTestPath("debug.out");

#ifdef DEBUG
    TEST(cDebug::SetOutputFile(str.c_str()));
#endif

    d.TraceDebug("This should be in trace and the file %s.\n", str.c_str());

    // restore the out source...
    // TODO -- note that the original output file cannot be restored; this sucks!
    if (oldOutTarget & cDebug::OUT_STDOUT)
        cDebug::AddOutTarget(cDebug::OUT_STDOUT);
    else
        cDebug::RemoveOutTarget(cDebug::OUT_STDOUT);
    if (oldOutTarget & cDebug::OUT_TRACE)
        cDebug::AddOutTarget(cDebug::OUT_TRACE);
    else
        cDebug::RemoveOutTarget(cDebug::OUT_TRACE);
    if (oldOutTarget & cDebug::OUT_FILE)
        cDebug::AddOutTarget(cDebug::OUT_FILE);
    else
        cDebug::RemoveOutTarget(cDebug::OUT_FILE);

    d.TraceDebug("Exiting...\n");

#ifndef DEBUG
    TEST("Should always succeed in release builds & cDebug should do nothing");
#endif
}

void RegisterSuite_Debug()
{
    RegisterTest("Debug", "Basic", TestDebug);
}
