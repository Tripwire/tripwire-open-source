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
///////////////////////////////////////////////////////////////////////////////
// dbexplore.cpp
//

#include "stdtw.h"
#include "dbexplore.h"
#include "fcodatabasefile.h"
#include "dbdatasource.h"
#include "core/debug.h"
#include "core/error.h"
#include "fco/fcopropdisplayer.h"
#include "fco/fcopropvector.h"
#include "fco/fcopropset.h"
#include "fco/fcoprop.h"
#include "fco/genreswitcher.h"
#include "fco/twfactory.h"
#include "fco/fconametranslator.h"

//
// TODO -- make an "associate with new genre" method for the db iter so we don't have to
//      delete and create a new one when we switch genres.
//

static inline bool IsSpace(TCHAR c)
{
    return ((c == _T(' ')) || (c == _T('\t')) || (c == _T('\r')) || (c == _T('\n')) || (c == _T('\0')));
}

static inline bool IsEnd(TCHAR c)
{
    return ((c == _T('\0')));
}


///////////////////////////////////////////////////////////////////////////////
// GetNoun
//      note -- you can only call this once after the verb (it eats the rest
//          of the line!)
///////////////////////////////////////////////////////////////////////////////
static void GetNoun(TSTRING& noun)
{
    static TSTRING prevNoun;
    TCHAR          buf[1024];

    TCIN.getline(buf, 1024);
    //
    // find the end of the noun...
    //
    TCHAR* pStart = buf;
    TCHAR* end    = &buf[1023];
    while (IsSpace(*pStart) && (!IsEnd(*pStart)) && (pStart < end))
        pStart++;

    if (IsEnd(*pStart) || (pStart >= end))
    {
        // no noun!
        noun = _T("");
        return;
    }
    TCHAR* pCur   = pStart;
    bool   bQuote = false;
    if (*pCur == _T('\"'))
    {
        bQuote = true;
        pCur++;
        pStart++;
    }
    while (pCur < end)
    {
        if ((!bQuote) && IsSpace(*pCur))
            break;

        if (*pCur == _T('\"') && bQuote)
            break;

        pCur++;
    }
    noun.assign(pStart, (pCur - pStart));

    if (noun.compare(_T("!$")) == 0)
    {
        noun = prevNoun;
    }
    prevNoun = noun;
}

///////////////////////////////////////////////////////////////////////////////
// SeekTo -- seeks to the named noun; this takes care of interpriting special
//      nouns. If the seek fails, then Done() is true and false is returned.
//      If noun is a special character, then it is altered to what it mapped to.
///////////////////////////////////////////////////////////////////////////////
static bool SeekTo(cDbDataSourceIter* pIter, TSTRING& noun)
{
    if (noun[0] == _T('*'))
    {
        // interprite the rest of the string as a number index to seek to...
        int index = _ttoi(&noun[1]);
        if (index < 0)
            return false;
        pIter->SeekBegin();
        for (int i = 0; ((i < index) && (!pIter->Done())); i++, pIter->Next())
        {
        }
        if (pIter->Done())
            return false;
        noun = pIter->GetShortName();
        return true;
    }
    else
        return (pIter->SeekTo(noun.c_str()));
}

///////////////////////////////////////////////////////////////////////////////
// PrintFCO
///////////////////////////////////////////////////////////////////////////////
static void PrintFCO(const iFCO* pFCO, const iFCOPropDisplayer* pDisplayer)
{
    TCOUT.setf(std::ios::left);

    TCOUT << _T("------- ") << iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay(pFCO->GetName())
          << _T(" -------") << std::endl;
    //
    // iterate over all of the properties
    //
    const iFCOPropSet* pPropSet = pFCO->GetPropSet();
    cFCOPropVector     v        = pPropSet->GetValidVector();
    for (int i = 0; i < pPropSet->GetNumProps(); i++)
    {
        if (v.ContainsItem(i))
        {
            TCOUT << _T("[");
            TCOUT.width(2);
            TCOUT << i << _T("]");
            TCOUT.width(25);
            TCOUT << pPropSet->GetPropName(i);
            TCOUT.width(0);
            TCOUT << pDisplayer->PropAsString(pFCO, i) << std::endl;
        }
    }
    TCOUT << _T("--------------------------------------------") << std::endl;
}

static void SplitString(const TSTRING& str, TCHAR c, std::vector<TSTRING>& vStrings)
{
    vStrings.clear();

    TSTRING::const_iterator i, prev;
    for (prev = i = str.begin(); i != str.end(); ++i)
    {
        if (*i == c)
        {
            vStrings.push_back(TSTRING());
            vStrings.back().assign(prev, i);
            prev = i + 1;
        }
    }
    if (prev != i)
    {
        vStrings.push_back(TSTRING());
        vStrings.back().assign(prev, i);
    }
}


///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////
void cDbExplore::Execute(cFCODatabaseFileIter& dbIter)
{
    ASSERT(!dbIter.Done());

    cDbDataSourceIter*       pIter      = new cDbDataSourceIter(&dbIter.GetDb(), dbIter.GetGenre());
    const iFCOPropDisplayer* pDisplayer = dbIter.GetGenreHeader().GetPropDisplayer();

    ////////////////////////////
    // the main event loop...
    ////////////////////////////
    while (true)
    {
        TSTRING verb, noun;
        TCOUT << _T(">>");
        TCIN >> verb;
        //
        // ok, now we switch on the command...
        //
        //-----------------------------------------------------------------
        // quit
        //-----------------------------------------------------------------
        if (verb.compare(_T("quit")) == 0)
        {
            // the quit command...
            break;
        }
        //-----------------------------------------------------------------
        // print
        //-----------------------------------------------------------------
        if (verb.compare(_T("print")) == 0)
        {
            GetNoun(noun);
            if (SeekTo(pIter, noun))
            {
                if (pIter->HasFCOData())
                {
                    iFCO* pFCO = pIter->CreateFCO();
                    PrintFCO(pFCO, pDisplayer);
                    pFCO->Release();
                }
                else
                {
                    TCOUT << _T("Object has no data associated with it.") << std::endl;
                }
            }
            else
            {
                TCOUT << _T("Unable to find object ") << noun << std::endl;
            }
        }
        //-----------------------------------------------------------------
        // mkdir
        //-----------------------------------------------------------------
        else if (verb.compare(_T("mkdir")) == 0)
        {
            GetNoun(noun);
            TCOUT << "Making a child of " << noun << std::endl;
            if (pIter->SeekTo(noun.c_str()))
            {
                pIter->AddChildArray();
            }
            else
            {
                TCOUT << "Unable to find object " << noun << std::endl;
            }
        }
        //-----------------------------------------------------------------
        // mk
        //-----------------------------------------------------------------
        else if (verb.compare(_T("mk")) == 0)
        {
            GetNoun(noun);
            TCOUT << "Making object " << noun << std::endl;
            if (pIter->SeekTo(noun.c_str()))
            {
                TCOUT << "Error: object already exists!" << std::endl;
            }
            else
            {
                pIter->AddFCO(noun, 0); // add a null fco for now
            }
        }
        //-----------------------------------------------------------------
        // rmdir
        //-----------------------------------------------------------------
        else if (verb.compare(_T("rmdir")) == 0)
        {
            GetNoun(noun);
            TCOUT << "Removing the child of " << noun << std::endl;
            if (pIter->SeekTo(noun.c_str()))
            {
                if (pIter->CanRemoveChildArray())
                {
                    pIter->RemoveChildArray();
                }
                else
                {
                    TCOUT << "Can't delete object; it still has children." << std::endl;
                }
            }
            else
            {
                TCOUT << "Unable to find object " << noun << std::endl;
            }
        }
        //-----------------------------------------------------------------
        // rm
        //-----------------------------------------------------------------
        else if (verb.compare(_T("rm")) == 0)
        {
            GetNoun(noun);
            TCOUT << "Removing object " << noun << std::endl;
            if (pIter->SeekTo(noun.c_str()))
            {
                if (pIter->CanDescend())
                {
                    TCOUT << "Can't delete object; it still has children." << std::endl;
                }
                else
                {
                    pIter->RemoveFCO();
                }
            }
            else
            {
                TCOUT << "Unable to find object " << noun << std::endl;
            }
        }
        else if (verb.compare(_T("rmdata")) == 0)
        {
            GetNoun(noun);
            TCOUT << "Removing object data " << noun << std::endl;
            if (pIter->SeekTo(noun.c_str()))
            {
                if (pIter->CanDescend())
                {
                    TCOUT << "Can't delete object; it still has children." << std::endl;
                }
                else
                {
                    pIter->RemoveFCOData();
                }
            }
            else
            {
                TCOUT << "Unable to find object " << noun << std::endl;
            }
        }
        //-----------------------------------------------------------------
        // pwd
        //-----------------------------------------------------------------
        else if (verb.compare(_T("pwd")) == 0)
        {
            TCOUT << iTWFactory::GetInstance()->GetNameTranslator()->ToStringDisplay(pIter->GetParentName())
                  << std::endl;
        }
        //-----------------------------------------------------------------
        // ls
        //-----------------------------------------------------------------
        else if (verb.compare(_T("ls")) == 0)
        {
            int cnt = 0;
            for (pIter->SeekBegin(); !pIter->Done(); pIter->Next(), cnt++)
            {
                TCOUT << _T("[") << cnt;
                if (pIter->CanDescend())
                {
                    TCOUT << _T("]*\t");
                }
                else
                {
                    TCOUT << _T("]\t");
                }
                TCOUT << pIter->GetShortName() << std::endl;
            }
        }
        //-----------------------------------------------------------------
        // cd
        //-----------------------------------------------------------------
        else if (verb.compare(_T("cd")) == 0)
        {
            GetNoun(noun);
            std::vector<TSTRING> vDirs;
            SplitString(noun, pIter->GetParentName().GetDelimiter(), vDirs);
            for (std::vector<TSTRING>::iterator i = vDirs.begin(); i != vDirs.end(); ++i)
            {

                if (i->compare(_T("..")) == 0)
                {
                    if (pIter->AtRoot())
                    {
                        TCOUT << _T("Can't ascend above root.") << std::endl;
                        break;
                    }
                    else
                    {
                        TCOUT << _T("Ascending...") << std::endl;
                        pIter->Ascend();
                    }
                }
                else
                {
                    if (SeekTo(pIter, *i))
                    {
                        if (pIter->CanDescend())
                        {
                            TCOUT << _T("Descending into ") << *i << std::endl;
                            pIter->Descend();
                        }
                        else
                        {
                            TCOUT << *i << _T(" has no children; can't descend.") << std::endl;
                            break;
                        }
                    }
                    else
                    {
                        TCOUT << _T("Unable to find object ") << *i << std::endl;
                        break;
                    }
                }
            }
        }
        //-----------------------------------------------------------------
        // cg
        //-----------------------------------------------------------------
        else if (verb.compare(_T("cg")) == 0)
        {
            GetNoun(noun);

            cGenre::Genre newGenre = cGenreSwitcher::GetInstance()->StringToGenre(noun.c_str());

            if (newGenre != cGenre::GENRE_INVALID)
            {
                dbIter.SeekToGenre(newGenre);
                if (!dbIter.Done())
                {
                    TCOUT << _T("Changing to Genre ") << noun << std::endl;
                    //
                    // create a new db iter for the new genre (db iters can only be
                    // assocaited with a single genre :-( )
                    //
                    delete pIter;
                    pIter = new cDbDataSourceIter(&dbIter.GetDb(), newGenre);
                }
                else
                {
                    TCOUT << _T("Unable to find Genre ") << noun << std::endl;
                }
            }
            else
            {
                TCOUT << _T("Invalid Genre ") << noun << std::endl;
            }
        }
        //-----------------------------------------------------------------
        // pwg
        //-----------------------------------------------------------------
        else if (verb.compare(_T("pwg")) == 0)
        {
            TCOUT << _T("Current Genre: ")
                  << cGenreSwitcher::GetInstance()->GenreToString((cGenre::Genre)dbIter.GetGenre(), true) << std::endl;
        }

        //-----------------------------------------------------------------
        // help
        //-----------------------------------------------------------------
        if (verb.compare(_T("help")) == 0)
        {
            TCOUT << _T("Commands: ") << std::endl
                  << _T(" cd <dir_name>") << std::endl
                  << _T(" pwd ") << std::endl
                  << _T(" ls ") << std::endl
                  << _T(" print <object_name>") << std::endl
                  << _T(" cg (FS | NTFS | NTREG)") << std::endl
                  << _T(" pwg") << std::endl
                  << _T(" quit") << std::endl;
        }

        // make sure the file is still valid...
        //

#ifdef _BLOCKFILE_DEBUG
        dbIter.GetDb().AssertAllBlocksValid();
#endif
    }

    delete pIter;
    TCOUT << _T("Exiting...") << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
//
// old stuff
//
///////////////////////////////////////////////////////////////////////////////
/*
    
        //-----------------------------------------------------------------
        // mkdir
        //-----------------------------------------------------------------
        else if( verb.compare( _T("mkdir") ) == 0 )
        {
            GetNoun(noun);
            TCOUT << "Making a child of " << noun << std::endl;
            if( iter.SeekTo( noun.c_str() ) )
            {
                iter.AddChildArray();
            }
            else
            {
                TCOUT << "Unable to find object " << noun << std::endl;
            }
        }
        //-----------------------------------------------------------------
        // mk
        //-----------------------------------------------------------------
        else if( verb.compare( _T("mk") ) == 0 )
        {
            GetNoun(noun);
            TCOUT << "Making object " << noun << std::endl;
            if( iter.SeekTo( noun.c_str() ) )
            {
                TCOUT << "Error: object already exists!" << std::endl;
            }
            else
            {
                iter.AddFCO( noun, 0 ); // add a null fco for now
            }
        }
        //-----------------------------------------------------------------
        // rmdir
        //-----------------------------------------------------------------
        // TODO -- still needs to be implemented in the iterator class!
        //
        else if( verb.compare( _T("rmdir") ) == 0 )
        {
            GetNoun(noun);
            TCOUT << "Removing the child of " << noun << std::endl;
            if( iter.SeekTo( noun.c_str() ) )
            {
                //TODO -- check that it has an empty child
                iter.DeleteChildArray();
            }
            else
            {
                TCOUT << "Unable to find object " << noun << std::endl;
            }
        }
        //-----------------------------------------------------------------
        // rm
        //-----------------------------------------------------------------
        else if( verb.compare( _T("rm") ) == 0 )
        {
            GetNoun(noun);
            TCOUT << "Removing object " << noun << std::endl;
            if( iter.SeekTo( noun.c_str() ) )
            {
                if( iter.CanDescend() )
                {
                    TCOUT << "Can't delete object; it still has children." << std::endl;
                }
                else
                {
                    iter.RemoveFCO();
                }
            }
            else
            {
                TCOUT << "Unable to find object " << noun << std::endl;
            }
        }

*/
