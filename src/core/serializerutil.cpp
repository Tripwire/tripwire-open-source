//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2018 Tripwire,
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
// serilaizerutil.cpp
#include "stdcore.h"
#include "serializerutil.h"
#include "archive.h"
#include "serializer.h"

namespace
{
template<class FROM, class TO> int64 CopyImpl(TO* pTo, FROM* pFrom, int64 amt)
{
    enum
    {
        BUF_SIZE = 8192
    };
    int8  buf[BUF_SIZE];
    int64 amtLeft = amt;

    while (amtLeft > 0)
    {
        // NOTE: We use int's here rather than int64 because iSerializer and cArchive
        // only take int's as their size parameter - dmb
        int amtToRead = amtLeft > BUF_SIZE ? BUF_SIZE : (int)amtLeft;
        int amtRead   = pFrom->ReadBlob(buf, amtToRead);
        amtLeft -= amtRead;
        pTo->WriteBlob(buf, amtRead);
        if (amtRead < amtToRead)
            break;
    }

    // return the amount copied ...
    return (amt - amtLeft);
}
} // namespace


int64 cSerializerUtil::Copy(iSerializer* pDest, cArchive* pSrc, int64 amt)
{
    return CopyImpl(pDest, pSrc, amt);
}

int64 cSerializerUtil::Copy(cArchive* pDest, iSerializer* pSrc, int64 amt)
{
    return CopyImpl(pDest, pSrc, amt);
}
