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
#include "stdtripwire.h"
#include "core/debug.h"
#include "mailmessage.h"
#include "core/fsservices.h"

#include "tripwirestrings.h"
#include "core/msystem.h"
#include "core/stringutil.h"
#include "tw/configfile.h"

#include "tw/twutil.h"

//
// This file should not be compiled under anything but UNIX OS's
//

///////////////////////////////////////////////////////////////////////////////
//
// Constructor.  Not much to do beside initialize the handle to the DLL
//
cPipedMailMessage::cPipedMailMessage(const TSTRING& strSendMailExePath)
{
    mstrSendMailExePath = strSendMailExePath;
    mpFile              = NULL;
}


///////////////////////////////////////////////////////////////////////////////
//
// destructor - make sure the DLL has been unloaded
//
cPipedMailMessage::~cPipedMailMessage()
{
}


///////////////////////////////////////////////////////////////////////////////
//
// Check to make sure the message is adequately defined and send it.
//
bool cPipedMailMessage::Send()
{
    // Be sure that everything that needs to be set has been set
    if (!Ready())
    {
        // the message has not been adequately defined and cannot be sent.
        return false;
    }

    SendInit();

    ASSERT(mpFile != 0);
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Get Body and Attachments
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // get body
    std::string sNBody = cStringUtil::TstrToStr(mstrBody);

    // get attachments
    std::string sAttachments;
    if (!GetAttachmentsAsString(sAttachments))
    {
        sAttachments.erase();
    }

    std::string sSend = sNBody + sAttachments;


    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Make sure that there's no lone LFs
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    cMailMessageUtil::LFToCRLF(sSend);

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Determine encoding needed for body or attachments
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    std::string sContentType = "Content-Transfer-Encoding: ";

    if (cMailMessageUtil::HasNonAsciiChars(sSend))
    {
        // encode text
        sSend = iMimeEncoding::GetInstance()->Encode(sSend, cMailMessageUtil::_MAX_RFC822_LINE_LEN);

        // identify content type
        sContentType += iMimeEncoding::GetInstance()->GetContentTypeIdentifier();
    }
    else
    {
        // do no encoding

        // identify content type
        sContentType += "7bit";
    }

    // send content type
    sContentType += "\r\n";
    SendString(cStringUtil::StrToTstr(sContentType));

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Send Body and Attachments
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    SendString(_T("\r\n"));
    SendString(cStringUtil::StrToTstr(sSend));

    SendFinit();

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// SendInit
///////////////////////////////////////////////////////////////////////////////
void cPipedMailMessage::SendInit() // throw( eMailMessageError )
{
    ASSERT(mpFile == 0);

    TSTRING strHeader;
    strHeader += cStringUtil::StrToTstr(cMailMessage::Create822Header());

#if USES_MPOPEN
    // call mpopen, our safe version popen
    mpFile = mpopen((char*)mstrSendMailExePath.c_str(), _T("w"));    
#elif HAVE_POPEN
    mpFile = popen(mstrSendMailExePath.c_str(), _T("w"));
#else
    // No pipes for you, sorry.
#endif
    if (!mpFile)
    {
        TOSTRINGSTREAM estr;
        estr << TSS_GetString(cTripwire, tripwire::STR_ERR2_MAIL_MESSAGE_COMMAND) << mstrSendMailExePath;

        throw eMailPipedOpen(estr.str());
    }

    SendString(strHeader);
}

void cPipedMailMessage::SendString(const TSTRING& s)
{
    if (_ftprintf(mpFile, "%s", s.c_str()) < 0)
    {
        TOSTRINGSTREAM estr;
        estr << TSS_GetString(cTripwire, tripwire::STR_ERR2_MAIL_MESSAGE_COMMAND) << mstrSendMailExePath;

        throw eMailPipedOpen(estr.str());
    }
}

///////////////////////////////////////////////////////////////////////////////
// SendFinit
///////////////////////////////////////////////////////////////////////////////
void cPipedMailMessage::SendFinit() //throw ( eMailMessageError )
{
    if (mpFile)
    {
#if !USES_MPOPEN
        int result = fclose(mpFile);
#else
        int result = mpclose(mpFile);
#endif
        if (result != 0)
        {
            TOSTRINGSTREAM estr;
            estr << TSS_GetString(cTripwire, tripwire::STR_ERR2_MAIL_MESSAGE_COMMAND) << mstrSendMailExePath;

            // uh oh! something bad has happened!
            throw eMailPipedCmdFailed(estr.str());
        }
    }
    mpFile = 0;
}
