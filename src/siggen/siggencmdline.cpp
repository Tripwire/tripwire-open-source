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
//siggencmdline.cpp
#include "stdsiggen.h"
#include "siggencmdline.h"
#include "core/cmdlineparser.h"

#include "core/archive.h"         // cArchive and friends
#include "fco/signature.h"        // cSignature
#include "fs/fsstrings.h"         // file system related strings
#include "core/usernotify.h"      // for notifying the user of events
#include "core/errorbucketimpl.h" // for the error table
#include "core/fsservices.h"
#include "tw/twstrings.h"
#include "core/displayencoder.h"
#include "siggenstrings.h"

#include <fstream> // for the FileExists() stuff

#include <unistd.h>
#include <fcntl.h>
#if SUPPORTS_TERMIOS
#include <termios.h>
#include <sys/ioctl.h>
#endif
//#include <signal.h>
int _getch(void);

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Static Functions, Variables --
///////////////////////////////////////////////////////////////////////////////
static void PrintHeader(TSTRING filename);
static bool util_FileExists(const TSTRING& fileName);

//=========================================================================
// GLOBALS
//=========================================================================

const TCHAR* g_szEightyDashes = _T("--------------------------------------------------------------------------------");

///////////////////////////////////////////////////////////////////////////////
//Insulated implementation for cSiggenCmdLine
struct cSiggen_i
{
    cSiggen_i() : mPrintHex(false), mTerseOutput(false)
    {
    }
    ~cSiggen_i();

    //Data Members:
    bool mPrintHex;
    //If this is true, the signatures will be output in hex rather than Base64
    bool mTerseOutput;
    //If this is true, only the signatures will be printed, and the output will only use one line.

    typedef std::list<std::pair<iSignature*, TSTRING> > ListType;
    ListType                                            mSignatures;
    std::list<TSTRING>                                  mFilesToCheck;
    //A list of the files that need signatures generated for them.
};

//Dtor:
cSiggen_i::~cSiggen_i()
{
    cSiggen_i::ListType::iterator i;
    //Delete all the dynamically allocated signature objects.
    for (i = mSignatures.begin(); i != mSignatures.end(); ++i)
    {
        if (((*i).first) != NULL)
            delete ((*i).first);
    }
}

//#############################################################################
// cSiggenCmdLine
//#############################################################################

cSiggenCmdLine::cSiggenCmdLine()
{
    mpData = new cSiggen_i;
}

cSiggenCmdLine::~cSiggenCmdLine()
{
    delete mpData;
}


///////////////////////////////////////////////////////////////////////////////
// InitCmdLineParser -- Make the command line cognizant of siggen's arguments
///////////////////////////////////////////////////////////////////////////////
void cSiggenCmdLine::InitCmdLineParser(cCmdLineParser& parser)
{

    parser.AddArg(HELP, TSTRING(_T("?")), TSTRING(_T("help")), cCmdLineParser::PARAM_NONE);
    //Signatures:
    parser.AddArg(CRC32, TSTRING(_T("C")), TSTRING(_T("CRC32")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(MD5, TSTRING(_T("M")), TSTRING(_T("MD5")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(SHA, TSTRING(_T("S")), TSTRING(_T("SHA")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(HAVAL, TSTRING(_T("H")), TSTRING(_T("HAVAL")), cCmdLineParser::PARAM_NONE);

    //Output switches
    parser.AddArg(ALL, TSTRING(_T("a")), TSTRING(_T("all")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(HEX, TSTRING(_T("h")), TSTRING(_T("hexadecimal")), cCmdLineParser::PARAM_NONE);
    parser.AddArg(TERSE, TSTRING(_T("t")), TSTRING(_T("terse")), cCmdLineParser::PARAM_NONE);

    //file parameters
    parser.AddArg(PARAMS, TSTRING(_T("")), TSTRING(_T("")), cCmdLineParser::PARAM_MANY);
}

int cSiggenCmdLine::Execute()
{
    cFileArchive arch;
    //archive for reading in files
    TCOUT.flags((TCOUT.flags() & ~std::ios::adjustfield) | std::ios::left);
    //align all output to the left.
    int rtn = 0;
    // return value -- by default, it is set to 0 (OK)

    //Check to see if files have been specified.
    if (mpData->mFilesToCheck.empty())
        return 0;

    //Iterate over file list and generate each signature:
    std::list<TSTRING>::iterator fileIter;
    for (fileIter = mpData->mFilesToCheck.begin(); fileIter != mpData->mFilesToCheck.end(); ++fileIter)
    {
        cDisplayEncoder e;
        TSTRING         displayStr = *fileIter;
        e.Encode(displayStr);

        if (!mpData->mTerseOutput)
            PrintHeader(displayStr);

        if (!util_FileExists(*fileIter))
        {
            TCOUT << TSS_GetString(cSiggen, siggen::STR_ERR_NO_FILE) << _T(": ") << displayStr.c_str() << endl;
            rtn = 1;
            continue;
        }

        //
        // ignore this if it is not a plain file...
        //
        cFSStatArgs fileInfo;
        try
        {
            iFSServices::GetInstance()->Stat(*fileIter, fileInfo);
            if (fileInfo.mFileType != cFSStatArgs::TY_FILE)
            {
                // Not a regular file; warn and skip this file.
                //
                TCOUT << displayStr << TSS_GetString(cSiggen, siggen::STR_SIGGEN_NOT_REG_FILE) << std::endl;
                rtn = 1;
                continue;
            }
        }
        catch (eFSServices& e)
        {
            cErrorReporter::PrintErrorMsg(e);
            rtn = 1;
            continue;
        }

        //Prepare the archive for reading from current file (in loop).
        try
        {
            arch.OpenRead((*fileIter).c_str());
        }
        catch (eArchive&)
        {
            TCOUT << TSS_GetString(cSiggen, siggen::STR_ERR_OPEN_FAILED) << _T(": ") << displayStr.c_str() << endl;
            rtn = 1;
            continue;
        }

        //
        // Iterate over the <signature, signature name> list and add each signature to the
        // sig generator
        //
        cArchiveSigGen                                        asg;
        std::list<std::pair<iSignature*, TSTRING> >::iterator sigIter;
        for (sigIter = mpData->mSignatures.begin(); sigIter != mpData->mSignatures.end(); ++sigIter)
            asg.AddSig((*sigIter).first);

        //
        // calculate signatures
        //
        arch.Seek(0, cBidirArchive::BEGINNING);
        asg.CalculateSignatures(arch);
        arch.Close();

        //
        // Iterate over the <signature, signature name> list and output each signature:
        //
        for (sigIter = mpData->mSignatures.begin(); sigIter != mpData->mSignatures.end(); ++sigIter)
        {
            if (!mpData->mTerseOutput)
                TCOUT.width(20);

            //Set the output string to Hex or Base64, depending on the value of mPrintHex
            TSTRING sigStringOut;
            if (mpData->mPrintHex)
                sigStringOut = ((*sigIter).first)->AsStringHex();
            else
                sigStringOut = ((*sigIter).first)->AsString();

            //Output the signatures, include identifiers and newlines only if mTerseOutput is false.
            if (!mpData->mTerseOutput)
                TCOUT << (*sigIter).second.c_str();

            TCOUT << sigStringOut;

            if (!mpData->mTerseOutput)
                TCOUT << endl;
            else
                TCOUT << _T(" ");
            //Output finished for iteration

        } //end for


        //Seperate lines of signatures (for multiple files) with a newline (if terse output)
        if (mpData->mTerseOutput)
            TCOUT << endl;
    }

    if (!mpData->mTerseOutput)
        TCOUT << g_szEightyDashes << _T("\n");

    return rtn;
}

//Interprets the parsed command line and sets the member variables necessary for correct output.
//See cSiggen_i.
//Returns 0 if no file parameters have been passed, otherwise, returns 1.
int cSiggenCmdLine::Init(cCmdLineParser& parser)
{
    cCmdLineIter iter(parser); //iterator for traversing command line
    iter.SeekBegin();
    int  i          = 0; //loop variable
    bool crc_select = false, md5_select = false, sha_select = false, haval_select = false;
    //boolean locals for dealing with ALL switch. (temp.?) fix -DA
    bool switch_present = false;
    int  ret            = 0; //return value. will be false unless some file is specified.

    for (iter.SeekBegin(); !iter.Done(); iter.Next())
    {
        switch (iter.ArgId())
        {
        case HELP:
        {
            return 0;
        }
        case CRC32:
        {
            crc_select = switch_present = true;
            break;
        }
        case MD5:
        {
            md5_select = switch_present = true;
            break;
        }
        case SHA:
        {
            sha_select = switch_present = true;
            break;
        }
        case HAVAL:
        {
            haval_select = switch_present = true;
            break;
        }
        case ALL:
        {
            crc_select = md5_select = sha_select = haval_select = switch_present = true;
            break;
        }
        case HEX:
        {
            mpData->mPrintHex = true;
            break;
        }
        case TERSE:
        {
            mpData->mTerseOutput = true;
            break;
        }
        case PARAMS:
        {
            ret |= 1;
            for (; i < iter.NumParams(); ++i)
            {
                mpData->mFilesToCheck.push_back(TSTRING(iter.ParamAt(i)));
            }
        }
        default:
            break;
        }
    }

    //Default behavior is to print all signatures if no switch is specified.
    if (!switch_present)
        crc_select = md5_select = sha_select = haval_select = true;

    //Push the signatures and their output identifiers onto the mSignature list:
    if (crc_select)
    {
        iSignature* sig_ptr = new cCRC32Signature;
        TSTRING     str     = TSS_GetString(cFS, fs::STR_PROP_CRC32);
        mpData->mSignatures.push_back(std::pair<iSignature*, TSTRING>(sig_ptr, str));
    }
    if (md5_select)
    {
        iSignature* sig_ptr = new cMD5Signature;
        TSTRING     str     = TSS_GetString(cFS, fs::STR_PROP_MD5);
        mpData->mSignatures.push_back(std::pair<iSignature*, TSTRING>(sig_ptr, str));
    }
    if (sha_select)
    {
        iSignature* sig_ptr = new cSHASignature;
        TSTRING     str     = TSS_GetString(cFS, fs::STR_PROP_SHA);
        mpData->mSignatures.push_back(std::pair<iSignature*, TSTRING>(sig_ptr, str));
    }
    if (haval_select)
    {
        iSignature* sig_ptr = new cHAVALSignature;
        TSTRING     str     = TSS_GetString(cFS, fs::STR_PROP_HAVAL);
        mpData->mSignatures.push_back(std::pair<iSignature*, TSTRING>(sig_ptr, str));
    }

    return ret;
}

//Prints a header for each signature.
void PrintHeader(TSTRING filename)
{
    TCOUT << g_szEightyDashes << _T( "\n" );
    TCOUT << _T("Signatures for file: ") << filename.c_str() << _T("\n\n");
}

bool util_FileExists(const TSTRING& fileName)
{
    return _taccess(fileName.c_str(), F_OK) == 0;
}
