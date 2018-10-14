#ifndef ZDEFLATE_H
#define ZDEFLATE_H

#include "cryptlib.h"
#include "misc.h"
#include "filters.h"
#include "ztrees.h"

class Deflator : public Filter, private CodeTree
{
public:
    // deflate_level can be from 1 to 9, 1 being fastest, 9 being most compression
    // default for the gzip program is 6
    Deflator(int deflate_level, BufferedTransformation *outQ = NULL);

    void Put(uint8_t inByte)
        {Deflator::Put(&inByte, 1);}
    void Put(const uint8_t *inString, unsigned int length);

    void InputFinished();

private:
#ifdef SMALL_MEM
    enum {HASH_BITS=13};
#else
#  ifdef MEDIUM_MEM
    enum {HASH_BITS=14};
#  else
    enum {HASH_BITS=15};
#  endif
#endif

    enum {HASH_SIZE = 1<<HASH_BITS, HASH_MASK = HASH_SIZE-1,
          WINDOW_SIZE = 2*WSIZE, WMASK = WSIZE-1,
          NIL = 0,  // Tail of hash chains
          // Matches of length 3 are discarded if their distance exceeds TOO_FAR
          TOO_FAR = 4096};

    struct config
    {
       word16 good_length; /* reduce lazy search above this match length */
       word16 max_lazy;    /* do not perform lazy search above this match length */
       word16 nice_length; /* quit search above this match length */
       word16 max_chain;
    };

    static const config configuration_table[10];

    typedef word16 Pos;
    typedef unsigned IPos;

    SecByteBlock window;
    SecBlock<Pos> prev, head;

    unsigned fill_window (const uint8_t*, unsigned);
    void     init_hash   ();

    int longest_match (IPos cur_match);

    int fast_deflate(const uint8_t *buffer, unsigned int length);
    int lazy_deflate(const uint8_t *buffer, unsigned int length);

    unsigned ins_h;  /* hash index of string to be inserted */
    char uptodate;   /* hash preparation flag */

    unsigned int prev_length;
    /* Length of the best match at previous step. Matches not greater than this
     * are discarded. This is used in the lazy match evaluation. */

    unsigned match_start; /* start of matching string */
    unsigned lookahead;   /* number of valid bytes ahead in window */
    unsigned minlookahead;

    unsigned max_chain_length;
    /* To speed up deflation, hash chains are never searched beyond this length.
     * A higher limit improves compression ratio but degrades the speed. */

    unsigned int max_lazy_match;
    /* Attempt to find a better match only when the current match is strictly
     * smaller than this value. This mechanism is used only for compression
     * levels >= 4. */

    unsigned good_match;
    /* Use a faster search when the previous match is longer than this */
    int nice_match; /* Stop searching when current match exceeds this */

    /* A block of local deflate process data to be saved between
     * sequential calls to deflate functions */
    int match_available; /* set if previous match exists */
    unsigned match_length; /* length of best match */
};

#endif

