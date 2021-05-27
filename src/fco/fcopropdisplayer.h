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
// fcopropdisplayer.h
//

#ifndef __FCOPROPDISPLAYER_H
#define __FCOPROPDISPLAYER_H

#ifndef __SERIALIZABLE_H
#include "core/serializable.h"
#endif

class iFCO;
class iFCOProp;
class cFCOPropVector;
class cFCOReport;
class cFCOName;


//////////////////////
// General:
//////////////////////
//
// In most cases, iFCOProp::AsString() is simply a 'dump' of the prop's
// internals.  In order to display this property in a meaningful way to the
// user, it was decided that an auxillary class was needed to perform any
// formatting needed for displaying a property.
//
// The premise here is that it is a good idea to separate the data in the
// database from its text representation.  The text representation of an FCO
// property may require information in addition to the property itself.  For
// instance, while file size ( stored as an integer ) can simply be output to
// the screen using the stream operators, something like access time ( also
// stored as an int ) must be sent through ctime() or a similar
// number-to-string function, in order to be meaningful to the user.
//
// This class's job is to encapsulate the knowledge of how to turn the database
// representation of an FCO property into a text representation.  The
// information needed may static, and known at compile time ( which can be as
// simple as knowing which system call to call for a certain property, such as
// ctime() for a time value ), or dynamically acquired information ( such as
// uid, which requires uid-to-username mapping information from the system ).
//
//////////////////////////////
// Major Functionality:
//////////////////////////////
//
// The main interface of this class is the PropAsString function, which
// returns a formatted string representing the property.
//
// Dynamic information is collected throught the InitForFCO or InitForProp,
// InitForProp simply being a subfunction of InitForFCO.
//
// This class also contains functionality to display an FCO and a few
// pre-defined properites.  This function, GetDetails(), was designed to mimic
// a Unix 'ls -l listing' of the properties of the FCO.   GetDetailsHeader()
// prints out the header for the listing.
//
//////////////////////////////
// Minor Functionality:
//////////////////////////////
//
// See implementation comments.
//
//////////////////////////////
// Relationships
//////////////////////////////
//
//////////////////////////////
// Typical Usage:
//////////////////////////////
//
//      (1) At some time after the construction of an FCO, some component of
//          tripwire will call InitForFCO() with the propdisplayer (PD)
//          appropriate for the genre of the FCO.  This retrieves and stores
//          any information needed for displaying the FCO.
//      (2) Later, when a property of the FCO needs to be displayed,
//          the client will call PropAsString().  For any property that the
//          PD does not know how to convert, the PD will simply return the
//          result of the prop's AsString() function.  So it is safe to call
//          PropAsString() for all properties, even those it does not know
//          how to display.
//      (3) Merge() is used to consolidate PD information into one PD.  So
//          when performing a database update, the DB's PD will have to be
//          merged with the report's PD.
//

class iFCOPropDisplayer : public iTypedSerializable
{
public:
    virtual ~iFCOPropDisplayer(){};

    virtual iFCOPropDisplayer* Clone() const = 0;

    virtual void Merge(const iFCOPropDisplayer* const ppd) = 0;
    // adds all prop display info from rhs to this.  it is an error to merge two prop diplayers of different derived types

    virtual const cFCOPropVector& GetPropsConverted() const = 0;
    // returns vector where if a given prop in the vector is valid, then this class
    // can do a PropAsString that's different from the prop's own AsString()

    virtual TSTRING PropAsString(const iFCO* const pFCO, int const propIdx, int iOffset = 0, int iWidth = 0) const = 0;
    virtual TSTRING PropAsString(const iFCO* const pFCO, int const propIdx, int iOffset = 0, int iWidth = 0)       = 0;
    // returns a more readable value than iFCOProp::AsString() (which simply returns a text version
    // of the prop's value), (e.g. instead of calling fsprop(ATIME)::AsString() and getting 4578295721,
    // you can call PropAsString to get "Sun Sep 26 1998" or the like.) If the displayer doesn't
    // know how to display the property, simply returns iFCOProp::AsString()
    // The non-const version will attempt to initialize the property if Lazy is set to true.

    // iOffset and iWidth should be used for multi-line properties;  they only have meaning for lines
    // after the first, so most single-line properties should ignore them.
    // iOffset is the offset from ^ which each new line should start.
    // iWidth is the space that the new line has ( uses setw( iWidth ) ).

    // the TSTRING returned should be convertable to a narrow string (that is, if there's any way that
    // the prop could have funky wide chars, the string is put through cDisplayEncoder )

    virtual bool IsMultiLineProp(int propID) const = 0;
    // returns true if PropAsString can return a string with \n's.

    virtual TSTRING& GetDetails(const iFCO* const pfco, TSTRING& strBuf) const = 0;
    // Returns a string with some of the FCO's genre specific properties. So, the
    // output will look somewhat like the UNIX "ls -l" command except that the FCO name
    // is not output

    virtual TSTRING& GetDetailsHeader(TSTRING& strBuf, int iMargin) const = 0;
    // describes output of GetDetails and is formatted just like it.  To be used as a header.
    // since the header is a two-lined output, it needs a margin for the second line

    virtual void InitForProp(const iFCO* const pFCO, const int propIdx) = 0;
    // InitForProp() is called to initialize the class's data structures to convert the passed
    // property only. This can be used if there is only a small subset of the total system
    // information that will actually be used, saving on memory and disk usage.

    virtual void InitForFCO(const iFCO* const ifco) = 0;
    // calls InitForProp on all valid props in ifco

    virtual void SetLazy(const bool bLazy = true) = 0;
    virtual bool GetLazy() const                  = 0;
    // Get/SetLazy() is used for the lazy evaluation of the class's conversion data; if SetLazy(true) is called,
    // then all PropAsString() calls will have an implicit InitForProp()call, if it is needed, before the conversion.
};


#endif //__FCOPROPDISPLAYER_H
