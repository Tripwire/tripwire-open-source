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
#ifndef __MAILMESSAGE_H
#define __MAILMESSAGE_H

#ifndef __ERROR_H
#include "core/error.h"
#endif

#if SUPPORTS_NETWORKING && HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifndef SOCKET
#define SOCKET int
#endif

///////////////////////////////////////////////////////////////////////////////
//
// This error class is thrown by cMailMessage and its derivatives.
//
TSS_EXCEPTION(eMailMessageError, eError);

// SMTP
TSS_EXCEPTION(eMailSMTPWinsockDidNotStart, eMailMessageError);
TSS_EXCEPTION(eMailSMTPWinsockUnloadable, eMailMessageError);
TSS_EXCEPTION(eMailSMTPWinsockCleanup, eMailMessageError);
TSS_EXCEPTION(eMailSMTPIPUnresolvable, eMailMessageError);
TSS_EXCEPTION(eMailSMTPSocket, eMailMessageError);
TSS_EXCEPTION(eMailSMTPOpenConnection, eMailMessageError);
TSS_EXCEPTION(eMailSMTPCloseConnection, eMailMessageError);
TSS_EXCEPTION(eMailSMTPServer, eMailMessageError);
TSS_EXCEPTION(eMailSMTPNotSupported, eMailMessageError);

// piped
TSS_EXCEPTION(eMailPipedOpen, eMailMessageError);
TSS_EXCEPTION(eMailPipedWrite, eMailMessageError);
TSS_EXCEPTION(eMailPipedCmdFailed, eMailMessageError);

///////////////////////////////////////////////////////////////////////////////
//
// This is the pure virtual base class that defines how to
// send an email message. No class derived from this one should
// have any new public methods (or data).
//
class cMailMessage
{
public:
    cMailMessage();
    virtual ~cMailMessage();

    void AddRecipient(const TSTRING& strRecipient);
    void SetFrom(const TSTRING& strFrom);
    void SetFromName(const TSTRING& strFromName);
    void SetSubject(const TSTRING& strSubject);
    void SetBody(const TSTRING& strBody);
    void AttachFile(const TSTRING& strFullPath);

    virtual bool Send() = 0; //throw(eMailMessageError)
        // returns true upon success

    // This enumerates the choices of email notification methods
    enum MailMethod
    {
        NO_METHOD,
        INVALID_METHOD,
        MAIL_BY_SMTP,
        MAIL_BY_PIPE,
        MAIL_NUM_ITEMS
    };


protected:
    TSTRING              mstrFrom;
    TSTRING              mstrFromName;
    TSTRING              mstrSubject;
    TSTRING              mstrBody;
    std::vector<TSTRING> mvstrRecipients;
    std::vector<TSTRING> mvstrAttachments;

    // Returns true if enough things have been set so that a useful
    // email message can be sent
    bool        Ready();
    std::string Create822Header();
    // creates 822 header with info from the class
    bool GetAttachmentsAsString(std::string& s);
};

class cMailMessageUtil
{

public:
    enum
    {
        _MAX_RFC822_LINE_LEN   = 76,
        _MAX_RFC822_HEADER_LEN = 76,
        _EOL_LEN               = 2
    };

    static const std::string::value_type* ConvertBase64(std::string&, const uint8_t*, size_t);

    static bool        HasNonAsciiChars(const std::string& s);
    static std::string CreateEncodedText(const std::string& text);

    static std::string FormatAddressHeader(const std::string& name, const std::string& addr);
    static std::string FormatNonAddressHeader(const std::string& name, const std::string& val);

    static bool         ReadDate(TSTRING& strDateBuf);
    static std::string& LFToCRLF(std::string& s);
    // converts lone \n's to \r\n
};

class iMimeEncoding
{
public:
    virtual ~iMimeEncoding()
    {
    }

    virtual std::string Encode(const std::string& sIn, int maxLineLen = -1, int maxLen = -1) = 0;
    // -1 means no limit

    virtual std::string GetEncodedWordIdentifier() = 0;

    virtual std::string GetContentTypeIdentifier() = 0;

    static iMimeEncoding* GetInstance();
};

// TODO:BAM -- convert ToBase64 to Encode
class cBase64Encoding : public iMimeEncoding
{
public:
    virtual ~cBase64Encoding()
    {
    }

    virtual std::string Encode(const std::string& sIn, int maxLineLen = -1, int maxLen = -1)
    {
        ASSERT(false);
        return "";
    };

    virtual std::string GetEncodedWordIdentifier()
    {
        return "B";
    };

    virtual std::string GetContentTypeIdentifier()
    {
        return "base64";
    };
};

class cQuotedPrintableEncoding : public iMimeEncoding
{
public:
    virtual ~cQuotedPrintableEncoding()
    {
    }

    virtual std::string Encode(const std::string& sIn, int maxLineLen = -1, int maxLen = -1);

    virtual std::string GetEncodedWordIdentifier()
    {
        return "Q";
    };

    virtual std::string GetContentTypeIdentifier()
    {
        return "quoted-printable";
    };
};


#if SUPPORTS_NETWORKING
///////////////////////////////////////////////////////////////////////////////
//
// This class implements sending a message via SMTP
//
class cSMTPMailMessage : public cMailMessage
{
public:
    cSMTPMailMessage(TSTRING strServerName, unsigned short portNumber);
    virtual ~cSMTPMailMessage();

    virtual bool Send(); //throw(eMailMessageError)
                         // returns true upon success

private:
    // socket related member variables
    SOCKET mSocket;
    void   SendString(const std::string& str);

    // methods common to windows and unix:
    bool OpenConnection();
    bool CloseConnection();

    long GetServerAddress();

    bool MailMessage();
    bool SendAttachments();

    bool GetAcknowledgement();

    void DecodeError();

    // the settings necessary for an SMTP connection:
    TSTRING        mstrServerName;
    unsigned short mPortNumber;
};
#endif

///////////////////////////////////////////////////////////////////////////////
//
// This class implements sending a message through a unix pipe to a program
// like 'sendmail'
//
class cPipedMailMessage : public cMailMessage
{
public:
    explicit cPipedMailMessage(const TSTRING& strSendMailExePath);
    virtual ~cPipedMailMessage();

    virtual bool Send(); //throw(eMailMessageError)
                         // returns true upon success

private:
    TSTRING& CreateHeader(TSTRING& strHeaderBuf) const;

    void SendInit(); // throw (eMailMessageError)
        // opens a pipe to sendmail and writes the header.

    void SendFinit(); //throw (eMailMessageError)
        // closes the file descriptor, sending the rest of the message

    bool SendAttachments();
    // called between SendInit and SendFinit to send the attachments

    void SendString(const TSTRING& s);

    TSTRING mstrSendMailExePath;
    FILE*   mpFile; // only valid with Init/Finit and Send
};

//#endif

#endif // __MAILMESSAGE_H
