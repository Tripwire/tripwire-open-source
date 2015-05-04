# Microsoft Developer Studio Project File - Name="cryptlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=cryptlib - Win32 Unicode Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cryptlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cryptlib.mak" CFG="cryptlib - Win32 Unicode Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cryptlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "cryptlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "cryptlib - Win32 Unicode Release MinDependency" (based on "Win32 (x86) Static Library")
!MESSAGE "cryptlib - Win32 Unicode Release" (based on "Win32 (x86) Static Library")
!MESSAGE "cryptlib - Win32 Unicode Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Code/Tripwire/2.0/ZZZ Consolidation Test/crypto", BAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cryptlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "cryptlib"
# PROP BASE Intermediate_Dir "cryptlib"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "lib_r"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "_LITTLE_ENDIAN" /Yu"pch.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "cryptlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "cryptli0"
# PROP BASE Intermediate_Dir "cryptli0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "lib_d"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "_LITTLE_ENDIAN" /Yu"pch.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "cryptlib - Win32 Unicode Release MinDependency"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "cryptlib"
# PROP BASE Intermediate_Dir "cryptlib"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../lib_r"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /Yu"pch.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "_LITTLE_ENDIAN" /Yu"pch.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "cryptlib - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "cryptlib___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "cryptlib___Win32_Unicode_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "cryptlib_Unicode_Release"
# PROP Intermediate_Dir "cryptlib_Unicode_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "_LITTLE_ENDIAN" /Yu"pch.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "_LITTLE_ENDIAN" /D "_UNICODE" /Yu"pch.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "cryptlib - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "cryptlib___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "cryptlib___Win32_Unicode_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "cryptlib_Unicode_Debug"
# PROP Intermediate_Dir "cryptlib_Unicode_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "_LITTLE_ENDIAN" /Yu"pch.h" /FD /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "_LITTLE_ENDIAN" /D "_UNICODE" /Yu"pch.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "cryptlib - Win32 Release"
# Name "cryptlib - Win32 Debug"
# Name "cryptlib - Win32 Unicode Release MinDependency"
# Name "cryptlib - Win32 Unicode Release"
# Name "cryptlib - Win32 Unicode Debug"
# Begin Source File

SOURCE=.\3way.cpp
# End Source File
# Begin Source File

SOURCE=.\3way.h
# End Source File
# Begin Source File

SOURCE=.\algebra.cpp
# End Source File
# Begin Source File

SOURCE=.\algebra.h
# End Source File
# Begin Source File

SOURCE=.\asn.cpp
# End Source File
# Begin Source File

SOURCE=.\asn.h
# End Source File
# Begin Source File

SOURCE=.\base64.cpp
# End Source File
# Begin Source File

SOURCE=.\base64.h
# End Source File
# Begin Source File

SOURCE=.\bfinit.cpp
# End Source File
# Begin Source File

SOURCE=.\blowfish.cpp
# End Source File
# Begin Source File

SOURCE=.\blowfish.h
# End Source File
# Begin Source File

SOURCE=.\blumgold.cpp
# End Source File
# Begin Source File

SOURCE=.\blumgold.h
# End Source File
# Begin Source File

SOURCE=.\blumshub.cpp
# End Source File
# Begin Source File

SOURCE=.\blumshub.h
# End Source File
# Begin Source File

SOURCE=.\cast.cpp
# End Source File
# Begin Source File

SOURCE=.\cast.h
# End Source File
# Begin Source File

SOURCE=.\cast128s.cpp
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\crc.cpp
# End Source File
# Begin Source File

SOURCE=.\crc.h
# End Source File
# Begin Source File

SOURCE=.\cryptlib.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptlib.h
# End Source File
# Begin Source File

SOURCE=.\default.cpp
# End Source File
# Begin Source File

SOURCE=.\default.h
# End Source File
# Begin Source File

SOURCE=.\des.cpp
# End Source File
# Begin Source File

SOURCE=.\des.h
# End Source File
# Begin Source File

SOURCE=.\dessp.cpp
# End Source File
# Begin Source File

SOURCE=.\dh.cpp
# End Source File
# Begin Source File

SOURCE=.\dh.h
# End Source File
# Begin Source File

SOURCE=.\diamond.cpp
# End Source File
# Begin Source File

SOURCE=.\diamond.h
# End Source File
# Begin Source File

SOURCE=.\diamondt.cpp
# End Source File
# Begin Source File

SOURCE=.\dsa.cpp
# End Source File
# Begin Source File

SOURCE=.\dsa.h
# End Source File
# Begin Source File

SOURCE=.\ec2n.cpp
# End Source File
# Begin Source File

SOURCE=.\ec2n.h
# End Source File
# Begin Source File

SOURCE=.\eccrypto.cpp
# End Source File
# Begin Source File

SOURCE=.\eccrypto.h
# End Source File
# Begin Source File

SOURCE=.\ecp.cpp
# End Source File
# Begin Source File

SOURCE=.\ecp.h
# End Source File
# Begin Source File

SOURCE=.\elgamal.cpp
# End Source File
# Begin Source File

SOURCE=.\elgamal.h
# End Source File
# Begin Source File

SOURCE=.\eprecomp.cpp
# End Source File
# Begin Source File

SOURCE=.\eprecomp.h
# End Source File
# Begin Source File

SOURCE=.\files.cpp
# End Source File
# Begin Source File

SOURCE=.\files.h
# End Source File
# Begin Source File

SOURCE=.\filters.cpp
# End Source File
# Begin Source File

SOURCE=.\filters.h
# End Source File
# Begin Source File

SOURCE=.\forkjoin.cpp
# End Source File
# Begin Source File

SOURCE=.\forkjoin.h
# End Source File
# Begin Source File

SOURCE=.\fstream
# End Source File
# Begin Source File

SOURCE=.\gf256.cpp
# End Source File
# Begin Source File

SOURCE=.\gf256.h
# End Source File
# Begin Source File

SOURCE=.\gf2_32.cpp
# End Source File
# Begin Source File

SOURCE=.\gf2_32.h
# End Source File
# Begin Source File

SOURCE=.\gf2n.cpp
# End Source File
# Begin Source File

SOURCE=.\gf2n.h
# End Source File
# Begin Source File

SOURCE=.\gost.cpp
# End Source File
# Begin Source File

SOURCE=.\gost.h
# End Source File
# Begin Source File

SOURCE=.\gzip.cpp
# End Source File
# Begin Source File

SOURCE=.\gzip.h
# End Source File
# Begin Source File

SOURCE=.\haval.cpp
# End Source File
# Begin Source File

SOURCE=.\haval.h
# End Source File
# Begin Source File

SOURCE=.\hex.cpp
# End Source File
# Begin Source File

SOURCE=.\hex.h
# End Source File
# Begin Source File

SOURCE=.\hmac.h
# End Source File
# Begin Source File

SOURCE=.\idea.cpp
# End Source File
# Begin Source File

SOURCE=.\idea.h
# End Source File
# Begin Source File

SOURCE=.\integer.cpp
# End Source File
# Begin Source File

SOURCE=.\integer.h
# End Source File
# Begin Source File

SOURCE=.\iomanip
# End Source File
# Begin Source File

SOURCE=.\iosfwd
# End Source File
# Begin Source File

SOURCE=.\iostream
# End Source File
# Begin Source File

SOURCE=.\iterhash.cpp
# End Source File
# Begin Source File

SOURCE=.\iterhash.h
# End Source File
# Begin Source File

SOURCE=.\lubyrack.h
# End Source File
# Begin Source File

SOURCE=.\luc.cpp
# End Source File
# Begin Source File

SOURCE=.\luc.h
# End Source File
# Begin Source File

SOURCE=.\md5.cpp
# End Source File
# Begin Source File

SOURCE=.\md5.h
# End Source File
# Begin Source File

SOURCE=.\md5mac.cpp
# End Source File
# Begin Source File

SOURCE=.\md5mac.h
# End Source File
# Begin Source File

SOURCE=.\mdc.h
# End Source File
# Begin Source File

SOURCE=.\misc.cpp
# End Source File
# Begin Source File

SOURCE=.\misc.h
# End Source File
# Begin Source File

SOURCE=.\modarith.h
# End Source File
# Begin Source File

SOURCE=.\modes.cpp
# End Source File
# Begin Source File

SOURCE=.\modes.h
# End Source File
# Begin Source File

SOURCE=.\nbtheory.cpp
# End Source File
# Begin Source File

SOURCE=.\nbtheory.h
# End Source File
# Begin Source File

SOURCE=.\oaep.cpp
# End Source File
# Begin Source File

SOURCE=.\oaep.h
# End Source File
# Begin Source File

SOURCE=.\pch.cpp
# ADD CPP /Yc"pch.h"
# End Source File
# Begin Source File

SOURCE=.\pch.h
# End Source File
# Begin Source File

SOURCE=.\pkcspad.cpp
# End Source File
# Begin Source File

SOURCE=.\pkcspad.h
# End Source File
# Begin Source File

SOURCE=.\polynomi.cpp
# End Source File
# Begin Source File

SOURCE=.\polynomi.h
# End Source File
# Begin Source File

SOURCE=.\pubkey.cpp
# End Source File
# Begin Source File

SOURCE=.\pubkey.h
# End Source File
# Begin Source File

SOURCE=.\queue.cpp
# End Source File
# Begin Source File

SOURCE=.\queue.h
# End Source File
# Begin Source File

SOURCE=.\rabin.cpp
# End Source File
# Begin Source File

SOURCE=.\rabin.h
# End Source File
# Begin Source File

SOURCE=.\randpool.cpp
# End Source File
# Begin Source File

SOURCE=.\randpool.h
# End Source File
# Begin Source File

SOURCE=.\rc5.cpp
# End Source File
# Begin Source File

SOURCE=.\rc5.h
# End Source File
# Begin Source File

SOURCE=.\ripemd.cpp
# End Source File
# Begin Source File

SOURCE=.\ripemd.h
# End Source File
# Begin Source File

SOURCE=.\rng.cpp
# End Source File
# Begin Source File

SOURCE=.\rng.h
# End Source File
# Begin Source File

SOURCE=.\rsa.cpp
# End Source File
# Begin Source File

SOURCE=.\rsa.h
# End Source File
# Begin Source File

SOURCE=.\rsarefcl.cpp
# End Source File
# Begin Source File

SOURCE=.\rsarefcl.h
# End Source File
# Begin Source File

SOURCE=.\safer.cpp
# End Source File
# Begin Source File

SOURCE=.\safer.h
# End Source File
# Begin Source File

SOURCE=.\sapphire.cpp
# End Source File
# Begin Source File

SOURCE=.\sapphire.h
# End Source File
# Begin Source File

SOURCE=.\seal.cpp
# End Source File
# Begin Source File

SOURCE=.\seal.h
# End Source File
# Begin Source File

SOURCE=.\secshare.cpp
# End Source File
# Begin Source File

SOURCE=.\secshare.h
# End Source File
# Begin Source File

SOURCE=.\secsplit.cpp
# End Source File
# Begin Source File

SOURCE=.\secsplit.h
# End Source File
# Begin Source File

SOURCE=.\sha.cpp
# End Source File
# Begin Source File

SOURCE=.\sha.h
# End Source File
# Begin Source File

SOURCE=.\shark.cpp
# End Source File
# Begin Source File

SOURCE=.\shark.h
# End Source File
# Begin Source File

SOURCE=.\sharkbox.cpp
# End Source File
# Begin Source File

SOURCE=.\smartptr.h
# End Source File
# Begin Source File

SOURCE=.\square.cpp
# End Source File
# Begin Source File

SOURCE=.\square.h
# End Source File
# Begin Source File

SOURCE=.\squaretb.cpp
# End Source File
# Begin Source File

SOURCE=.\strstream
# End Source File
# Begin Source File

SOURCE=.\tea.cpp
# End Source File
# Begin Source File

SOURCE=.\tea.h
# End Source File
# Begin Source File

SOURCE=.\tiger.cpp
# End Source File
# Begin Source File

SOURCE=.\tiger.h
# End Source File
# Begin Source File

SOURCE=.\tigertab.cpp
# End Source File
# Begin Source File

SOURCE=.\wake.cpp
# End Source File
# Begin Source File

SOURCE=.\wake.h
# End Source File
# Begin Source File

SOURCE=.\words.h
# End Source File
# Begin Source File

SOURCE=.\xormac.h
# End Source File
# Begin Source File

SOURCE=.\zbits.cpp
# End Source File
# Begin Source File

SOURCE=.\zbits.h
# End Source File
# Begin Source File

SOURCE=.\zdeflate.cpp
# End Source File
# Begin Source File

SOURCE=.\zdeflate.h
# End Source File
# Begin Source File

SOURCE=.\zinflate.cpp
# End Source File
# Begin Source File

SOURCE=.\zinflate.h
# End Source File
# Begin Source File

SOURCE=.\ztrees.cpp
# End Source File
# Begin Source File

SOURCE=.\ztrees.h
# End Source File
# End Target
# End Project
