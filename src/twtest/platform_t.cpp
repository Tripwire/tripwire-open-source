//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2021 Tripwire,
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
// platform_t.cpp
//
// test some platform assumptions

#include "core/stdcore.h"
#include "core/platform.h"
#include "twtest/test.h"
#include "core/error.h"

#if HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

using namespace std;

/////////////////////////////////////////////////////////
// UTIL FUNCTION PROTOTYPES
/////////////////////////////////////////////////////////
void TestAlignment();
void TestSizes();

/////////////////////////////////////////////////////////
// TEMPLATIZED UTIL FUNCTION PROTOTYPES
/////////////////////////////////////////////////////////
template<class E, class T> bool CanBeRepresentedAs(E e, T t);

/////////////////////////////////////////////////////////
// TEMPLATIZED UTIL CLASSES DECLARATIONS
/////////////////////////////////////////////////////////

// Constructing this class will write to a memory location
// offset by ALIGN_SIZE.  If it chokes, you'll get a bus error
template<class T, int ALIGN_SIZE> class AlignMe
{
public:
    AlignMe();

private:
    uint8_t a[sizeof(T) + ALIGN_SIZE + 1]; // we want to be able to access a T at address [ALIGN_SIZE]
};

/////////////////////////////////////////////////////////
// TEMPLATIZED UTIL CLASSES IMPLEMENTATIONS
/////////////////////////////////////////////////////////

template<class T, int ALIGN_SIZE> AlignMe<T, ALIGN_SIZE>::AlignMe()
{
// HP-UX does not play your silly alignment games, at least unless you
// first invoke something called "allow_unaligned_data_access()", which
// apparently incurs a substantial perf penalty. Luckily we don't appear
// to have any need for that behavior, which begs the question of why
// this test exists in the first place.
//  -bcox
//Turns out Solaris SPARC is unhappy with this test too, btw
#if ENABLE_ALIGNMENT_TEST
    //TCOUT << _T("Testing alignment of size ") << ALIGN_SIZE << std::endl;

    // access a value in the byte array to see if it is aligned.  if it isn't and the CPU
    // can't handle it, you'll get a bus error

    // this should choke if the CPU can't
    // handle misaligned memory access
    memset(a, 0, sizeof(T) + ALIGN_SIZE + 1);
  
    T* valuePtr = (T*)&a[ALIGN_SIZE];
    TEST(*valuePtr == 0);
    
    T value = *valuePtr; // access memory for read
    TEST(value == 0);
    
    *valuePtr = value; // access memory for write
    TEST(*valuePtr == 0);
#endif
}


template<class T>
void testAlignmentForType()
{
    AlignMe<T, 128> a128;
    AlignMe<T, 64>  a64;
    AlignMe<T, 32>  a32;
    AlignMe<T, 16>  a16;
    AlignMe<T, 8>   a8;
    AlignMe<T, 4>   a4;
    AlignMe<T, 2>   a2;
    AlignMe<T, 1>   a1;
}

/////////////////////////////////////////////////////////
// UTIL FUNCTIONS
/////////////////////////////////////////////////////////

void TestAlignment()
{
    // - - - - - - - - - - - - - - - - - - - - - -
    // empirically determine byte alignment -- see AlignMe definition
    // The last AlignMe to be successfully constructed
    // - - - - - - - - - - - - - - - - - - - - - -

    //    TCOUT << _T("Testing for byte alignment\n")
    //          << _T("=========================================\n");
    testAlignmentForType<int64_t>();
    testAlignmentForType<int32_t>();
    testAlignmentForType<int16_t>();

    testAlignmentForType<float>();
    testAlignmentForType<double>();
    testAlignmentForType<long double>();
}


void TestAlignment2()
{
    // - - - - - - - - - - - - - - - - - - - - - -
    // test a misaligned memory access -- if this
    // chokes, your CPU can't handle such accesses
    // (if it works, your CPU may still not handle
    // such accesses: it may have handled the
    // hardware interrupt that might have occured.
    // - - - - - - - - - - - - - - - - - - - - - -
    uint8_t a[sizeof(int32_t) + sizeof(uint8_t)];

    // this should be fine
    a[0] = 0xAB;


    /*    TCOUT << _T("=========================================\n")
          << _T("About to test memory access off by 1 byte\n") 
          << _T("If you do not see a confirmation after this line, the test choked") 
          << std::endl; */

    // this should choke if the CPU can't
    // handle misaligned memory access
    int32_t* pi = (int32_t*)&a[1];
    *pi       = *pi; // misaligned access (read and write)

    TCOUT << _T("Misaligned access OK.") << std::endl;
    TEST("Misaligned ok"); //again, the test is not exploding up above

    // - - - - - - - - - - - - - - - - - - - - - -
    // make sure our BYTE_ALIGN value is correct --
    // OK, if the above test failed, then comment it out and try this one.
    // if it works, then our BYTE_ALIGN value is large enough
    // - - - - - - - - - - - - - - - - - - - - - -

    uint8_t b[2 * sizeof(BYTE_ALIGN)];

    // this should be fine
    b[0] = 0xAB;

    /*    TCOUT << _T("=========================================\n")
          << _T("About to test memory access off by ") << BYTE_ALIGN 
          << _T(" ") << ( BYTE_ALIGN == 1 ? _T("byte") : _T("bytes") ) << std::endl
          << _T("If you do not see a confirmation after this line, the test choked") 
          << std::endl;
*/
    // this should choke if the CPU can't
    // handle misaligned memory access
    pi  = (int32_t*)&b[BYTE_ALIGN];
    *pi = *pi; // aligned (hopefully) access (read and write)

    /*    TCOUT << _T("Aligned access OK.  BYTE_ALIGN value of ") << BYTE_ALIGN << _T(" is good.") << std::endl;
    TCOUT << _T("=========================================\n");
    TEST("BYTE_ALIGN ok"); // yet again, the test is not falling over a couple of lines up. */
}

#if !ARCHAIC_STL
// Not sure this is a super valuable test, since it just verifies that builtin integer types
// work the way we think they do.
void TestSizes()
{
    TEST(CanBeRepresentedAs(int8_t(),   int8_t()));
    TEST(!CanBeRepresentedAs(int8_t(),  uint8_t()));
    TEST(!CanBeRepresentedAs(uint8_t(), int8_t()));
    TEST(CanBeRepresentedAs(uint8_t(),  uint8_t()));

    TEST(CanBeRepresentedAs(int8_t(),   int16_t()));
    TEST(CanBeRepresentedAs(int16_t(),  int32_t()));
    TEST(CanBeRepresentedAs(int32_t(),  int64_t()));

    TEST(CanBeRepresentedAs(uint8_t(),  uint16_t()));
    TEST(CanBeRepresentedAs(uint16_t(), uint32_t()));
    TEST(CanBeRepresentedAs(uint32_t(), uint64_t()));
}

/////////////////////////////////////////////////////////
// TEMPLATIZED UTIL FUNCTION IMPEMENTATIONS
/////////////////////////////////////////////////////////

template<class E, class T> bool CanBeRepresentedAs(E e, T t)
{
    ASSERT(std::numeric_limits<E>::is_specialized);
    ASSERT(std::numeric_limits<T>::is_specialized);

    bool fReturn = true;

    fReturn &= (std::numeric_limits<E>::min() >= std::numeric_limits<T>::min());
    fReturn &= (std::numeric_limits<E>::max() <= std::numeric_limits<T>::max());

    return fReturn;
}
#endif

////////////////////////////
void RegisterSuite_Platform()
{
    RegisterTest("Platform", "Alignment",  TestAlignment);
    RegisterTest("Platform", "Alignment2", TestAlignment2);
#if !ARCHAIC_STL    
    RegisterTest("Platform", "Sizes",      TestSizes);
#endif    
}
