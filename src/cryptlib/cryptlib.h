// cryptlib.h - written and placed in the public domain by Wei Dai

// This file contains the declarations for the abstract base
// classes that provide a uniform interface to this library.

#ifndef CRYPTLIB_H
#define CRYPTLIB_H

#include "config.h"
#include <exception>
#include <string>

/// base class for all exceptions thrown by Crypto++
class CryptlibException : public std::exception
{
public:
    explicit CryptlibException(const std::string& s) : m_what(s) {}
    virtual ~CryptlibException() throw() {}
    const char *what() const throw() {return (m_what.c_str());}

private:
    std::string m_what;
};

/// used to specify a direction for a cipher to operate in (encrypt or decrypt)
enum CipherDir {
    ///
    ENCRYPTION, 
    ///
    DECRYPTION};


/// abstract base class for block ciphers

/** A particular derived class may change state as blocks are processed,
    so the ProcessBlock() functions are not specified to be const.

    However, most classes derived from BlockTransformation are block ciphers
    in ECB mode (for example the DESEncryption class), which are stateless.
    These classes should not be used directly, but only in combination with
    a mode class (see \Ref{Mode}).  However, if you know what you are doing, and
    need to call ProcessBlock() on a const ECB cipher object, you can safely
    cast away its constness.

    Note: BlockTransformation objects may assume that pointers to input and
    output blocks are aligned on 32-bit word boundaries.
*/
class BlockTransformation
{
public:
    ///
    virtual ~BlockTransformation() {}

    /// encrypt or decrypt one block in place
    //* Precondition: size of inoutBlock == BlockSize().
    virtual void ProcessBlock(uint8_t *inoutBlock) =0;

    /// encrypt or decrypt one block, may assume inBlock != outBlock
    //* Precondition: size of inBlock and outBlock == BlockSize().
    virtual void ProcessBlock(const uint8_t *inBlock, uint8_t *outBlock) =0;

    /// block size of the cipher in bytes
    virtual unsigned int BlockSize() const =0;
};


/// abstract base class for stream ciphers

class StreamCipher
{
public:
    ///
    virtual ~StreamCipher() {}

    /// encrypt or decrypt one byte
    virtual uint8_t ProcessByte(uint8_t input) =0;

    /// encrypt or decrypt an array of bytes of specified length in place
    virtual void ProcessString(uint8_t *inoutString, unsigned int length);
    /// encrypt or decrypt an array of bytes of specified length, may assume inString != outString
    virtual void ProcessString(uint8_t *outString, const uint8_t *inString, unsigned int length);
};

/// abstract base class for random access stream ciphers

class RandomAccessStreamCipher : public virtual StreamCipher
{
public:
    ///
    virtual ~RandomAccessStreamCipher() {}
    /*/ specify that the next byte to be processed is at absolute position n
        in the plaintext/ciphertext stream */
    virtual void Seek(unsigned long n) =0;
};

/// abstract base class for random number generators
/** All return values are uniformly distributed over the range specified.
*/
class RandomNumberGenerator
{
public:
    ///
    virtual ~RandomNumberGenerator() {}

    /// generate new random byte and return it
    virtual uint8_t GetByte() =0;

    /// generate new random bit and return it
    /** Default implementation is to call GetByte() and return its parity. */
    virtual unsigned int GetBit();

    /// generate a random 32 bit word in the range min to max, inclusive
    virtual word32 GetLong(word32 a=0, word32 b=0xffffffffL);
    /// generate a random 16 bit word in the range min to max, inclusive
    virtual word16 GetShort(word16 a=0, word16 b=0xffff)
        {return (word16)GetLong(a, b);}

    /// generate random array of bytes
    //* Default implementation is to call GetByte size times.
    virtual void GetBlock(uint8_t *output, unsigned int size);
};

/// randomly shuffle the specified array, resulting permutation is uniformly distributed
template <class T> void Shuffle(RandomNumberGenerator &rng, T *array, unsigned int size)
{
    while (--size)
        swap(array[size], array[(unsigned int)rng.GetLong(0, size)]);
}

/// abstract base class for hash functions
/** HashModule objects are stateful.  They are created in an initial state,
    change state as Update() is called, and return to the initial
    state when Final() is called.  This interface allows a large message to
    be hashed in pieces by calling Update() on each piece followed by
    calling Final().
*/
class HashModule
{
public:
    ///
    virtual ~HashModule() {}

    /// process more input
    virtual void Update(const uint8_t *input, unsigned int length) =0;

    /*/ calculate hash for the current message (the concatenation of all 
        inputs passed in via Update()), then reinitialize the object */
    //* Precondition: size of digest == DigestSize().
    virtual void Final(uint8_t *digest) =0;

    /// size of the hash returned by Final()
    virtual unsigned int DigestSize() const =0;

    /// use this if your input is short and you don't want to call Update() and Final() seperately
    virtual void CalculateDigest(uint8_t *digest, const uint8_t *input, int length)
        {Update(input, length); Final(digest);}
};

/// abstract base class for message authentication codes

/** The main differences between a MAC and an hash function (in terms of
    programmatic interface) is that a MAC is keyed, and that calculating
    a MAC for the same message twice may produce two different results so
    verifying a MAC may not be simply recalculating it and doing a bitwise
    comparison.
*/
class MessageAuthenticationCode : public virtual HashModule
{
public:
    ///
    virtual ~MessageAuthenticationCode() {}

    /// verify that mac is a valid MAC for the current message, then reinitialize the object
    /** Default implementation is to call Final() and do a bitwise comparison
        between its output and mac. */
    virtual bool Verify(const uint8_t *mac);

    /// use this if your input is short and you don't want to call Update() and Verify() seperately
    virtual bool VerifyMAC(const uint8_t *mac, const uint8_t *input, int length)
        {Update(input, length); return Verify(mac);}
};

/// abstract base class for buffered transformations

/** BufferedTransformation is a generalization of \Ref{BlockTransformation},
    \Ref{StreamCipher}, and \Ref{HashModule}.

    A buffered transformation is an object that takes a stream of bytes 
    as input (this may be done in stages), does some computation on them, and
    then places the result into an internal buffer for later retrieval.  Any
    partial result already in the output buffer is not modified by further
    input.
    
    Computation is generally done as soon as possible, but some buffering
    on the input may be done for performance reasons.
*/
class BufferedTransformation
{
public:
    ///
    virtual ~BufferedTransformation() {}

    //@Man: INPUT
    //@{
        /// input a byte for processing
        virtual void Put(uint8_t inByte) =0;
        /// input multiple bytes
        virtual void Put(const uint8_t *inString, unsigned int length) =0;
        /// signal that no more input is available
        virtual void InputFinished() {}

        /// input a 16-bit word, big-endian or little-endian depending on highFirst
        void PutShort(word16 value, bool highFirst=true);
        /// input a 32-bit word
        void PutLong(word32 value, bool highFirst=true);
    //@}

    //@Man: RETRIEVAL
    //@{
        /// returns number of bytes that is currently ready for retrieval
        /** All retrieval functions return the actual number of bytes
            retrieved, which is the lesser of the request number and
            MaxRetrieveable(). */
        virtual unsigned long MaxRetrieveable() =0;

        /// try to retrieve a single byte
        virtual unsigned int Get(uint8_t &outByte) =0;
        /// try to retrieve multiple bytes
        virtual unsigned int Get(uint8_t *outString, unsigned int getMax) =0;

        /// try to retrieve a 16-bit word, big-endian or little-endian depending on highFirst
        int GetShort(word16 &value, bool highFirst=true);
        /// try to retrieve a 32-bit word
        int GetLong(word32 &value, bool highFirst=true);

        /// move all of the buffered output to target as input
        virtual void TransferTo(BufferedTransformation &target);
        /// same as above but only transfer up to transferMax bytes
        virtual unsigned int TransferTo(BufferedTransformation &target, unsigned int transferMax);

        /// peek at the next byte without removing it from the output buffer
        virtual unsigned int Peek(uint8_t &outByte) const =0;

        /// discard some bytes from the output buffer
        unsigned int Skip(unsigned int skipMax);
    //@}

    //@Man: ATTACHMENT
    //@{
        /** Some BufferedTransformation objects (e.g. \Ref{Filter} objects)
            allow other BufferedTransformation objects to be attached.  When
            this is done, the first object instead of buffering its output,
            sents that output to the attached object as input.  See the
            documentation for the \Ref{Filter} class for the details.
        */
        ///
        virtual bool Attachable() {return false;}
        ///
        virtual void Detach(BufferedTransformation *p = 0) {}   // NULL is undefined at this point
        ///
        virtual void Attach(BufferedTransformation *) {}
        /// call InputFinished() for all attached objects
        virtual void Close() {InputFinished();}
    //@}
};

/// abstract base class for public-key encryptors and decryptors

/** This class provides an interface common to encryptors and decryptors
    for querying their plaintext and ciphertext lengths.
*/
class PK_CryptoSystem
{
public:
    ///
    virtual ~PK_CryptoSystem() {}

    /// maximum length of plaintext for a given ciphertext length
    //* This function returns 0 if cipherTextLength is not valid (too long or too short).
    virtual unsigned int MaxPlainTextLength(unsigned int cipherTextLength) const =0;

    /// calculate length of ciphertext given length of plaintext
    //* This function returns 0 if plainTextLength is not valid (too long).
    virtual unsigned int CipherTextLength(unsigned int plainTextLength) const =0;
};

/// abstract base class for public-key encryptors

/** An encryptor is also a public encryption key.  It contains both the
    key and the algorithm to perform the encryption.
*/
class PK_Encryptor : public virtual PK_CryptoSystem
{
public:
    /// encrypt a byte string
    /** Preconditions:
            \begin{itemize} 
            \item CipherTextLength(plainTextLength) != 0 (i.e., plainText isn't too long)
            \item size of cipherText == CipherTextLength(plainTextLength)
            \end{itemize}
    */
    virtual void Encrypt(RandomNumberGenerator &rng, const uint8_t *plainText, unsigned int plainTextLength, uint8_t *cipherText) =0;
};

/// abstract base class for public-key decryptors

/** An decryptor is also a private decryption key.  It contains both the
    key and the algorithm to perform the decryption.
*/
class PK_Decryptor : public virtual PK_CryptoSystem
{
public:
    /// decrypt a byte string, and return the length of plaintext
    /** Precondition: size of plainText == MaxPlainTextLength(cipherTextLength)
        bytes.  
        
        The function returns the actual length of the plaintext, or 0
        if decryption fails.
    */
    virtual unsigned int Decrypt(const uint8_t *cipherText, unsigned int cipherTextLength, uint8_t *plainText) =0;
};

/// abstract base class for encryptors and decryptors with fixed length ciphertext

/** A simplified interface (as embodied in this
    class and its subclasses) is provided for crypto systems (such
    as RSA) whose ciphertext depend only on the key, not on the length
    of the plaintext.  The maximum plaintext length also depend only on
    the key.
*/
class PK_FixedLengthCryptoSystem : public virtual PK_CryptoSystem
{
public:
    ///
    virtual unsigned int MaxPlainTextLength() const =0;
    ///
    virtual unsigned int CipherTextLength() const =0;

    unsigned int MaxPlainTextLength(unsigned int cipherTextLength) const;
    unsigned int CipherTextLength(unsigned int plainTextLength) const;
};

/// abstract base class for encryptors with fixed length ciphertext

class PK_FixedLengthEncryptor : public virtual PK_Encryptor, public virtual PK_FixedLengthCryptoSystem
{
};

/// abstract base class for decryptors with fixed length ciphertext

class PK_FixedLengthDecryptor : public virtual PK_Decryptor, public virtual PK_FixedLengthCryptoSystem
{
public:
    /// decrypt a byte string, and return the length of plaintext
    /** Preconditions:
            \begin{itemize} 
            \item length of cipherText == CipherTextLength()
            \item size of plainText == MaxPlainTextLength()
            \end{itemize}
        
        The function returns the actual length of the plaintext, or 0
        if decryption fails.
    */
    virtual unsigned int Decrypt(const uint8_t *cipherText, uint8_t *plainText) =0;

    unsigned int Decrypt(const uint8_t *cipherText, unsigned int cipherTextLength, uint8_t *plainText);
};

/// abstract base class for public-key signers and verifiers

/** This class provides an interface common to signers and verifiers
    for querying their signature lengths and maximum message lengths.

    The maximum message length is typically very low (less than 1000)
    because it is intended that only message digests (see \Ref{HashModule})
    should be signed.
*/
class PK_SignatureSystem
{
public:
    ///
    virtual ~PK_SignatureSystem() {};

    /// maximum length of a message that can be signed or verified
    virtual unsigned int MaxMessageLength() const =0;
    /// signature length support by this object (as either input or output)
    virtual unsigned int SignatureLength() const =0;
};

/// abstract base class for public-key signers

/** A signer is also a private signature key.  It contains both the
    key and the algorithm to perform the signature.
*/
class PK_Signer : public virtual PK_SignatureSystem
{
public:
    /// sign a message
    /** Preconditions:
            \begin{itemize} 
            \item messageLen <= MaxMessageLength()
            \item size of signature == SignatureLength()
            \end{itemize}
    */
    virtual void Sign(RandomNumberGenerator &rng, const uint8_t *message, unsigned int messageLen, uint8_t *signature) =0;
};

/// abstract base class for public-key verifiers

/** A verifier is also a public verification key.  It contains both the
    key and the algorithm to perform the verification.
*/
class PK_Verifier : public virtual PK_SignatureSystem
{
public:
    /// check whether sig is a valid signature for message
    /** Preconditions:
            \begin{itemize} 
            \item messageLen <= MaxMessageLength()
            \item length of signature == SignatureLength()
            \end{itemize}
    */
    virtual bool Verify(const uint8_t *message, unsigned int messageLen, const uint8_t *sig) =0;
};

/// abstract base class for public-key verifiers with recovery

/** In a signature scheme with recovery, a verifier is able to extract
    a message from its valid signature.  This saves space since
    you don't need to store the message seperately.
*/
class PK_VerifierWithRecovery : public PK_Verifier
{
public:
    /// recover a message from its signature, return length of message, or 0 if signature is invalid
    /** Preconditions:
            \begin{itemize} 
            \item length of signature == SignatureLength()
            \item size of recoveredMessage == MaxMessageLength()
            \end{itemize}
    */
    virtual unsigned int Recover(const uint8_t *signature, uint8_t *recoveredMessage) =0;

    bool Verify(const uint8_t *message, unsigned int messageLen, const uint8_t *signature);
};

/// abstract base class for key agreement protocols

/** This class defines the interface for symmetric 2-pass key agreement
    protocols.  It isn't very general and only basic Diffie-Hellman
    protocols fit the abstraction, so possibly a more general interface
    is needed.

    To use a KeyAgreementProtocol class, the two parties create matching
    KeyAgreementProtocol objects, call Setup() on these objects,
    and send each other the public values produced.  The objects are
    responsible for remembering the corresponding secret values, and
    will produce a shared secret value when Agree() is called with the
    other party's public value.
*/
class KeyAgreementProtocol
{
public:
    ///
    virtual ~KeyAgreementProtocol() {}

    ///
    virtual unsigned int PublicValueLength() const =0;
    ///
    virtual unsigned int AgreedKeyLength() const =0;

    /// produce public value
    //* Precondition: size of publicValue == PublicValueLength()
    virtual void Setup(RandomNumberGenerator &rng, uint8_t *publicValue) =0;

    /// calculate agreed key given other party's public value
    /** Precondition:
            \begin{itemize} 
            \item Setup() was called previously on this object
            \item size of agreedKey == AgreedKeyLength()
            \end{itemize}
    */
    virtual void Agree(const uint8_t *otherPublicValue, uint8_t *agreedKey) const =0;
};

/// abstract base class for all objects that support precomputation

/** The class defines a common interface for doing precomputation,
    and loading and saving precomputation.
*/
class PK_Precomputation
{
public:
    ///
    virtual ~PK_Precomputation() {}

    ///
    /** The exact semantics of Precompute() is varies, but
        typically it means calculate a table of n objects
        that can be used later to speed up computation.
    */
    virtual void Precompute(unsigned int n) =0;

    /// retrieve previously saved precomputation
    virtual void LoadPrecomputation(BufferedTransformation &storedPrecomputation) =0;
    /// save precomputation for later use
    virtual void SavePrecomputation(BufferedTransformation &storedPrecomputation) const =0;
};

///
template <class T> class PK_WithPrecomputation : public T, public virtual PK_Precomputation
{
};

#endif
