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
#include "stdtripwire.h"
#include "core/debug.h"
#include "mailmessage.h"
#include "tw/configfile.h"
#include "tw/twutil.h"
#include "tripwirestrings.h"
#include "core/stringutil.h"
#include <sstream>
#include "core/msystem.h"
#include "core/file.h"

#if IS_UNIX

#include <time.h>

//All the spleck that it takes to run sockets in Unix...
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/utsname.h>

#ifdef _SORTIX_SOURCE
# include <sys/select.h>
#endif

/* Some systems like Solaris and AIX don't define
 * INADDR_NONE, but it's pretty standard.  If not,
 * then the OS _should_ define it for us.
 */
#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

#include <unistd.h>

#define INVALID_SOCKET -1

#ifdef __AROS__
 #ifndef HAVE_GETHOSTNAME
  #define HAVE_GETHOSTNAME 1
 #endif
#endif

#ifndef HAVE_GETHOSTNAME
static int gethostname( char* name, int namelen )
{
    name[0] = '\0';
    
    struct utsname myname;
    uname( & myname );
    
    if ( strlen( myname.nodename ) < (unsigned int)namelen )
    {
        strcpy( name, myname.nodename );
        return 0;
    }
    else
    {
        //Not enough room in the buffer for the nodename
        return -1;
            // equivalent of SOCKET_ERROR
    }
}
#endif
    // Unix does not require us to go though any silly DLL hoops, so we'll
    // just #define the pointers to functions needed by Windows to be the 
    // berkely functions.
    #define mPfnSocket          socket
    #define mPfnInetAddr        inet_addr
    #define mPfnGethostname     gethostname
    #define mPfnGethostbyname   gethostbyname
    #define mPfnConnect         connect
    #define mPfnCloseSocket     close
    #define mPfnSend            send
    #define mPfnRecv            recv
    #define mPfnSelect          select
    #define mPfnNtohl           ntohl
    #define mPfnHtonl           htonl
    #define mPfnNtohs           ntohs
    #define mPfnHtons           htons
#endif

//
// TODO - maybe convert this SMTP code to non-blocking socket calls, or use
// another thread, or make it fail gracefully when the server fails to 
// respond at all.
//

///////////////////////////////////////////////////////////////////////////////
//
// Construct the SMTP Mail Message Sender
//
cSMTPMailMessage::cSMTPMailMessage(TSTRING strServerName, unsigned short portNumber)
{
    mstrServerName      = strServerName;
    mPortNumber         = portNumber;
    mSocket             = INVALID_SOCKET;

#if USES_WINSOCK
    mHlibWinsock        = NULL;

    mPfnSocket          = NULL;
    mPfnInetAddr        = NULL;
    mPfnGethostname     = NULL;
    mPfnGethostbyname   = NULL;
    mPfnConnect         = NULL;
    mPfnCloseSocket     = NULL;
    mPfnSend            = NULL;
    mPfnRecv            = NULL;
    mPfnSelect          = NULL;

    mPfnWSAStartup      = NULL;
    mPfnWSACleanup      = NULL;
    mPfnWSAGetLastError = NULL;
    
    mPfnNtohl           = NULL;
    mPfnHtonl           = NULL;
    mPfnNtohs           = NULL;
    mPfnHtons           = NULL;
#endif // USES_WINSOCK
}


///////////////////////////////////////////////////////////////////////////////
//
// Clean up any thing left over from sending or failing to send the message.
//
cSMTPMailMessage::~cSMTPMailMessage()
{
}




///////////////////////////////////////////////////////////////////////////////
//
// Get the IP address from the the server string. It's OK to have 
// this function look up a string like "192.34.64.23" or one like 
// "mail.stinkycheese.com"
//
long cSMTPMailMessage::GetServerAddress()
{
    bool bIsNumeric = true;

    // Decide if the string is in the form "127.0.0.1" or "mail.domain.com"
    // by looking for an character that is not a digit or a period.
    for (std::vector<TSTRING>::size_type i=0; i < mstrServerName.length(); i++)
    {
        if (mstrServerName[i] != '.' && (mstrServerName[i]<'0' || mstrServerName[i]>'9'))
        {
            bIsNumeric = false;
        }
    }

    std::string sNarrowString = cStringUtil::TstrToStr(mstrServerName);

    if (bIsNumeric)
    {
        // convert the numberic address to a long
        return mPfnInetAddr(sNarrowString.c_str());
    }
    else
    {
#ifdef _SORTIX_SOURCE
        return INADDR_NONE;
#else
        // do a DNS lookup of the hostname and get the long
        hostent *ent = mPfnGethostbyname(sNarrowString.c_str());
        if (!ent)
            return INADDR_NONE;
        else
            return *(long *)ent->h_addr_list[0];
#endif
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// Create and open the socket connection
//
bool cSMTPMailMessage::OpenConnection()
{
    // Initialize the socket structure
    sockaddr_in sockAddrIn;
    memset(&sockAddrIn, 0, sizeof(sockaddr));
    sockAddrIn.sin_family           = AF_INET;
    sockAddrIn.sin_port             = mPfnHtons(mPortNumber);
    uint32 iServerAddress = GetServerAddress();

#if IS_UNIX
    sockAddrIn.sin_addr.s_addr = iServerAddress;
#endif
    if ( iServerAddress  == INADDR_NONE ) 
    {
        DecodeError();

        TOSTRINGSTREAM estr;
        estr << TSS_GetString( cTripwire, tripwire::STR_ERR2_MAIL_MESSAGE_SERVER ) 
             << mstrServerName;

        throw eMailSMTPIPUnresolvable(estr.str());
        return false;
    }
    
    // Create the socket
    mSocket = mPfnSocket(AF_INET, SOCK_STREAM, 0);
    if (mSocket == INVALID_SOCKET) 
    {
        DecodeError();
        throw eMailSMTPSocket();
        return false;
    }

    // Make the connection
    int connectVal = mPfnConnect(mSocket, (struct sockaddr *)&sockAddrIn, sizeof(sockAddrIn));
    if (connectVal < 0) 
    {
        DecodeError();

        TOSTRINGSTREAM estr;
        estr << TSS_GetString( cTripwire, tripwire::STR_ERR2_MAIL_MESSAGE_SERVER ) 
             << mstrServerName;

        throw eMailSMTPOpenConnection(estr.str());
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
//
// Close the socket connection
//
bool cSMTPMailMessage::CloseConnection()
{
    if ( INVALID_SOCKET != mSocket )
    {
        // close the connection
        int closeVal = mPfnCloseSocket(mSocket);
        if (closeVal != 0) 
        {
            DecodeError();
            throw eMailSMTPCloseConnection();
            return false;
        }
        return true;
    }
    else
    {
        return true;
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// Call this function to send the mail message once the requisite
// methods have been called to define the mail message.
//
bool cSMTPMailMessage::Send()
{
    // Be sure that everything that needs to be set has been set
    if (!Ready()) 
    {
        // the message has not been adequately defined and cannot be sent.
        return false;
    }


    bool errorOccured = false;

    if ((errorOccured = !OpenConnection()) == false)
    {
        errorOccured |= !MailMessage();
        errorOccured |= !CloseConnection();
    }


    return !errorOccured;
}


///////////////////////////////////////////////////////////////////////////////
// 
// Once the connection is set, this function conducts the SMTP protocol 
// to actually send the message.
//
bool cSMTPMailMessage::MailMessage()
{
    cDebug d("cSMTPMailMessage::MailMessage()");
    std::string sNarrowString;

    // Get the greeting message from the SMTP server
    if (!GetAcknowledgement())
        return false;

    char sLocalHost[256];   // It's alright for this to be a fixed buffer, since
                            // we will be explicitely passing it's length to 
                            // mpfnGethostname (see below).  It won't be used
                            // after that.

    std::ostringstream strmSend;
                            // This should be a stream object, so we don't have
                            // to use nasty calls to sprintf that might overflow
                            // the buffer.  Before, we used a fixed buffer of 512
                            // characters, and there's really no guarantee that any
                            // of the string objects (that we are printing to the buffer
                            // from) will be below this limit.

    ASSERT( strmSend.str().length() == 0 ); // This bad boy better be empty.

    // get our hostname for the HELO message
    if (mPfnGethostname(sLocalHost, 256) < 0 ) {
        DecodeError();
        return false;
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Set up connection
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // Say hello
    strmSend.str("");   //Clear the stream buffer.
    strmSend << "HELO " << sLocalHost << "\r\n";    //Fill the stream buffer.
    SendString( strmSend.str() );
    if (!GetAcknowledgement())
        return false;


    strmSend.str("");   //Clear the stream buffer.
    strmSend    << "MAIL FROM:<" 
                << cStringUtil::TstrToStr(mstrFrom)
                << ">\r\n";
    SendString( strmSend.str() );
    if (!GetAcknowledgement())
        return false;

    // Say who all we're sending to
    strmSend.str("");   //Clear the stream buffer.
    for (std::vector<TSTRING>::size_type i=0; i<mvstrRecipients.size(); i++) 
    {
        sNarrowString = cStringUtil::TstrToStr(mvstrRecipients[i]);
        strmSend    << "RCPT TO:<" 
                    << cStringUtil::TstrToStr(mvstrRecipients[i]) 
                    << ">\r\n";

        SendString( strmSend.str() );

        if (!GetAcknowledgement())
            return false;
    }

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Start data
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // Send start the message process
    SendString( "DATA\r\n" );
    if (!GetAcknowledgement())
        return false;
    
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Send Header
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // set up header
    strmSend.str("");
    strmSend << cMailMessage::Create822Header();
    SendString( strmSend.str() );

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Get Body and Attachments
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    bool allOK = true;


    // get body
    std::string sNBody = cStringUtil::TstrToStr( mstrBody );

    std::string sAttachments;
    if( ! GetAttachmentsAsString( sAttachments ) )
    {
        sAttachments.erase();
        allOK = false;
    }
    
    std::string sSend = sNBody + sAttachments;
    

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Make sure that there's no lone LFs
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    cMailMessageUtil::LFToCRLF( sSend );    

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Determine encoding needed for body or attachments
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    std::string sContentType = "Content-Transfer-Encoding: ";

    if( cMailMessageUtil::HasNonAsciiChars( sSend ) )
    {
        // encode text
        sSend = iMimeEncoding::GetInstance()->Encode( sSend, 
                        cMailMessageUtil::_MAX_RFC822_LINE_LEN );

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
    SendString( sContentType );

    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Send Body and Attachments
    //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    SendString( "\r\n" );
    SendString( sSend );

    // send the end of message line
    SendString( "\r\n.\r\n" );
    if (!GetAcknowledgement())
        return false;

    // send the quit message
    SendString( "QUIT" );

    return allOK;
}


///////////////////////////////////////////////////////////////////////////////
//
// Get the response code from the server to see if the last command
// we sent was accepted.
//
bool cSMTPMailMessage::GetAcknowledgement()
{
    cDebug d( "cSMTPMailMessage::GetAcknowledgement" );

    const int bufsize = 512;
    TCHAR sRecvString[bufsize+1]; // This string is and should be unicode
    char  sTempString[bufsize+1]; // This string is not, and should not be unicode
    int bytes;
    int i=0;

    // make socket array for the call to select
    fd_set socketSet;
    // need comment
    timeval tv;


#if IS_UNIX
    FD_ZERO( &socketSet );
    FD_SET( mSocket, &socketSet );
#else
    socketSet.fd_count = 1;
    socketSet.fd_array[0] = mSocket;
#endif

    // set the timeout time to sixty seconds
    tv.tv_sec  = 60;
    tv.tv_usec = 0;

    // Wait up to sixty seconds fot data to show up on the socket to be read
    if (mPfnSelect(mSocket+1, &socketSet, NULL, NULL, &tv) == 1)
    {
        // Get the reply message
        bytes = mPfnRecv(mSocket, sTempString, 512, 0);
        
        // TODO:BAM -- this should be changed to use 'cStringUtil'
        for (int j=0; j<bytes && i<bufsize; j++, i++)
            sRecvString[i] = sTempString[j];
        sRecvString[i] = 0;
        
        std::string sIn( sTempString, bytes );
        d.TraceDebug( "Received \"%s\"\n", sIn.c_str() );
    }
    else
    {
        d.TraceDebug( "No Receive\n" );
    }

    // decode the numeric reply
    int code = _ttoi(sRecvString);
    if (code >= 200 && code < 400) 
    {
        // Error codes in the range of 200-399 indicate success.  See RFC 821
        return true;
    }
    else
    {
        // Error codes other than 200-399 indicate an error or a failure.  See RFC 821
        TOSTRINGSTREAM estr;
        estr << TSS_GetString( cTripwire, tripwire::STR_ERR2_MAIL_MESSAGE_SERVER_RETURNED_ERROR ) 
             << sRecvString;

        throw eMailSMTPServer(estr.str());
        return false;
    }
}

void cSMTPMailMessage::SendString( const std::string& str )
{
    cDebug d("util_SendString()");

    if( str.length() < 800 )
        d.TraceDebug( "Sending \"%s\"\n", str.c_str() );
    else
        d.TraceDebug( "Sending (truncated in this debug output)\"%s\"\n", std::string( str.c_str(), 800 ).c_str() );
    mPfnSend( mSocket, str.c_str(), str.length(), 0 );
}



///////////////////////////////////////////////////////////////////////////////
//
// Get debug info when a error is encountered.
//
void cSMTPMailMessage::DecodeError()
{
#if defined(_DEBUG)

#if USES_WINSOCK
    //
    // decode an error condition encountered by Windows sockets.
    // TODO -  write something less technical to explain what when wrong. Windows
    // message names are not good error strings for a shipping product.
    //
    int error = mPfnWSAGetLastError();
    fprintf(stderr, "Encountered winsock error message %d", error);
    struct ErrorStruct 
    {
        const char *msg;
        int error;
    }
    static const errors[52] = 
    {
        { "WSAEINTR",                (WSABASEERR+4) },
        { "WSAEBADF",                (WSABASEERR+9) },
        { "WSAEACCES",               (WSABASEERR+13) },
        { "WSAEFAULT",               (WSABASEERR+14) },
        { "WSAEINVAL",               (WSABASEERR+22) },
        { "WSAEMFILE",               (WSABASEERR+24) },
        { "WSAEWOULDBLOCK",          (WSABASEERR+35) },
        { "WSAEINPROGRESS",          (WSABASEERR+36) },
        { "WSAEALREADY",             (WSABASEERR+37) },
        { "WSAENOTSOCK",             (WSABASEERR+38) },
        { "WSAEDESTADDRREQ",         (WSABASEERR+39) },
        { "WSAEMSGSIZE",             (WSABASEERR+40) },
        { "WSAEPROTOTYPE",           (WSABASEERR+41) },
        { "WSAENOPROTOOPT",          (WSABASEERR+42) },
        { "WSAEPROTONOSUPPORT",      (WSABASEERR+43) },
        { "WSAESOCKTNOSUPPORT",      (WSABASEERR+44) },
        { "WSAEOPNOTSUPP",           (WSABASEERR+45) },
        { "WSAEPFNOSUPPORT",         (WSABASEERR+46) },
        { "WSAEAFNOSUPPORT",         (WSABASEERR+47) },
        { "WSAEADDRINUSE",           (WSABASEERR+48) },
        { "WSAEADDRNOTAVAIL",        (WSABASEERR+49) },
        { "WSAENETDOWN",             (WSABASEERR+50) },
        { "WSAENETUNREACH",          (WSABASEERR+51) },
        { "WSAENETRESET",            (WSABASEERR+52) },
        { "WSAECONNABORTED",         (WSABASEERR+53) },
        { "WSAECONNRESET",           (WSABASEERR+54) },
        { "WSAENOBUFS",              (WSABASEERR+55) },
        { "WSAEISCONN",              (WSABASEERR+56) },
        { "WSAENOTCONN",             (WSABASEERR+57) },
        { "WSAESHUTDOWN",            (WSABASEERR+58) },
        { "WSAETOOMANYREFS",         (WSABASEERR+59) },
        { "WSAETIMEDOUT",            (WSABASEERR+60) },
        { "WSAECONNREFUSED",         (WSABASEERR+61) },
        { "WSAELOOP",                (WSABASEERR+62) },
        { "WSAENAMETOOLONG",         (WSABASEERR+63) },
        { "WSAEHOSTDOWN",            (WSABASEERR+64) },
        { "WSAEHOSTUNREACH",         (WSABASEERR+65) },
        { "WSAENOTEMPTY",            (WSABASEERR+66) },
        { "WSAEPROCLIM",             (WSABASEERR+67) },
        { "WSAEUSERS",               (WSABASEERR+68) },
        { "WSAEDQUOT",               (WSABASEERR+69) },
        { "WSAESTALE",               (WSABASEERR+70) },
        { "WSAEREMOTE",              (WSABASEERR+71) },
        { "WSAEDISCON",              (WSABASEERR+101) },
        { "WSASYSNOTREADY",          (WSABASEERR+91) },
        { "WSAVERNOTSUPPORTED",      (WSABASEERR+92) },
        { "WSANOTINITIALISED",       (WSABASEERR+93) },
        { "WSAHOST_NOT_FOUND",       (WSABASEERR+1001) },
        { "WSATRY_AGAIN",            (WSABASEERR+1002) },
        { "WSANO_RECOVERY",          (WSABASEERR+1003) },
        { "WSANO_DATA",              (WSABASEERR+1004) },
        { NULL, 0 }
    };

    int i = 0;
    while (errors[i].msg) 
    {
        if (errors[i].error == error) 
        {
            fprintf(stderr, ": %s", errors[i].msg);
            break;
        } 
        i++;
    }
    fprintf(stderr, "\n");
#endif // USES_WINSOCK

#if IS_UNIX

    //
    // TODO - Write what ever error reporting will be needed under unix.
    //

#endif // IS_UNIX

#endif // defined(_DEBUG)
}

