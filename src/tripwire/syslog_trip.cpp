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
///////////////////////////////////////////////////////////////////////////////
// syslog.cpp
//

#include "stdtripwire.h"
#include "syslog_trip.h" 

#if HAVE_SYSLOG_H
#include <syslog.h>
#endif

#if HAVE_SYS_SYSLOG_H
#include <sys/syslog.h>
#endif

#if SUPPORTS_EVENTLOG
#include "tripwiremsg.h"
#include "tw/systeminfo.h"
#endif

// next three includes are for error reporting
#include "tw/twutil.h"
#include "tw/twerrors.h"
#include "tw/twstrings.h"

///////////////////////////////////////////////////////////////////////////////
// Syslog
///////////////////////////////////////////////////////////////////////////////

#if SUPPORTS_EVENTLOG
static void InitEventLog();
#endif

void cSyslog::Log(const TCHAR* programName, cSyslog::LogType logType, const TCHAR* message)
{
#if HAVE_SYSLOG_H

    (void)logType; // logType is not used for Unix syslog

#ifdef _UNICODE
    std::basic_string identString;
    std::basic_string msgString;
    int count;

    count = sizeof(char) * MB_CUR_MAX * _tcslen(programName); // note we use sizeof(char) * 2 because mb chars can be two bytes long
    identString.resize(count); 
	count = wcstombs((char*)identString.data(), programName, count);
    identString.resize(count); // count will be set to number of bytes written

    count = sizeof(char) * MB_CUR_MAX * _tcslen(message); 
    msgString.resize(count); 
	count = wcstombs((char*)msgString.data(), message, count);
    msgString.resize(count);

    const char* ident = programName.data();
    const char* msg = message.data();
#else
    ASSERT(sizeof(TCHAR) == sizeof(char));
    const char* ident = programName;
    const char* msg = message;
#endif

    openlog(ident, LOG_PID, LOG_USER);
    syslog(LOG_NOTICE, "%s", msg);
    closelog();

#elif SUPPORTS_EVENTLOG

    static bool eventLogInitialized = false;
    if (!eventLogInitialized)
    {
        InitEventLog();
        eventLogInitialized = true;
    }

    HANDLE h = RegisterEventSource(NULL, _T("Tripwire"));
    if (h != NULL)
    {
        LPCTSTR stringArray[1];
        stringArray[0] = message;

        WORD type;
        DWORD id;

        switch (logType)
        {
        default:
            ASSERT(false);
        case LOG_SUCCESS:
            type = EVENTLOG_INFORMATION_TYPE;
            id = MSG_TRIPWIRE_GENERIC_SUCCESS;
            break;
        case LOG_INFORMATION:
            type = EVENTLOG_INFORMATION_TYPE;
            id = MSG_TRIPWIRE_GENERIC_INFO;
            break;
        case LOG_WARNING:
            type = EVENTLOG_WARNING_TYPE;
            id = MSG_TRIPWIRE_GENERIC_WARNING;
            break;
        case LOG_ERROR:
            type = EVENTLOG_ERROR_TYPE;
            id = MSG_TRIPWIRE_GENERIC_ERROR;
            break;
        }

        BOOL ret = ReportEvent(
            h,
            type,                       // event type
            0,                          // catagory
            id,                         // event id
            0,                          // user sid
            1,                          // num strings
            0,                          // raw binary data size
            stringArray,                // array of strings
            0                           // raw binrary data
            );

        if (!ret)
        {
            eTWSyslog e( TSS_GetString(cTW, tw::STR_REPORTEVENT_FAILED).c_str(), eError::NON_FATAL );
            cTWUtil::PrintErrorMsg(e);
        }

        DeregisterEventSource(h);
    }

#else
    // No support for syslog like functionality
    //ASSERT(false);
#endif
}


#if SUPPORTS_EVENTLOG
static void InitEventLog()
{
    // To initialize the event log, we need to verify that Tripwire 
    // is a valid event source.
    // To do this we look up the value for the Tripwire event source.  If it 
    // exists, the the dest is a file called "tripwire.exe" (or this executable) and the
    // file exists, then we assume that the event source is set correctly.

    HKEY hKey;
    DWORD disposition;
 
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\Tripwire"), 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &disposition) == ERROR_SUCCESS && 
        disposition == REG_OPENED_EXISTING_KEY) 
    {
        DWORD type, count;

        // We need to insure we have back slashes in our exepath
        TSTRING exepath = cSystemInfo::GetExePath();
        TSTRING::iterator i;
        for (i = exepath.begin(); i != exepath.end(); ++i)
            if (*i == _T('/'))
                *i = _T('\\');
        
        if (RegQueryValueEx(hKey, _T("TypesSupported"), 0, &type, 0, &count) == ERROR_SUCCESS &&
            type == REG_DWORD &&
            RegQueryValueEx(hKey, _T("EventMessageFile"), 0, &type, 0, &count) == ERROR_SUCCESS &&
            type == REG_EXPAND_SZ)
        {
            TSTRING data;
            data.resize(count);
            if (RegQueryValueEx(hKey, _T("EventMessageFile"), 0, &type, (LPBYTE)data.data(), &count) == ERROR_SUCCESS &&
                type == REG_EXPAND_SZ)
            {
                TSTRING::size_type lastDelimitor;
                lastDelimitor = data.find_last_of(_T('\\'));
                if (lastDelimitor == TSTRING::npos)
                    lastDelimitor = (TSTRING::size_type)-1;

                if (_tcscmp(exepath.c_str(), data.c_str()) == 0)
                {
                    RegCloseKey(hKey);
                    return;
                }

                if (_tcsicmp(_T("tripwire.exe"), data.substr(lastDelimitor + 1).c_str()) == 0)
                {
                    HINSTANCE hInst = LoadLibraryEx(data.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES|LOAD_LIBRARY_AS_DATAFILE);
                    if (hInst != NULL)
                    {
                        FreeLibrary(hInst);
                        RegCloseKey(hKey);
                        return;
                    }
                }
            }
        }

        // If we got here then the event source is not set up correctly
 
        // Add the name to the EventMessageFile subkey. 
        RegSetValueEx(hKey,             // subkey handle 
                _T("EventMessageFile"),       // value name 
                0,                        // must be zero 
                REG_EXPAND_SZ,            // value type 
                (LPBYTE) exepath.c_str(),           // pointer to value data 
                sizeof(TCHAR)*(exepath.length() + 1));       // length of value data 
 
        // Set the supported event types in the TypesSupported subkey. 
        DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE; 
 
        RegSetValueEx(hKey,      // subkey handle 
                _T("TypesSupported"),  // value name 
                0,                 // must be zero 
                REG_DWORD,         // value type 
                (LPBYTE) &dwData,  // pointer to value data 
                sizeof(DWORD));    // length of value data 
 
        RegCloseKey(hKey); 
    }
}
#endif

