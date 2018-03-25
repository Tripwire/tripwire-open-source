//
// The developer of the original code and/or files is Tripwire, Inc.
// Portions created by Tripwire, Inc. are copyright (C) 2000-2018 Tripwire,
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
template<int ALIGN_SIZE> class AlignMe
{
public:
    AlignMe();

private:
    byte a[sizeof(int64) + ALIGN_SIZE]; // we want to be able to access a int64 at address [ALIGN_SIZE]
};

/////////////////////////////////////////////////////////
// TEMPLATIZED UTIL CLASSES IMPLEMENTATIONS
/////////////////////////////////////////////////////////

template<int ALIGN_SIZE> AlignMe<ALIGN_SIZE>::AlignMe()
{
// HP-UX does not play your silly alignment games, at least unless you
// first invoke something called "allow_unaligned_data_access()", which
// apparently incurs a substantial perf penalty. Luckily we don't appear
// to have any need for that behavior, which begs the question of why
// this test exists in the first place.
//  -bcox
#if (!IS_HPUX && !IS_SOLARIS) //Turns out Solaris SPARC is unhappy with this test too, btw
    //TCOUT << _T("Testing alignment of size ") << ALIGN_SIZE << std::endl;

    // access a double in the byte array to see if it is aligned.  if it isn't and the CPU
    // can't handle it, you'll get a bus error

    // this should choke if the CPU can't
    // handle misaligned memory access
    int32* pi = (int32*)&a[ALIGN_SIZE];
    //TCOUT << _T("Testing alignment of an int32...") << std::endl;
    //TCOUT << _T("Reading...") << std::endl;
    int32 i = *pi; // access memory for read
    //TCOUT << _T("Read succeeded.") << std::endl;
    //TCOUT << _T("Writing...") << std::endl;
    *pi = i; // access memory for write
    TCOUT << _T("Write succeeded.") << std::endl;


    // this should choke if the CPU can't
    // handle misaligned memory access
    int64* pb = (int64*)&a[ALIGN_SIZE];
    //TCOUT << _T("Testing alignment of an int64...") << std::endl;
    //TCOUT << _T("Reading...") << std::endl;
    int64 I = *pb; // access memory for read
    //TCOUT << _T("Read succeeded") << std::endl;
    //TCOUT << _T("Writing...") << std::endl;
    *pb = I; // access memory for write
    //TCOUT << _T("Write succeeded.") << std::endl;


    /*TCOUT << _T("Alignment of ") << ALIGN_SIZE << _T(" ") << ( ALIGN_SIZE == 1 ? _T("byte") : _T("bytes") ) << _T(" is OK") << std::endl
          << _T("=========================================\n"); */

    TEST("Aligned"); // The actual test is not bus erroring up above; this just tells the framework we tested something.

#endif
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

    AlignMe<128> a128;
    AlignMe<64>  a64;
    AlignMe<32>  a32;
    AlignMe<16>  a16;
    AlignMe<8>   a8;
    AlignMe<4>   a4;
    AlignMe<2>   a2;
    AlignMe<1>   a1;

    // - - - - - - - - - - - - - - - - - - - - - -
    // test a misaligned memory access -- if this
    // chokes, your CPU can't handle such accesses
    // (if it works, your CPU may still not handle
    // such accesses: it may have handled the
    // hardware interrupt that might have occured.
    // - - - - - - - - - - - - - - - - - - - - - -
    byte a[sizeof(int32) + sizeof(byte)];

    // this should be fine
    a[0] = 0xAB;


    /*    TCOUT << _T("=========================================\n")
          << _T("About to test memory access off by 1 byte\n") 
          << _T("If you do not see a confirmation after this line, the test choked") 
          << std::endl; */

    // this should choke if the CPU can't
    // handle misaligned memory access
    int32* pi = (int32*)&a[1];
    *pi       = *pi; // misaligned access (read and write)

    TCOUT << _T("Misaligned access OK.") << std::endl;
    TEST("Misaligned ok"); //again, the test is not exploding up above

    // - - - - - - - - - - - - - - - - - - - - - -
    // make sure our BYTE_ALIGN value is correct --
    // OK, if the above test failed, then comment it out and try this one.
    // if it works, then our BYTE_ALIGN value is large enough
    // - - - - - - - - - - - - - - - - - - - - - -

    byte b[2 * sizeof(BYTE_ALIGN)];

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
    pi  = (int32*)&b[BYTE_ALIGN];
    *pi = *pi; // aligned (hopefully) access (read and write)

    /*    TCOUT << _T("Aligned access OK.  BYTE_ALIGN value of ") << BYTE_ALIGN << _T(" is good.") << std::endl;
    TCOUT << _T("=========================================\n");
    TEST("BYTE_ALIGN ok"); // yet again, the test is not falling over a couple of lines up. */
}

// Not sure this is a super valuable test, since it just verifies that builtin integer types
// work the way we think they do.
void TestSizes()
{
    TEST(CanBeRepresentedAs(int8(), int8()));
    TEST(!CanBeRepresentedAs(int8(), uint8()));
    TEST(!CanBeRepresentedAs(uint8(), int8()));
    TEST(CanBeRepresentedAs(uint8(), uint8()));

    TEST(CanBeRepresentedAs(int8(), int16()));
    TEST(CanBeRepresentedAs(int16(), int32()));
    TEST(CanBeRepresentedAs(int32(), int64()));

    TEST(CanBeRepresentedAs(uint8(), uint16()));
    TEST(CanBeRepresentedAs(uint16(), uint32()));
    TEST(CanBeRepresentedAs(uint32(), uint64()));
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


////////////////////////////

#if IS_LINUX
const TSTRING expected_os("Linux");
#elif IS_DARWIN
const TSTRING expected_os("Darwin");
#elif IS_CYGWIN
const TSTRING expected_os("Cygwin");
#elif IS_DOS_DJGPP
const TSTRING expected_os("FreeDOS"); // This will likely fail for other DOS flavors
#elif IS_ANDROID
const TSTRING expected_os("Android");
#elif IS_DRAGONFLYBSD
const TSTRING expected_os("DragonFly");
#elif IS_MIDNIGHTBSD
const TSTRING expected_os("MidnightBSD");
#elif IS_FREEBSD
const TSTRING expected_os("FreeBSD");
#elif IS_NETBSD
const TSTRING expected_os("NetBSD");
#elif IS_MIRBSD
const TSTRING expected_os("MirBSD");
#elif IS_BITRIG
const TSTRING expected_os("Bitrig");
#elif IS_LIBERTYBSD
const TSTRING expected_os("LibertyBSD");
#elif IS_OPENBSD
const TSTRING expected_os("OpenBSD");
#elif IS_SOLARIS
const TSTRING expected_os("SunOS");
#elif IS_AIX
const TSTRING expected_os("AIX");
#elif IS_HPUX
const TSTRING expected_os("HP-UX");
#elif IS_IRIX
const TSTRING expected_os("IRIX");
#elif IS_OSF1
const TSTRING expected_os("Tru64");
#elif IS_MINIX
const TSTRING expected_os("Minix");
#elif IS_HURD
const TSTRING expected_os("GNU");
#elif IS_HAIKU
const TSTRING expected_os("Haiku");
#elif IS_SYLLABLE
const TSTRING expected_os("Syllable");
#elif IS_SKYOS
const TSTRING expected_os("SkyOS");
#elif IS_SORTIX
const TSTRING expected_os("Sortix");
#elif IS_MINT
const TSTRING expected_os("MiNT");
#elif IS_AROS
const TSTRING expected_os("AROS");
#elif IS_RTEMS
const TSTRING expected_os("RTEMS");
#elif IS_RISCOS
const TSTRING expected_os("RISC OS");
#elif IS_RISCOS
const TSTRING expected_os("Redox");
#elif IS_QNX
const TSTRING expected_os("QNX");
#else
const TSTRING expected_os("?!?!?");
#endif

void TestPlatformDetection()
{
#if HAVE_SYS_UTSNAME_H
    struct utsname os_info;
    TEST(uname(&os_info) == 0);

    TSTRING observed_os(os_info.sysname);

    if (observed_os != expected_os)
        TCERR << "Expected OS: " << expected_os << " | Observed OS: " << observed_os << std::endl;

    TEST(observed_os == expected_os);
#endif
}

void RegisterSuite_Platform()
{
    RegisterTest("Platform", "Alignment", TestAlignment);
    RegisterTest("Platform", "Sizes", TestSizes);
    RegisterTest("Platform", "PlatformDetection", TestPlatformDetection);
}
