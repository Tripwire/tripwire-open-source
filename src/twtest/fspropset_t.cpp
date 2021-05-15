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
// fspropset_t.cpp -- FSPropSet test driver
#include "fs/stdfs.h"
#include "fs/fspropset.h"
#include "twtest/test.h"
#include "core/debug.h"


///////////////////////////////////////////////////////////////////////////////
// PrintPropVector -- function that prints the contents of a cFCOPropVector
///////////////////////////////////////////////////////////////////////////////
static void PrintPropVector(const cFCOPropVector& v, cDebug& d)
{
    TOSTRINGSTREAM stream;
    for (int i = 0; i < v.GetSize(); i++)
    {
        if (v.ContainsItem(i))
            stream << i << ","
                   << " ";
    }

    tss_mkstr(out, stream);

    d.TraceDebug("%s\n", out.c_str());
}

void TestFSPropSet()
{
    cDebug d("TestFSPropSet");

    cFSPropSet propSet;

    // mess around with inode...
    d.TraceDebug("Setting Inode (property %d)\n", cFSPropSet::PROP_INODE);
    propSet.SetInode(53);
    TEST(propSet.GetInode() == 53);
    d.TraceDebug("Valid Vector is now \n");
    PrintPropVector(propSet.GetValidVector(), d);
    TEST(cFSPropSet::PROP_INODE == propSet.GetPropIndex(_T("Inode Number")));
    TEST(TSTRING(_T("Inode Number")).compare(propSet.GetPropName(cFSPropSet::PROP_INODE)) == 0);

    // mess around with blocks...
    d.TraceDebug("Setting Blocks (property %d)\n", cFSPropSet::PROP_BLOCKS);
    propSet.SetBlocks(50);
    TEST(propSet.GetBlocks() == 50);
    d.TraceDebug("Valid Vector is now \n");
    PrintPropVector(propSet.GetValidVector(), d);
    TEST(cFSPropSet::PROP_BLOCKS == propSet.GetPropIndex(_T("Blocks")));
    TEST(TSTRING(_T("Blocks")).compare(propSet.GetPropName(cFSPropSet::PROP_BLOCKS)) == 0);

    // try copying it...
    cFSPropSet ps2 = propSet;
    d.TraceDebug("Copied Object's Valid Vector:\n");
    PrintPropVector(propSet.GetValidVector(), d);
    TEST(ps2.GetValidVector() == propSet.GetValidVector());
    TEST(ps2.GetBlocks() == propSet.GetBlocks());
    TEST(ps2.GetInode() == propSet.GetInode());

    // try invalidating properties...
    propSet.InvalidateProp(cFSPropSet::PROP_INODE);
    TEST(propSet.GetValidVector().ContainsItem(cFSPropSet::PROP_INODE) == false);
    propSet.InvalidateAll();
    cFCOPropVector emptyVector(propSet.GetValidVector().GetSize());
    TEST(propSet.GetValidVector() == emptyVector);

    return;
}

void RegisterSuite_FSPropSet()
{
    RegisterTest("FSPropSet", "Basic", TestFSPropSet);
}
