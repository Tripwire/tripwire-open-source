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
// objectpool_t

#include "core/stdcore.h"
#include "core/objectpool.h"
#include "core/debug.h"
#include "twtest/test.h"


// this is the struct we will use for testing purposes
struct cDog
{
	int		i;
	char	c;
	double	d;
	cDog() : i(53), c('f'), d(3.14) { cDebug d("cDog::cDog");	d.TraceDebug("Dog ctor...\n"); }
	~cDog()							{ cDebug d("cDog::~cDog");	d.TraceDebug("Dog dtor...\n"); }
};


void TestObjectPool()
{
    int k, j;
	cDebug d("TestObjectPool");

	// first, just try the growing properties of the pool
	cObjectPoolBase pool(10, 5);
	d.TraceDebug("object size = 10, chunk size = 5\n");
	for(k=0; k < 12; k++)
	{
		d.TraceDebug("Allocating...\n");
		pool.Alloc();
	}
	d.TraceDebug("Removing everything...\n");
	pool.Clear();

	// test the template class
	cObjectPool<cDog> dogPool(3);
	std::list<cDog*> lDog;
	for(j=0; j < 7; j++)
	{
		lDog.push_back(dogPool.New());
	}

	std::list<cDog*>::iterator i;
	for( i = lDog.begin(); i != lDog.end(); i++)
	{
		d.TraceDebug("dog contents: %d %c %lf\n", (*i)->i, (*i)->c, (*i)->d);
		dogPool.Delete(*i);
	}
	lDog.clear();

	// now, do some random insertions and deletions...
	std::vector<void*> vAlloced;
	for(k=0; k < 1000; k++)
	{
		if(rand() % 3 > 0 )
		{
			// alloc
			void* pNew = pool.Alloc();
			d.TraceDebug("Allocating %p\n", pNew);
			vAlloced.push_back(pNew);
		}
		else
		{
			// free
   		        int randval = rand();
			int vsize = vAlloced.size();
                        if (vsize) 
			{
			  int idx = randval % vsize;
			  std::vector<void*>::iterator vi = vAlloced.begin() + idx;
			  void* pGone = *vi;
			  d.TraceDebug("Removing %p\n", pGone);
			  pool.Free(pGone);
			  vAlloced.erase(vi);
                        }
		}
	}
	
	d.TraceDebug("Leaving...\n");
}

