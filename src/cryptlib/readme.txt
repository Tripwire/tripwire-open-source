Crypto++: a C++ Class Library of Cryptographic Primitives
Version 2.3  1/17/1998

This library includes:

MD5, SHA, HAVAL, Tiger, RIPE-MD160, MD5-MAC, HMAC, XOR-MAC, DES, IDEA, 
WAKE, 3-WAY, TEA, SAFER, Blowfish, SHARK, GOST, CAST-128, Square, Diamond2,
Sapphire, RC5, SEAL, Luby-Rackoff, MDC, various encryption modes (CFB,
CBC, OFB, counter), DH, DSA, ElGamal, LUC, LUCDIF, LUCELG, Rabin, RSA,
BlumGoldwasser, elliptic curve cryptosystems, BBS, DEFLATE compression, 
Shamir's secret sharing scheme, Rabin's information dispersal scheme.
There are also various miscellanous modules such as base 64 coding and
32-bit CRC.

You are welcome to use it for any purpose without paying me, but see
license.txt for the fine print.

Crypto++ has been compiled successfully with MSVC 5.0 and EGCS/cygwin32
1.0 on Windows NT 4.0.

To compile Crypto++ with MSVC, open the "cryptest.dsw" workspace file
and build the "cryptest" project. This will compile Crypto++ as a static
library and also build the test driver. Run the test driver and make sure
the validation suite passes.  Then to use the library simply insert the
"cryptlib.dsp" project file into your own application workspace as a
dependent project.

A makefile is included for you to compile Crypto++ with EGCS. But first
you may have to make some changes to the system header files to work around
problems in the EGCS 1.0 release. Please check the Crypto++ homepage
(see below) for the latest updates.

Right now there is very little documentation for Crypto++.  If you are
not familiar with cryptography, I suggest that you read an introductory
text (such as Bruce Schneier's _Applied Cryptography_) before attempting
to use this library.  Otherwise, you should start by looking at
cryptlib.h, which contains the main abstract base classes and their
descriptions, and test.cpp, which contains sample/test code.  

Finally, a note on object ownership:  If a constructor for A takes 
a pointer to an object B (except primitive types such as int and char),
then A owns B and will delete B at A's destruction.  If a constructor 
for A takes a reference to an object B, then the caller retains ownership 
of B and should not destroy it until A no longer needs it.

Good luck, and feel free to e-mail me at weidai@eskimo.com if you have
any problems.  Also, check http://www.eskimo.com/~weidai/cryptlib.html
for updates and new versions.

Wei Dai

History

1.0 - First public release.  Withdrawn at the request of RSA DSI.
    - included Blowfish, BBS, DES, DH, Diamond, DSA, ElGamal, IDEA,
      MD5, RC4, RC5, RSA, SHA, WAKE, secret sharing, DEFLATE compression
    - had a big bug in the RSA key generation code.

1.1 - Removed RSA, RC4, RC5
    - Disabled calls to RSAREF's non-public functions
    - Minor bugs fixed

2.0 - a completely new, faster multiprecision integer class
    - added MD5-MAC, HAVAL, 3-WAY, TEA, SAFER, LUC, Rabin, BlumGoldwasser,
      elliptic curve algorithms
    - added the Lucas strong probable primality test
    - ElGamal encryption and signature schemes modified to avoid weaknesses
    - Diamond changed to Diamond2 because of key schedule weakness
    - fixed bug in WAKE key setup
    - SHS class renamed to SHA
    - lots of miscellaneous optimizations

2.1 - added Tiger, HMAC, GOST, RIPE-MD160, LUCELG, LUCDIF, XOR-MAC,
      OAEP, PSSR, SHARK
    - added precomputation to DH, ElGamal, DSA, and elliptic curve algorithms
    - added back RC5 and a new RSA
    - optimizations in elliptic curves over GF(p)
    - changed Rabin to use OAEP and PSSR
    - changed many classes to allow copy constructors to work correctly
    - improved exception generation and handling

2.2 - added SEAL, CAST-128, Square
    - fixed bug in HAVAL (padding problem)
    - fixed bug in triple-DES (decryption order was reversed)
    - fixed bug in RC5 (couldn't handle key length not a multiple of 4)
    - changed HMAC to conform to RFC-2104 (which is not compatible
      with the original HMAC)
    - changed secret sharing and information dispersal to use GF(2^32)
      instead of GF(65521)
    - removed zero knowledge prover/verifier for graph isomorphism
    - removed several utility classes in favor of the C++ standard library

2.3 - ported to EGCS
    - fixed incomplete workaround of min/max conflict in MSVC
