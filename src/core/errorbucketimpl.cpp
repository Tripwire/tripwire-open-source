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
// errorbucketimpl.cpp
#include "stdcore.h"
#include "errorbucketimpl.h"
#include "errortable.h"
#include "serializer.h"
#include "corestrings.h"
#include "displayencoder.h"

//#############################################################################
// cErrorBucket
//#############################################################################
void cErrorBucket::AddError(const eError& error)
{
    HandleError(error);
    if (mpChild)
        mpChild->AddError(error);
}

//#############################################################################
// cErrorReporter
//#############################################################################
void cErrorReporter::PrintErrorMsg(const eError& error, const TSTRING& strExtra)
{
    cDisplayEncoder e((cDisplayEncoder::Flags)(cDisplayEncoder::NON_ROUNDTRIP | cDisplayEncoder::ALLOW_WHITESPACE));
    TSTRING         errStr;

    //
    // if the ID is zero, just return.
    // this should only occur at the top level of a program (ie -- twcmdline.cpp) and
    // indicates that an error occurred and an error message has already been printed out.
    // Therefore, we will do nothing here but return.
    //

    // TODO: Having an error with an ID of 0 is legacy.  The only place it happens at this
    // point is when we throw ePoly() with no constructor arguments.  At some point we want
    // to stop using the mechanism have non-printing errors, thus we leave in the ASSERT below.
    // But we don't want to break any release code, thus we return on the next line - June 2, 1999 DMB.
    ASSERT(error.GetID() != 0);

    if (error.GetID() == 0)
        return;

    // "First Part" header
    errStr = TSS_GetString(cCore, error.IsFatal() ? core::STR_ERROR_ERROR : core::STR_ERROR_WARNING);

    if (errStr.empty())
    {
        TOSTRINGSTREAM strm;
        ASSERT(sizeof(uint32_t) == sizeof(unsigned int)); // for cast on next line
        strm << _T("Unknown Error ID ") << (unsigned int)error.GetID();
	tss_stream_to_string(strm, errStr);
    }

    //int len = errStr.length(); // save for later
    TCERR << errStr;

    // "First Part" error string
    TSTRING prependToSecond;

    // #pragma message("errorbucketimpl.cpp needs a little help in the mb arena, with the findfirst/last and such")

    errStr = cErrorTable::GetInstance()->Get(error.GetID());
    if (!errStr.empty())
    {
        // If the first part has a '\n' in it, we take everything following and prepend it to the
        // second part.  This was added to allow specifing a verbose string as the second part
        // of an error message when building the error table.  This change was made on July 27, 1999
        TSTRING::size_type firstLF;
        if ((firstLF = errStr.find_first_of(_T('\n'))) != TSTRING::npos)
        {
            prependToSecond = errStr.substr(firstLF + 1); // don't include '\n' in new string
            errStr.erase(firstLF);
        }

        ASSERT(errStr.length() + 6 < 80); // line too big for terminal?
                                                // Add 6 to account for "### ' and ': '
        TCERR << TSS_GetString(cCore, core::STR_ERROR_COLON) << _T(" ") << errStr;
        TCERR << std::endl;
    }

    // "Second Part" error string
    const int   WIDTH = 80 - 4; // allow for "### " prefix to every error line
    const TCHAR SPACE = _T(' ');

    errStr = prependToSecond + error.GetMsg() + strExtra;
    e.Encode(errStr);
    if (!errStr.empty())
    {
        do
        {
            // look for newline chars
            TSTRING::size_type firstNewLine;
            TSTRING            currentStr = errStr.substr(0, (firstNewLine = errStr.find_first_of(_T('\n'))));
            errStr.erase(0, (firstNewLine < errStr.length() ? firstNewLine + 1 : firstNewLine));

            // break up the error string in case it is larger than screen width
            do
            {
                if (currentStr.length() <= (unsigned int)WIDTH)
                {
                    TCERR << TSS_GetString(cCore, core::STR_ERROR_HEADER) << currentStr << std::endl;
                    break;
                }

                TSTRING::size_type lastSpace =
                    currentStr.find_last_of(SPACE, currentStr.length() >= WIDTH - 1 ? WIDTH - 1 : TSTRING::npos);
                if (lastSpace == TSTRING::npos)
                {
                    // can't find space to break at so this string will just have to be longer than screen width.
                    // search forward for a space so we break at the next even word boundry.
                    lastSpace = currentStr.find_first_of(SPACE, 0);
                    if (lastSpace == TSTRING::npos)
                        // Well, there is no space before the end of the string.
                        // So print the whole string out.
                        lastSpace = currentStr.length();
                }

                TCERR << TSS_GetString(cCore, core::STR_ERROR_HEADER) << currentStr.substr(0, lastSpace) << std::endl;

                currentStr.erase(0, lastSpace + 1);
            } while (!currentStr.empty());
        } while (!errStr.empty());
    }

    // "Third Part" print 'exiting' or 'continuing'
    // note that we supress this part if the appropriate flag is set...
    //
    if ((error.GetFlags() & eError::SUPRESS_THIRD_MSG) == 0)
    {
        TCERR << TSS_GetString(cCore, core::STR_ERROR_HEADER)
              << TSS_GetString(cCore, error.IsFatal() ? core::STR_ERROR_EXITING : core::STR_ERROR_CONTINUING)
              << std::endl;
    }
}

void cErrorReporter::HandleError(const eError& error)
{
    PrintErrorMsg(error);
}

//#############################################################################
// cErrorTracer
//#############################################################################
void cErrorTracer::HandleError(const eError& error)
{
    cDebug d("cErrorTracer::HandleError");

    d.TraceError(_T("%s : %s\n"), cErrorTable::GetInstance()->Get(error.GetID()).c_str(), error.GetMsg().c_str());
}

//#############################################################################
// cErrorQueue
//#############################################################################
IMPLEMENT_TYPEDSERIALIZABLE(cErrorQueue, _T("cErrorQueue"), 0, 1);

void cErrorQueue::Clear()
{
    mList.clear();
}

cErrorQueue::ListType::size_type cErrorQueue::GetNumErrors() const
{
    return mList.size();
}

void cErrorQueue::HandleError(const eError& error)
{
    mList.push_back(ePoly(error));
}

cErrorQueueIter::cErrorQueueIter(cErrorQueue& queue) : mList(queue.mList)
{
    SeekBegin();
}

cErrorQueueIter::cErrorQueueIter(const cErrorQueue& queue) : mList(((cErrorQueue*)&queue)->mList)
{
    SeekBegin();
}


void cErrorQueueIter::SeekBegin()
{
    mIter = mList.begin();
}

void cErrorQueueIter::Next()
{
    ++mIter;
}

bool cErrorQueueIter::Done() const
{
    return (mIter == mList.end());
}

const ePoly& cErrorQueueIter::GetError() const
{
    ASSERT(!Done());
    return (*mIter);
}

///////////////////////////////////////////////////////////////////////////////
// Read
///////////////////////////////////////////////////////////////////////////////
void cErrorQueue::Read(iSerializer* pSerializer, int32_t version)
{
    if (version > Version())
        ThrowAndAssert(eSerializerVersionMismatch(_T("ErrorQueue Read")));

    int32_t size;
    mList.clear();
    pSerializer->ReadInt32(size);
    for (int i = 0; i < size; ++i)
    {
        int32_t   errorNumber;
        TSTRING errorString;
        int32_t   flags;

        pSerializer->ReadInt32(errorNumber);
        pSerializer->ReadString(errorString);
        pSerializer->ReadInt32(flags);

        mList.push_back(ePoly(errorNumber, errorString, flags));
    }
}

///////////////////////////////////////////////////////////////////////////////
// Write
///////////////////////////////////////////////////////////////////////////////
void cErrorQueue::Write(iSerializer* pSerializer) const
{
    pSerializer->WriteInt32(mList.size());
    ListType::const_iterator i;
    for (i = mList.begin(); i != mList.end(); ++i)
    {
        pSerializer->WriteInt32((*i).GetID());
        pSerializer->WriteString((*i).GetMsg());
        pSerializer->WriteInt32((*i).GetFlags());
    }
}


///////////////////////////////////////////////////////////////////////////////
// TraceContents
///////////////////////////////////////////////////////////////////////////////
void cErrorQueue::TraceContents(int dl) const
{
    if (dl < 0)
        dl = cDebug::D_DEBUG;

    cDebug                   d("cFCOErrorQueue::TraceContents");
    ListType::const_iterator i;
    int                      counter = 0;
    for (i = mList.begin(); i != mList.end(); i++, counter++)
    {
        d.Trace(dl, _T("Error[%d]: num = %x string = %s\n"), counter, (*i).GetID(), (*i).GetMsg().c_str());
    }
}
