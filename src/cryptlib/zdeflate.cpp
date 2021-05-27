// zdeflate.cpp - modified by Wei Dai from:
// Distributed with Jean-loup Gailly's permission.

/*
 The following sorce code is derived from Info-Zip 'zip' 2.01
 distribution copyrighted by Mark Adler, Richard B. Wales,
 Jean-loup Gailly, Kai Uwe Rommel, Igor Mandrichenko and John Bush.
*/

/*
 *  deflate.c by Jean-loup Gailly.
 *
 *  PURPOSE
 *
 *      Identify new text as repetitions of old text within a fixed-
 *      length sliding window trailing behind the new text.
 *
 *  DISCUSSION
 *
 *      The "deflation" process depends on being able to identify portions
 *      of the input text which are identical to earlier input (within a
 *      sliding window trailing behind the input currently being processed).
 *
 *      The most straightforward technique turns out to be the fastest for
 *      most input files: try all possible matches and select the longest.
 *      The key feature of this algorithm is that insertions into the string
 *      dictionary are very simple and thus fast, and deletions are avoided
 *      completely. Insertions are performed at each input character, whereas
 *      string matches are performed only when the previous match ends. So it
 *      is preferable to spend more time in matches to allow very fast string
 *      insertions and avoid deletions. The matching algorithm for small
 *      strings is inspired from that of Rabin & Karp. A brute force approach
 *      is used to find longer strings when a small match has been found.
 *      A similar algorithm is used in comic (by Jan-Mark Wams) and freeze
 *      (by Leonid Broukhis).
 *         A previous version of this file used a more sophisticated algorithm
 *      (by Fiala and Greene) which is guaranteed to run in linear amortized
 *      time, but has a larger average cost, uses more memory and is patented.
 *      However the F&G algorithm may be faster for some highly redundant
 *      files if the parameter max_chain_length (described below) is too large.
 *
 *  ACKNOWLEDGEMENTS
 *
 *      The idea of lazy evaluation of matches is due to Jan-Mark Wams, and
 *      I found it in 'freeze' written by Leonid Broukhis.
 *      Thanks to many info-zippers for bug reports and testing.
 *
 *  REFERENCES
 *
 *      APPNOTE.TXT documentation file in PKZIP 1.93a distribution.
 *
 *      A description of the Rabin and Karp algorithm is given in the book
 *         "Algorithms" by R. Sedgewick, Addison-Wesley, p252.
 *
 *      Fiala,E.R., and Greene,D.H.
 *         Data Compression with Finite Windows, Comm.ACM, 32,4 (1989) 490-595
 */

#include "pch.h"
#include "zdeflate.h"
#include <stddef.h>     // for NULL

/* Define this symbol if your target allows access to unaligned data.
 * This is not mandatory, just a speed optimization. The compressed
 * output is strictly identical.
 */
#ifndef UNALIGNED_OK
#  ifdef MSDOS
#   ifndef WIN32
#    define UNALIGNED_OK
#   endif
#  endif
#  ifdef i386
#    define UNALIGNED_OK
#  endif
#  ifdef mc68020
#    define UNALIGNED_OK
#  endif
#  ifdef vax
#    define UNALIGNED_OK
#  endif
#endif

/* Compile with MEDIUM_MEM to reduce the memory requirements or
 * with SMALL_MEM to use as little memory as possible. Use BIG_MEM if the
 * entire input file can be held in memory (not possible on 16 bit systems).
 * Warning: defining these symbols affects HASH_BITS (see below) and thus
 * affects the compression ratio. The compressed output
 * is still correct, and might even be smaller in some cases.
 */

#define H_SHIFT  ((HASH_BITS+MIN_MATCH-1)/MIN_MATCH)
/* Number of bits by which ins_h and del_h must be shifted at each
 * input step. It must be such that after MIN_MATCH steps, the oldest
 * byte no longer takes part in the hash key, that is:
 *   H_SHIFT * MIN_MATCH >= HASH_BITS */

#define max_insert_length  max_lazy_match
/* Insert new strings in the hash table only if the match length
 * is not greater than this length. This saves time but degrades compression.
 * max_insert_length is used only for compression levels <= 3. */

/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may
 * be found for specific files. */

const Deflator::config Deflator::configuration_table[10] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0},  /* store only */
/* 1 */ {4,    4,  8,    4},  /* maximum speed, no lazy matches */
/* 2 */ {4,    5, 16,    8},
/* 3 */ {4,    6, 32,   32},

/* 4 */ {4,    4, 16,   16},  /* lazy matches */
/* 5 */ {8,   16, 32,   32},
/* 6 */ {8,   16, 128, 128},
/* 7 */ {8,   32, 128, 256},
/* 8 */ {32, 128, 258, 1024},
/* 9 */ {32, 258, 258, 4096}}; /* maximum compression */

/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
 * For deflate_fast() (levels <= 3) good is ignored and lazy has a different
 * meaning. */

/* Update a hash value with the given input byte
 * IN  assertion: all calls to to UPDATE_HASH are made with consecutive
 *    input characters, so that a running hash key can be computed from the
 *    previous key instead of complete recalculation each time. */
#define UPDATE_HASH(h,c) (h = (((h)<<H_SHIFT) ^ (c)) & HASH_MASK)

/* Insert string s in the dictionary and set match_head to the previous head
 * of the hash chain (the most recent string with same hash key). Return
 * the previous length of the hash chain.
 * IN  assertion: all calls to to INSERT_STRING are made with consecutive
 *    input characters and the first MIN_MATCH bytes of s are valid
 *    (except for the last MIN_MATCH-1 bytes of the input file). */
#define INSERT_STRING(s, match_head) \
   (UPDATE_HASH(ins_h, window[(s) + MIN_MATCH-1]), \
    prev[(s) & WMASK] = match_head = head[ins_h], \
    head[ins_h] = (s))

void Deflator::init_hash()
{
   unsigned j;

   for (ins_h=0, j=0; j<MIN_MATCH-1; j++) UPDATE_HASH(ins_h, window[j]);
   /* If lookahead < MIN_MATCH, ins_h is garbage, but this is
      not important since only literal bytes will be emitted. */
}

/* Initialize the "longest match" routines for a new file */
Deflator::Deflator(int deflate_level, BufferedTransformation *outQ)
    : Filter(outQ),
      CodeTree(deflate_level, *outQueue),
      window(WINDOW_SIZE), prev(WSIZE), head(HASH_SIZE)
{
   match_available = 0;
   match_length = MIN_MATCH-1;
   /* Initialize the hash table (avoiding 64K overflow for 16 bit systems).
    * prev[] will be initialized on the fly. */
    memset(head, NIL, HASH_SIZE*sizeof(*head.ptr));
   /* Set the default configuration parameters: */
   max_lazy_match   = configuration_table[deflate_level].max_lazy;
   good_match       = configuration_table[deflate_level].good_length;
   nice_match       = configuration_table[deflate_level].nice_length;
   max_chain_length = configuration_table[deflate_level].max_chain;

   strstart = 0;
   block_start = 0L;
   lookahead = 0;
   uptodate  = 0;
   minlookahead = MIN_LOOKAHEAD-1;
   match_available = 0;
   prev_length = MIN_MATCH-1;
}

void Deflator::Put(const uint8_t *inString, unsigned int length)
{
    if (deflate_level <= 3)
        fast_deflate(inString, length);
    else
        lazy_deflate(inString, length);
}

void Deflator::InputFinished()
{
    minlookahead = 0;
    Put(NULL, 0);
}

/* Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 */
#ifndef ASMV
/* For MSDOS, OS/2 and 386 Unix, an optimized version is in match.asm or
 * match.s. The code is functionally equivalent, so you can use the C version
 * if desired.  A 68000 version is in amiga/match_68.a -- this could be used
 * with other 68000 based systems such as Macintosh with a little effort.
 */
int Deflator::longest_match(IPos cur_match)
{
   unsigned chain_length = max_chain_length;   /* max hash chain length */
   uint8_t *scan = window + strstart;     /* current string */
   uint8_t *match;                        /* matched string */
   int len;                           /* length of current match */
   int best_len = prev_length;                 /* best match length so far */
   IPos limit = strstart > (IPos)MAX_DIST ? strstart - (IPos)MAX_DIST : NIL;
   /* Stop when cur_match becomes <= limit. To simplify the code,
      we prevent matches with the string of window index 0. */

/* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
 * It is easy to get rid of this optimization if necessary. */
/*
#if HASH_BITS < 8 || MAX_MATCH != 258
   #error Code too clever
#endif
*/
#ifdef UNALIGNED_OK
   /* Compare two bytes at a time. Note: this is not always beneficial.
      Try with and without -DUNALIGNED_OK to check. */
   uint8_t *strend = window + strstart + MAX_MATCH - 1;
   word16 scan_start = *(word16*)scan;
   word16 scan_end   = *(word16*)(scan+best_len-1);
#else
   uint8_t *strend = window + strstart + MAX_MATCH;
   uint8_t scan_end1 = scan[best_len-1];
   uint8_t scan_end  = scan[best_len];
#endif

   /* Do not waste too much time if we already have a good match: */
   if (prev_length >= good_match) {
       chain_length >>= 2;
   }
   //assert(strstart <= (unsigned)WINDOW_SIZE-MIN_LOOKAHEAD);

   do {
       assert(cur_match < strstart);
       match = window + cur_match;

       /* Skip to next match if the match length cannot increase
        * or if the match length is less than 2:
        */
#ifdef UNALIGNED_OK
       /* This code assumes sizeof(unsigned short) == 2. Do not use
        * UNALIGNED_OK if your compiler uses a different size.
        */
       if (*(word16*)(match+best_len-1) != scan_end ||
           *(word16*)match != scan_start) continue;

       /* It is not necessary to compare scan[2] and match[2] since they are
        * always equal when the other bytes match, given that the hash keys
        * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
        * strstart+3, +5, ... up to strstart+257. We check for insufficient
        * lookahead only every 4th comparison; the 128th check will be made
        * at strstart+257. If MAX_MATCH-2 is not a multiple of 8, it is
        * necessary to put more guard bytes at the end of the window, or
        * to check more often for insufficient lookahead.
        */
       scan++, match++;
       do {
       } while (*(word16*)(scan+=2) == *(word16*)(match+=2) &&
                *(word16*)(scan+=2) == *(word16*)(match+=2) &&
                *(word16*)(scan+=2) == *(word16*)(match+=2) &&
                *(word16*)(scan+=2) == *(word16*)(match+=2) &&
                scan < strend);
       /* The funny "do {}" generates better code on most compilers */

       /* Here, scan <= window+strstart+257 */
       assert(scan <= window+(unsigned)(WINDOW_SIZE-1));
       if (*scan == *match) scan++;

       len = (MAX_MATCH - 1) - (int)(strend-scan);
       scan = strend - (MAX_MATCH-1);

#else /* UNALIGNED_OK */

       if (match[best_len]   != scan_end  ||
           match[best_len-1] != scan_end1 ||
           *match            != *scan     ||
           *++match          != scan[1])      continue;

       /* The check at best_len-1 can be removed because it will be made
        * again later. (This heuristic is not always a win.)
        * It is not necessary to compare scan[2] and match[2] since they
        * are always equal when the other bytes match, given that
        * the hash keys are equal and that HASH_BITS >= 8.
        */
       scan += 2, match++;

       /* We check for insufficient lookahead only every 8th comparison;
        * the 256th check will be made at strstart+258.
        */
       do {
       } while (*++scan == *++match && *++scan == *++match &&
                *++scan == *++match && *++scan == *++match &&
                *++scan == *++match && *++scan == *++match &&
                *++scan == *++match && *++scan == *++match &&
                scan < strend);

       len = MAX_MATCH - (int)(strend - scan);
       scan = strend - MAX_MATCH;

#endif /* UNALIGNED_OK */

       if (len > best_len) {
           match_start = cur_match;
           best_len = len;
           if (len >= nice_match) break;
#ifdef UNALIGNED_OK
           scan_end = *(word16*)(scan+best_len-1);
#else
           scan_end1  = scan[best_len-1];
           scan_end   = scan[best_len];
#endif
       }
   } while ((cur_match = prev[cur_match & WMASK]) > limit
            && --chain_length != 0);

   return best_len;
}
#endif /* ASMV */

#if defined(DEBUG) && 0
/* Check that the match at match_start is indeed a match. */
static void check_match(start, match, length)
IPos start, match;
int length;
{
   if (memcmp((char*)window + match, (char*)window + start, length) != 0)
   {
      fprintf(stderr, " start %d, match %d, length %d\n",
         start, match, length);
      error("invalid match");
   }
   if (verbose > 1) {
      fprintf(stderr,"\\[%d,%d]", start-match, length);
      do { putc(window[start++], stderr); } while (--length != 0);
   }
}
#else
#  define check_match(start, match, length)
#endif

/* Add a block of data into the window. Updates strstart and lookahead.
 * IN assertion: lookahead < MIN_LOOKAHEAD.
 * Note: call with either lookahead == 0 or length == 0 is valid
 */
unsigned Deflator::fill_window(const uint8_t *buffer, unsigned int length)
{
   unsigned n, m;
   unsigned more = length;

   /* Amount of free space at the end of the window. */
   if (WINDOW_SIZE - lookahead - strstart < more) {
      more = (unsigned)(WINDOW_SIZE - lookahead - strstart);
   }
   /* If the window is almost full and there is insufficient lookahead,
    * move the upper half to the lower one to make room in the upper half.
    */
   if (strstart >= (unsigned)WSIZE+MAX_DIST) {
      memcpy(window, window+(unsigned int)WSIZE, WSIZE);
      match_start -= WSIZE;
      strstart    -= WSIZE; /* we now have strstart >= MAX_DIST: */

      block_start -= (long) WSIZE;

      for (n = 0; n < (unsigned)HASH_SIZE; n++) {
         m = head[n];
         head[n] = (Pos)(m >= (unsigned)WSIZE ? m-WSIZE : NIL);
      }
      for (n = 0; n < (unsigned)WSIZE; n++) {
         m = prev[n];
         prev[n] = (Pos)(m >= (unsigned)WSIZE ? m-WSIZE : NIL);
         /* If n is not on any hash chain, prev[n] is garbage but
            its value will never be used. */
      }
      if ((more += WSIZE) > length) more = length;
   }
   if (more) {
      memcpy((uint8_t*)window+strstart+lookahead, buffer, more);
      lookahead += more;
   }
   return more;
}

/* Flush the current block, with given end-of-file flag.
   IN assertion: strstart is set to the end of the current match. */
#define FLUSH_BLOCK(eof) flush_block(block_start >= 0L ?\
        window+block_start : \
        (uint8_t *)0, (long)strstart - block_start, (eof))

/* Processes a new input block.
 * This function does not perform lazy evaluationof matches and inserts
 * new strings in the dictionary only for unmatched strings or for short
 * matches. It is used only for the fast compression options. */
int Deflator::fast_deflate(const uint8_t *buffer, unsigned int length)
{
   IPos hash_head; /* head of the hash chain */
   int flush;      /* set if current block must be flushed */
   unsigned accepted = 0;

   do {
      /* Make sure that we always have enough lookahead, except
       * at the end of the input file. We need MAX_MATCH bytes
       * for the next match, plus MIN_MATCH bytes to insert the
       * string following the next match. */
      accepted += fill_window(buffer+accepted, length-accepted);
      if (lookahead <= minlookahead) break;
      if (!uptodate) {
         match_length = 0; init_hash(); uptodate = 1;
      }
      while (lookahead > minlookahead) {
         /* Insert the string window[strstart .. strstart+2] in the
          * dictionary, and set hash_head to the head of the hash chain:
          */
         INSERT_STRING(strstart, hash_head);

         /* Find the longest match, discarding those <= prev_length.
          * At this point we have always match_length < MIN_MATCH */
         if (hash_head != NIL && strstart - hash_head <= MAX_DIST) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            match_length = longest_match(hash_head);
            /* longest_match() sets match_start */
            if (match_length > lookahead) match_length = lookahead;
         }
         if (match_length >= MIN_MATCH) {
            check_match(strstart, match_start, match_length);

            flush = ct_tally(strstart-match_start, match_length - MIN_MATCH);

            lookahead -= match_length;

            /* Insert new strings in the hash table only if the match length
             * is not too large. This saves time but degrades compression.
             */
            if (match_length <= max_insert_length) {
                match_length--; /* string at strstart already in hash table */
                do {
                    strstart++;
                    INSERT_STRING(strstart, hash_head);
                    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                     * always MIN_MATCH bytes ahead. If lookahead < MIN_MATCH
                     * these bytes are garbage, but it does not matter since
                     * the next lookahead bytes will be emitted as literals.
                     */
                } while (--match_length != 0);
                strstart++;
            } else {
                strstart += match_length;
                match_length = 0;
                ins_h = window[strstart];
                UPDATE_HASH(ins_h, window[strstart+1]);
/*
#if MIN_MATCH != 3
                Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
*/
            }
         } else {
            /* No match, output a literal byte */
//            Tracevv((stderr,"%c",window[strstart]));
            flush = ct_tally (0, window[strstart]);
            lookahead--;
            strstart++;
         }
         if (flush) {
            FLUSH_BLOCK(0);
            block_start = strstart;
         }
      }
   } while (accepted < length);
   if (!minlookahead) {/* eof achieved */
      FLUSH_BLOCK(1);
   }
   return accepted;
}

/* Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.  */
int Deflator::lazy_deflate(const uint8_t *buffer, unsigned int length)
{
   IPos hash_head;          /* head of hash chain */
   IPos prev_match;         /* previous match */
   int flush;               /* set if current block must be flushed */
   unsigned ml = match_length; /* length of best match */
#ifdef DEBUG
   extern word32 isize;        /* byte length of input file, for debug only */
#endif
   unsigned accepted = 0;

   /* Process the input block. */
   do {
      /* Make sure that we always have enough lookahead, except
       * at the end of the input file. We need MAX_MATCH bytes
       * for the next match, plus MIN_MATCH bytes to insert the
       * string following the next match. */
      accepted += fill_window(buffer+accepted, length-accepted);
      if (lookahead <= minlookahead) break;
      if (!uptodate) {
         ml = MIN_MATCH-1; /* length of best match */
         init_hash();
         uptodate = 1;
      }
      while (lookahead > minlookahead) {
         INSERT_STRING(strstart, hash_head);

         /* Find the longest match, discarding those <= prev_length. */
         prev_length = ml, prev_match = match_start;
         ml = MIN_MATCH-1;

         if (hash_head != NIL && prev_length < max_lazy_match &&
             strstart - hash_head <= MAX_DIST) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            ml = longest_match (hash_head);
            /* longest_match() sets match_start */
            if (ml > lookahead) ml = lookahead;

            /* Ignore a length 3 match if it is too distant: */
            if (ml == MIN_MATCH && strstart-match_start > TOO_FAR){
               /* If prev_match is also MIN_MATCH, match_start is garbage
                  but we will ignore the current match anyway. */
               ml--;
            }
         }
         /* If there was a match at the previous step and the current
            match is not better, output the previous match: */
         if (prev_length >= MIN_MATCH && ml <= prev_length) {

            check_match(strstart-1, prev_match, prev_length);

            flush = ct_tally(strstart-1-prev_match, prev_length - MIN_MATCH);

            /* Insert in hash table all strings up to the end of the match.
             * strstart-1 and strstart are already inserted.
             */
            lookahead -= prev_length-1;
            prev_length -= 2;
            do {
               strstart++;
               INSERT_STRING(strstart, hash_head);
               /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                * always MIN_MATCH bytes ahead. If lookahead < MIN_MATCH
                * these bytes are garbage, but it does not matter since the
                * next lookahead bytes will always be emitted as literals.
                */
            } while (--prev_length != 0);
            match_available = 0;
            ml = MIN_MATCH-1;
            strstart++;
            if (flush) {
               FLUSH_BLOCK(0);
               block_start = strstart;
            }

         } else if (match_available) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
//            Tracevv((stderr,"%c",window[strstart-1]));
            if (ct_tally (0, window[strstart-1])) {
                FLUSH_BLOCK(0), block_start = strstart;
            }
            strstart++;
            lookahead--;
         } else {
            /* There is no previous match to compare with,
               wait for the next step to decide. */
            match_available = 1;
            strstart++;
            lookahead--;
         }
//         assert(strstart <= isize && lookahead <= isize);
      }
   } while (accepted < length);
   if (!minlookahead) {/* eof achieved */
      if (match_available) ct_tally (0, window[strstart-1]);
      FLUSH_BLOCK(1);
   }
   match_length = ml;
   return accepted;
}
