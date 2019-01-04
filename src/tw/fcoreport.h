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
// fcoreport.h
//
// class cFCOReport -- the tripwire report class

#ifndef __FCOREPORT_H
#define __FCOREPORT_H

#ifndef __FCOGENRE_H
#include "fco/fcogenre.h"
#endif

#ifndef __SERIALIZABLE_H
#include "core/serializable.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// class cFCOReport -- The information from an integrity check.
//
// This class must encompass all the information from an integrity check:
//
//  * It must report each added, removed or changed FCO.
//
//  * It must contain enough information to determine what line the in the policy
//    file caused each FCO change to be reported.
//
//  * It must contain enough information to update the database when the user
//    OK's a FCO change.
//
//  To store all of this information we organize by the specifier list that
//  was used to create the report.  During integrity checking each cFCOSpec
//  will generate an cFCOSet of added FCO's, an cFCOSet of removed FCO's,
//  and a pair of cFCOSet's for each changed FCO (a cFCOSet for the old
//  properties of the FCO's and a cFCOSet for new properties of the FCO).
//  There will also be a cPropertyVector to specify what properties differ
//  between the old and new FCO for the changed set.
//
//  The information in the report will be organized by cFCOSpec.  For each
//  cFCOSpec there will be a corresponding added FCO set, removed FCO set,
//  and for changed fcos a new and old cFCOset and a changed mask.
///////////////////////////////////////////////////////////////////////////////

class cFCOName;
class cFCOReport_i;
class cFCOReportGenreIter_i;
class cFCOReportSpecIter;
class cFCOReportSpecIter_i;
class cFCOReportChangeIter_i;
class iFCOSpec;
class iFCOSet;
class iFCO;
class cFCOPropVector;
class cFCOSpecAttr;
class cFCOReportUtil;
class cFileHeaderID;
class cErrorQueue;
class cFCOReportGenreHeader;
class cFCOReportSpecIter;

class cFCOReport : public iTypedSerializable
{
    DECLARE_TYPEDSERIALIZABLE()
    friend class cFCOReportSpecIter;
    friend class cFCOReportUtil;
    friend class cFCOReportGenreIter;

public:
    cFCOReport();
    virtual ~cFCOReport();

    void ClearReport();

    void
    AddSpec(cGenre::Genre genre, const iFCOSpec* pSpec, const cFCOSpecAttr* pAttr, cFCOReportSpecIter* pIter = NULL);
    // Add a spec to the report.  The report will save copies of all three of these
    // objects in its internal data structure.
    // If pSpec, pAttr, and pIter are all NULL, an empty genre is added to report
    // if pIter is passed in, it will be updated to point at the newly added spec.
    // if genre is not in this report, it will be added
    // if this spec overlaps another spec, an eInternal will be thrown

    void AddChangedFCO(const cFCOReportSpecIter& iter,
                       const iFCO*               pOldFCO,
                       const iFCO*               pNewFCO,
                       const cFCOPropVector&     changedProps);
    // For the spec pointed to by the iter, add a changed FCO pair and property vector.

    const cErrorQueue* GetErrorQueue() const;
    cErrorQueue*       GetErrorQueue();
    // returns a pointer to the report's error queue. The caller is free to iterate over the errors, remove all the
    // errors, or chain an error bucket to or from the report's error queue.

    int GetNumSpecs(cGenre::Genre genre) const;
    // returns the number of specs in the report; these can be iterated over using the iterators below.
    // returns 0 if genre is not in report

    void TraceContents(int dl = -1) const;

    // iSerializable interface
    virtual void Read(iSerializer* pSerializer, int32_t version = 0); // throw (eSerializer, eArchive)
    virtual void Write(iSerializer* pSerializer) const;             // throw (eSerializer, eArchive)

    static const cFileHeaderID& GetFileHeaderID();
    // Return a cFileHeaderID for all databases

protected:
    cFCOReport_i* mpData;
};

///////////////////////////////////////////////////////////////////////////////
// iterator classes -- iterates over all the specs in the report and returns
//      refrences to all the spec's related data
///////////////////////////////////////////////////////////////////////////////
class cFCOReportGenreIter
{
    friend class cFCOReport;
    friend class cFCOReportSpecIter;
    friend class cFCOReportChangeIter;

public:
    explicit cFCOReportGenreIter(const cFCOReport& report);
    cFCOReportGenreIter(const cFCOReportGenreIter& rhs);
    ~cFCOReportGenreIter();

    cFCOReportGenreIter& operator=(const cFCOReportGenreIter& rhs);

    // iteration methods
    void SeekBegin() const;
    void Next() const;
    bool Done() const;

    bool SeekToGenre(cGenre::Genre genre);
    // seeks to specific genre.  Returns false if genre does not exist in report

    void Remove();
    // removes the entire genre from report

    // access to data
    cGenre::Genre GetGenre() const;
    // Get current genre
    cFCOReportGenreHeader& GetGenreHeader() const;

private:
    cFCOReportGenreIter_i* mpData;
};

class cFCOReportSpecIter
{
    friend class cFCOReport;
    friend class cFCOReportChangeIter;

public:
    cFCOReportSpecIter(const cFCOReport& report, cGenre::Genre genre);
    // if genre does not exist in the report, an eInternal error will be thrown.
    cFCOReportSpecIter(cFCOReport& report, cGenre::Genre genre);
    // if genre does not exist in the report, it will be added to the report with
    // and empty spec list.
    explicit cFCOReportSpecIter(const cFCOReportGenreIter& genreIter);
    // thorws eInternal if genreIter is not at a valid genre or is Done()

    cFCOReportSpecIter(const cFCOReportSpecIter& rhs);
    ~cFCOReportSpecIter();

    cFCOReportSpecIter& operator=(const cFCOReportSpecIter& rhs);

    // iteration methods
    void SeekBegin() const;
    void Next() const;
    bool Done() const;

    bool SeekToSpec(const iFCOSpec* pSpec);
    // seeks to the first spec that equals the passed in one; returns false
    // and seeks to end if not found.

    void Remove();
    // removes the spec and all fcos associated wit hit. Behavior is undefined if (Done() == true).
    // after the erase, the iterator points to the next element in the list

    // access to data
    const iFCOSpec*     GetSpec() const;
    const cFCOSpecAttr* GetAttr() const;
    const cErrorQueue*  GetErrorQueue() const;
    cErrorQueue*        GetErrorQueue();
    const iFCOSet*      GetAddedSet() const;
    iFCOSet*            GetAddedSet();
    const iFCOSet*      GetRemovedSet() const;
    iFCOSet*            GetRemovedSet();

    int GetNumChanged() const;
    // returns the number of fcos in the changed list.
    // you access the members of the list by using the iterator below...

    void SetObjectsScanned(int nObjectsScanned);
    int  GetObjectsScanned() const;

private:
    cFCOReportSpecIter_i* mpData;
    // TODO -- if it turns out that we are creating a lot of these, we should consider
    // making a pool of them.
};

class cFCOReportChangeIter
{
public:
    explicit cFCOReportChangeIter(const cFCOReportSpecIter& specIter);
    cFCOReportChangeIter(const cFCOReportChangeIter& rhs);
    ~cFCOReportChangeIter();

    cFCOReportChangeIter& operator=(const cFCOReportChangeIter& rhs);
    void                  SetSpecIter(const cFCOReportSpecIter& specIter);
    // assocaite this iterator with a different spec. We will assert
    // that specIter.Done() is not true.

    void Remove();
    // removes the the change entry pointed at by this iter. Behavior is undefined if (Done() == true).
    // after the erase, the iterator points to the next element in the list

    // iteration methods
    void SeekBegin() const;
    void Next() const;
    bool Done() const;

    // access to data...
    const iFCO*           GetOld() const;
    const iFCO*           GetNew() const;
    const cFCOPropVector& GetChangeVector() const;

private:
    cFCOReportChangeIter()
    {
    }

    cFCOReportChangeIter_i* mpData;
    // TODO -- if it turns out that we are creating a lot of these, we should consider
    // making a pool of them.
};

#endif //__FCOREPORT_H
