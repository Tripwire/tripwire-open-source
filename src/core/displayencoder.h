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
// displayencoder.h
//

#ifndef __DISPLAYENCODER_H
#define __DISPLAYENCODER_H

//=========================================================================
// EXCEPTION DECLARATION
//=========================================================================

TSS_EXCEPTION(eEncoder, eError);
TSS_EXCEPTION(eBadDecoderInput, eEncoder);
TSS_EXCEPTION(eBadHexConversion, eEncoder);
TSS_EXCEPTION(eUnknownEscapeEncoding, eEncoder);

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

///////////////////////////////////////////////
// cDisplayEncoder
///////////////////////////////////////////////

class iCharEncoder;

// TODO:BAM -- i don't think that the order of encodings is important
// as long as AllIdentifiersUnique and OnlyOneCatagoryPerChar are true
class cEncoder
{
public:
    enum Encodings
    {
        NON_NARROWABLE = 0x01, // WC -> MB, MB alway narrowable:
        NON_PRINTABLE  = 0x02,
        BACKSLASH      = 0x04,
        DBL_QUOTE      = 0x08
    };

    enum Flags
    {
        ROUNDTRIP        = 0x01,
        NON_ROUNDTRIP    = 0x02,
        ALLOW_WHITESPACE = 0x04
    };

    cEncoder(int e, int f); // mask of Encodings
    virtual ~cEncoder();

    void Encode(TSTRING& str) const;
    void Decode(TSTRING& str) const; // TODO:BAM -- throw error!

    void ValidateSchema() const;
    bool OnlyOneCatagoryPerChar() const;
    bool AllIdentifiersUnique() const;
    bool AllTestsRunOnEncodedString(const TSTRING& str) const;

private:
    typedef std::vector<iCharEncoder*> sack_type;

    TSTRING
    Encode(TSTRING::const_iterator first, TSTRING::const_iterator last, sack_type::const_iterator encoding) const;

    bool RoundTrip() const;
    bool AllowWhiteSpace() const;

    sack_type m_encodings;
    int       m_fFlags;
};

// encodes ( BACKSLASH | DBL_QUOTE | NON_NARROWABLE | NON_PRINTABLE )

class cDisplayEncoder : public cEncoder
{
public:
    cDisplayEncoder(Flags f = NON_ROUNDTRIP);

    void Encode(TSTRING& str) const;
    bool Decode(TSTRING& str) const;

    static TSTRING EncodeInline(const TSTRING& sIn)
    {
        TSTRING         sOut = sIn;
        cDisplayEncoder e;
        e.Encode(sOut);
        return sOut;
    }

    static TSTRING EncodeInlineAllowWS(const TSTRING& sIn)
    {
        TSTRING         sOut = sIn;
        cDisplayEncoder e((Flags)(NON_ROUNDTRIP | ALLOW_WHITESPACE));
        e.Encode(sOut);
        return sOut;
    }
};


class cCharEncoderUtil
{
public:
    static bool IsPrintable(TCHAR ch);
    static bool IsWhiteSpace(TCHAR ch);

    static TSTRING CharStringToHexValue(const TSTRING& str);

    static TSTRING HexValueToCharString(const TSTRING& str);

    static TCHAR hex_to_char(TSTRING::const_iterator first, TSTRING::const_iterator last);

    static TSTRING char_to_hex(TCHAR ch);

    static TSTRING DecodeHexToChar(TSTRING::const_iterator* pcur, const TSTRING::const_iterator end);


    enum
    {
        BYTE_AS_HEX__IN_TCHARS  = 2,
        BYTES_PER_TCHAR         = sizeof(TCHAR),
        TCHAR_AS_HEX__IN_TCHARS = BYTE_AS_HEX__IN_TCHARS * BYTES_PER_TCHAR
    };
};

#endif //__DISPLAYENCODER_H


/*
///////////////////////////////////////////////////////////////////////////////
// displayencoder.h
//

#ifndef __DISPLAYENCODER_H
#define __DISPLAYENCODER_H

//=========================================================================
// DECLARATION OF CLASSES
//=========================================================================

///////////////////////////////////////////////
// cDisplayEncoder
///////////////////////////////////////////////

template< class CharT >
class iCharEncoder_;

// TODO:BAM -- i don't think that the order of encodings is important 
// as long as AllIdentifiersUnique and OnlyOneCatagoryPerChar are true
template< class CharT >
class cEncoder
{
    public:                
        enum Encodings 
        {
            NON_NARROWABLE = 0x01, // WC -> SB, SB alway narrowable: 
            NON_PRINTABLE = 0x02, 
            BACKSLASH = 0x04, 
            DBL_QUOTE = 0x08,
        };
        
        enum Flags
        {
            ROUNDTRIP = 0x01, 
            NON_ROUNDTRIP = 0x02, 
        };

        cEncoder( int e, int f ); // mask of Encodings
        virtual ~cEncoder();

        void Encode( TSTRING& str ) const;
        void Decode( TSTRING& str ) const; // TODO:BAM -- throw error!

        void ValidateSchema() const;
        bool OnlyOneCatagoryPerChar() const;
        bool AllIdentifiersUnique() const;
        bool AllTestsRunOnEncodedString( const TSTRING& str ) const;

    private:
        typedef std::vector< iCharEncoder_<CharT>* > sack_type;

        sack_type   m_encodings;
        bool        m_fRoundTrip;
};


// encodes ( BACKSLASH | DBL_QUOTE | NON_NARROWABLE | NON_PRINTABLE )
template< class CharT >
class cDisplayEncoder_ : public cEncoder< CharT >
{
    public:
        cDisplayEncoder_( Flags f = NON_ROUNDTRIP );
        
        void Encode( TSTRING& str ) const;
        bool Decode( TSTRING& str ) const;
};


///////////////////////////////////////////////
// cTWLocale
///////////////////////////////////////////////
// to make it easier to use....
///////////////////////////////////////////////
typedef cDisplayEncoder_<TCHAR> cDisplayEncoder;


#endif //__DISPLAYENCODER_H

*/
