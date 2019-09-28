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
// error.h
//
#ifndef __ERROR_H
#define __ERROR_H


//-----------------------------------------------------------------------------
// eError
//-----------------------------------------------------------------------------
class eError
{
public:
    //-------------------------------------------------------------------------
    // Construction and Assignment
    //-------------------------------------------------------------------------
    eError(const TSTRING& msg, uint32_t flags = 0);
    explicit eError(const eError& rhs);
    explicit eError();
    void operator=(const eError& rhs);

    //-------------------------------------------------------------------------
    // Destruction
    //-------------------------------------------------------------------------
    virtual ~eError();

    //-------------------------------------------------------------------------
    // Data Access
    //-------------------------------------------------------------------------
    virtual uint32_t GetID() const = 0;
    // returns a system wide unique identifier for this exception. See the
    // macro below for the typical implementation of this method.
    // This is used to associate the error with a string description of the
    // error via the global error table.

    virtual TSTRING GetMsg() const;
    // returns specific information about the error that occured. Provides
    // additional information about the error described by GetID(). It should
    // not provide any information redundant with GetID().
    //
    // The string passed to the constructor should be formated properly to
    // be displayed as the "Second" part of an error message, or the derived
    // class should override GetMsg() and return a string appropriate for display.

    uint32_t GetFlags() const;
    // Flags are defined below. Currently, these only have an impact on how errors are
    // displayed.

    //-------------------------------------------------------------------------
    // Flags
    //-------------------------------------------------------------------------
    enum Flag
    {
        NON_FATAL         = 0x00000001, // displays "Error" or "Warning" ?
        SUPRESS_THIRD_MSG = 0x00000002  // supresses the "continuing" or "exiting" message
    };

    void SetFlags(uint32_t flags);

    //-------------------------------------------------------------------------
    // Flag Convenience Methods
    //-------------------------------------------------------------------------
    void SetFatality(bool fatal);
    bool IsFatal() const;
    // Fatality is set to true by default when eError is constructed.  But when an error
    // it is put in an cErrorBucket chain it the code doing so may wish to set the fatality
    // to non-fatal to indicate that the error is actually a warning and program flow
    // is going to continue.
    void SetSupressThird(bool supressThird);
    bool SupressThird() const;

    //-------------------------------------------------------------------------
    // Utility Methods
    //-------------------------------------------------------------------------
    static uint32_t CalcHash(const char* name);
    // calculates the CRC32 of the string passed in as name. This methods
    // asserts that name is non null. This is used to generate unique IDs
    // for errors.

    //-------------------------------------------------------------------------
    // Private Implementation
    //-------------------------------------------------------------------------
protected:
    TSTRING   mMsg;
    uint32_t  mFlags;
};

//-----------------------------------------------------------------------------
// Error Related Macros
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// TSS_BEGIN_EXCEPTION / TSS_END_EXCEPTION
//
// Serves the same purpose as TSS_EXCEPTION but allows custom data and methods
//      to be added to the exception class.
///////////////////////////////////////////////////////////////////////////////

// TODO: make use of 'explicit' more portable here, and/or figure out why
// this doesn't work on so many compilers
#if 1 //HAVE_GCC || HAVE_CLANG || HAVE_IBM_XLC || HAVE_IBM_GXLC || HAVE_ORACLE_SUNCC
#    define TSS_BEGIN_EXCEPTION_EXPLICIT
#else
#    define TSS_BEGIN_EXCEPTION_EXPLICIT explicit
#endif

#define TSS_BEGIN_EXCEPTION(except, base)                                  \
    class except : public base                                             \
    {                                                                      \
    public:                                                                \
        except(const TSTRING& msg, uint32_t flags = 0) : base(msg, flags)    \
        {                                                                  \
        }                                                                  \
        TSS_BEGIN_EXCEPTION_EXPLICIT except(const except& rhs) : base(rhs) \
        {                                                                  \
        }                                                                  \
        explicit except() : base()                                         \
        {                                                                  \
        }                                                                  \
                                                                           \
        virtual uint32_t GetID() const                                       \
        {                                                                  \
            return CalcHash(#except);                                      \
        }

#define TSS_END_EXCEPTION() \
    }                       \
    ;

///////////////////////////////////////////////////////////////////////////////
// TSS_BEGIN_EXCEPTION_NO_CTOR
//
// Same as TSS_BEGIN_EXCEPTION, but doesn't define any ctors.
///////////////////////////////////////////////////////////////////////////////
#define TSS_BEGIN_EXCEPTION_NO_CTOR(except, base) \
    class except : public base                    \
    {                                             \
    public:                                       \
        explicit except() : base()                \
        {                                         \
        }                                         \
                                                  \
        virtual uint32_t GetID() const              \
        {                                         \
            return CalcHash(#except);             \
        }

///////////////////////////////////////////////////////////////////////////////
// TSS_EXCEPTION
//
//  This is a convenience define for quickly defining an exception class. After
//  defining a new exception, don't forget to add it to the package's error
//  string file!
//
// TODO (mdb) -- do we want to cache the CRC? if we store it in a class static
//      variable, then we will need to define it in the cpp file as well ...
///////////////////////////////////////////////////////////////////////////////
#define TSS_EXCEPTION(except, base)   \
    TSS_BEGIN_EXCEPTION(except, base) \
    TSS_END_EXCEPTION()

//-----------------------------------------------------------------------------
// Inline Implementation
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// eError
///////////////////////////////////////////////////////////////////////////////
inline eError::eError(const TSTRING& msg, uint32_t flags) : mMsg(msg), mFlags(flags)
{
}

///////////////////////////////////////////////////////////////////////////////
// eError
///////////////////////////////////////////////////////////////////////////////
inline eError::eError(const eError& rhs) : mMsg(rhs.mMsg), mFlags(rhs.mFlags)
{
}

///////////////////////////////////////////////////////////////////////////////
// eError
///////////////////////////////////////////////////////////////////////////////
inline eError::eError() : mMsg(_T("")), mFlags(0)
{
}


///////////////////////////////////////////////////////////////////////////////
// operator=
///////////////////////////////////////////////////////////////////////////////
inline void eError::operator=(const eError& rhs)
{
    mMsg   = rhs.mMsg;
    mFlags = rhs.mFlags;
}

///////////////////////////////////////////////////////////////////////////////
// ~eError
///////////////////////////////////////////////////////////////////////////////
inline eError::~eError()
{
}

///////////////////////////////////////////////////////////////////////////////
// GetMsg
///////////////////////////////////////////////////////////////////////////////
inline TSTRING eError::GetMsg() const
{
    return mMsg;
}

///////////////////////////////////////////////////////////////////////////////
// GetFlags
///////////////////////////////////////////////////////////////////////////////
inline uint32_t eError::GetFlags() const
{
    return mFlags;
}

///////////////////////////////////////////////////////////////////////////////
// SetFlags
///////////////////////////////////////////////////////////////////////////////
inline void eError::SetFlags(uint32_t flags)
{
    mFlags = flags;
}

///////////////////////////////////////////////////////////////////////////////
// SetFatality
///////////////////////////////////////////////////////////////////////////////
inline void eError::SetFatality(bool fatal)
{
    if (fatal)
        mFlags &= ~(uint32_t)NON_FATAL;
    else
        mFlags |= (uint32_t)NON_FATAL;
}

///////////////////////////////////////////////////////////////////////////////
// IsFatal
///////////////////////////////////////////////////////////////////////////////
inline bool eError::IsFatal() const
{
    return (mFlags & (uint32_t)NON_FATAL) == 0;
}

///////////////////////////////////////////////////////////////////////////////
// SetSupressThird
///////////////////////////////////////////////////////////////////////////////
inline void eError::SetSupressThird(bool supressThird)
{
    if (supressThird)
        mFlags |= (uint32_t)SUPRESS_THIRD_MSG;
    else
        mFlags &= ~(uint32_t)SUPRESS_THIRD_MSG;
}

///////////////////////////////////////////////////////////////////////////////
// SupressThird
///////////////////////////////////////////////////////////////////////////////
inline bool eError::SupressThird() const
{
    return (mFlags & (uint32_t)SUPRESS_THIRD_MSG) == 0;
}


#endif //__ERROR_H
