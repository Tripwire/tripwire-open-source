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
// dbdatasource_t.cpp
#include "tw/stdtw.h"
#include "tw/dbdatasource.h"
#include "db/hierdatabase.h"
#include "core/fsservices.h"
#include "core/debug.h"
#include "core/error.h"
#include "fco/fcopropvector.h"
#include "fco/fcopropset.h"
#include "fco/fcoprop.h"
#include "fco/fco.h"

static void GetNoun( TSTRING& noun )
{
	static TSTRING prevNoun;
	TCIN >> noun;
	if( noun.compare( _T("!$") ) == 0 )
	{
		noun = prevNoun;
	}
	prevNoun = noun;
}

//
// TODO -- implement this with the prop displayer as well!
//
static void PrintFCO( const iFCO* pFCO )
{
	TCOUT.setf(std::ios::left);
	
	TCOUT << "------- " << pFCO->GetName().AsString() << " -------" << std::endl;
	//
	// iterate over all of the properties
	//
	const iFCOPropSet* pPropSet	= pFCO->GetPropSet();
	cFCOPropVector v		= pPropSet->GetValidVector();
	for( int i=0; i < pPropSet->GetNumProps(); i++ )
	{	
		if( v.ContainsItem( i ) )
		{
			TCOUT << "[";
			TCOUT.width(2);
			TCOUT << i << "]" ;
			TCOUT.width(25);
			TCOUT << pPropSet->GetPropName(i);
			TCOUT.width(0);
			TCOUT << pPropSet->GetPropAt( i )->AsString() << std::endl;
		}
	}
	TCOUT << "--------------------------------------------" << std::endl;
}


void TestDbDataSource()
{
	cDebug d("TestDbDataSource");
	cHierDatabase db;

	const TSTRING dbName = _T("c:/tmp/tw.db");

	try
	{
		// TODO -- get the case sensitiveness  and delimiting char out of the factory instead of iFSServices
		//
		TCOUT << _T("Opening database ") << dbName << std::endl;
		db.Open( dbName, 5, false );
		cDbDataSourceIter iter( &db );

		////////////////////////////
		// the main event loop...
		////////////////////////////
		while( true )
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
			if( verb.compare( _T("quit") ) == 0 )
			{
				// the quit command...
				break;
			}
			//-----------------------------------------------------------------
			// print
			//-----------------------------------------------------------------
			if( verb.compare( _T("print") ) == 0 )
			{
				GetNoun(noun);
				if( iter.SeekTo( noun.c_str() ) )
				{
					if( iter.HasFCOData() )
					{
						iFCO* pFCO = iter.CreateFCO();
						PrintFCO( pFCO );
						pFCO->Release();
					}
					else
					{
						TCOUT << "Object has no data associated with it." << std::endl;
					}
				}
				else
				{
					TCOUT << "Unable to find object " << noun << std::endl;
				}
			}
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
					iter.AddFCO( noun, 0 );	// add a null fco for now
				}
			}
			//-----------------------------------------------------------------
			// rmdir
			//-----------------------------------------------------------------
			// TODO -- still needs to be implemented in the iterator class!
			//
			/*
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
			*/
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
			//-----------------------------------------------------------------
			// pwd
			//-----------------------------------------------------------------
			else if( verb.compare( _T("pwd") ) == 0 )
			{
				TCOUT << iter.GetParentName().AsString() << std::endl;
			}
			//-----------------------------------------------------------------
			// ls
			//-----------------------------------------------------------------
			else if( verb.compare( _T("ls") ) == 0 )
			{
				int cnt = 0;
				for( iter.SeekBegin(); ! iter.Done(); iter.Next(), cnt++ )
				{
					TCOUT << "[" << cnt ;
					if( iter.CanDescend() )
					{
						TCOUT << "]*\t" ;
					}
					else
					{
						TCOUT << "]\t" ;
					}
					TCOUT << iter.GetShortName() << std::endl;
				}
			}
			//-----------------------------------------------------------------
			// cd
			//-----------------------------------------------------------------
			else if( verb.compare( _T("cd") ) == 0 )
			{
				GetNoun(noun);
				if( noun.compare( _T("..") ) == 0 )
				{
					if( iter.AtRoot() )
					{
						TCOUT << "Can't ascend above root." << std::endl;
					}
					else
					{
						TCOUT << "Ascending..." << std::endl;
						iter.Ascend();
					}
				}
				else
				{
					if( iter.SeekTo( noun.c_str() ) )
					{
						if( iter.CanDescend() )
						{
							TCOUT << "Descending into " << noun << std::endl;
							iter.Descend();
						}
						else
						{
							TCOUT << noun << " has no children; can't descend." << std::endl;
						}
					}
					else
					{
						TCOUT << "Unable to find object " << noun << std::endl;
					}
				}
			}

			// make sure the file is still valid...
			//
#ifdef _BLOCKFILE_DEBUG
			db.AssertAllBlocksValid() ;
#endif
		}

		TCOUT << "Exiting..." << std::endl;


	}
	catch( eError& e )
	{
		d.TraceError("*** Caught error: %d %s\n", e.GetID(), e.GetMsg().c_str() );
	}
}
