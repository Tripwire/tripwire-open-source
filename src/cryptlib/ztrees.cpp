// ztrees.cpp - modified by Wei Dai from:
// Distributed with Jean-loup Gailly's permission.

/*
 The following sorce code is derived from Info-Zip 'zip' 2.01
 distribution copyrighted by Mark Adler, Richard B. Wales,
 Jean-loup Gailly, Kai Uwe Rommel, Igor Mandrichenko and John Bush.
*/

/*
 *  trees.c by Jean-loup Gailly
 *
 *  This is a new version of im_ctree.c originally written by Richard B. Wales
 *  for the defunct implosion method.
 *
 *  PURPOSE
 *
 *      Encode various sets of source values using variable-length
 *      binary code trees.
 *
 *  DISCUSSION
 *
 *      The PKZIP "deflation" process uses several Huffman trees. The more
 *      common source values are represented by shorter bit sequences.
 *
 *      Each code tree is stored in the ZIP file in a compressed form
 *      which is itself a Huffman encoding of the lengths of
 *      all the code strings (in ascending order by source values).
 *      The actual code strings are reconstructed from the lengths in
 *      the UNZIP process, as described in the "application note"
 *      (APPNOTE.TXT) distributed as part of PKWARE's PKZIP program.
 *
 *  REFERENCES
 *
 *      Lynch, Thomas J.
 *          Data Compression:  Techniques and Applications, pp. 53-55.
 *          Lifetime Learning Publications, 1985.  ISBN 0-534-03418-7.
 *
 *      Storer, James A.
 *          Data Compression:  Methods and Theory, pp. 49-50.
 *          Computer Science Press, 1988.  ISBN 0-7167-8156-5.
 *
 *      Sedgewick, R.
 *          Algorithms, p290.
 *          Addison-Wesley, 1983. ISBN 0-201-06672-6.
 *
 *  INTERFACE
 *
 *      int ct_init (void)
 *          Allocate the match buffer and initialize the various tables.
 *
 *      int ct_tally(int dist, int lc);
 *          Save the match info and tally the frequency counts.
 *          Return true if the current block must be flushed.
 *
 *      long flush_block (char *buf, ulg stored_len, int eof)
 *          Determine the best encoding for the current block: dynamic trees,
 *          static trees or store, and output the encoded block to the zip
 *          file. Returns the total compressed length for the file so far.
 */

#include "pch.h"
#include "ztrees.h"

bool CodeTree::streesBuilt = false;
CodeTree::ct_data CodeTree::static_ltree[CodeTree::L_CODES+2];
CodeTree::ct_data CodeTree::static_dtree[CodeTree::D_CODES];
                                            
const int CodeTree::extra_lbits[] /* extra bits for each length code */
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

const int CodeTree::extra_dbits[] /* extra bits for each distance code */
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

const int CodeTree::extra_blbits[]/* extra bits for each bit length code */
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

const uint8_t CodeTree::bl_order[]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
/* The lengths of the bit length codes are sent in order of decreasing
 * probability, to avoid transmitting the lengths for unused bit length codes.
 */

#define send_code(c, tree) send_bits(tree[(unsigned int)c].Code, tree[(unsigned int)c].Len)
/* Send a code of the given tree. c and tree must not have side effects */

#define d_code(dist) \
   ((dist) < 256 ? dist_code[(unsigned int)dist] : dist_code[(unsigned int)(256+((dist)>>7))])
/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. dist_code[256] and dist_code[257] are never
 * used.
 */

#define MAX(a,b) (a >= b ? a : b)
/* the arguments must not have side effects */

static unsigned reverse(unsigned int code, int len)
/* Reverse the first len bits of a code. */
{
   unsigned res = 0;
   do res = (res << 1) | (code & 1), code>>=1; while (--len);
   return res;
}

/* Allocate the match buffer and initialize the various tables. */
CodeTree::CodeTree(int deflate_level, BufferedTransformation &outQ)
  : BitOutput(outQ),
    deflate_level(deflate_level),
    dyn_ltree(HEAP_SIZE), dyn_dtree(2*D_CODES+1),
    bl_tree(2*BL_CODES+1),
    bl_count(MAX_BITS+1),
    l_desc(dyn_ltree, static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_BITS, 0),
    d_desc(dyn_dtree, static_dtree, extra_dbits, 0,          D_CODES, MAX_BITS, 0),
    bl_desc(bl_tree, (ct_data *)0, extra_blbits, 0,     BL_CODES, MAX_BL_BITS, 0),
    heap(2*L_CODES+1),
    depth(2*L_CODES+1),
    length_code(MAX_MATCH-MIN_MATCH+1),
    dist_code(512),
    base_length(LENGTH_CODES),
    base_dist(D_CODES),
    l_buf(LIT_BUFSIZE),
    d_buf(DIST_BUFSIZE),
    flag_buf(LIT_BUFSIZE/8)
{

   unsigned int n;    /* iterates over tree elements */
   unsigned int bits;      /* bit counter */
   unsigned int length;    /* length value */
   unsigned int code; /* code value */
   unsigned int dist;      /* distance index */

    compressed_len = input_len = 0L;

   /* Initialize the mapping length (0..255) -> length code (0..28) */
   length = 0;
   for (code=0; code < LENGTH_CODES-1; code++) {
      base_length[code] = length;
      for (n=0; n < (1U<<extra_lbits[code]); n++) {
         length_code[length++] = (uint8_t)code;
      }
   }
   assert (length == 256);
    /* Note that the length 255 (match length 258) can be represented
       in two different ways: code 284 + 5 bits or code 285, so we
       overwrite length_code[255] to use the best encoding:     */
   length_code[length-1] = (uint8_t)code;

   /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
   dist = 0;
   for (code=0 ; code < 16; code++) {
      base_dist[code] = dist;
      for (n=0; n < (1U<<extra_dbits[code]); n++) {
         dist_code[dist++] = (uint8_t)code;
      }
   }
   assert (dist == 256);
   dist >>= 7; /* from now on, all distances are divided by 128 */
   for (; code < D_CODES; code++) {
      base_dist[code] = dist << 7;
      for (n=0; n < (1U<<(extra_dbits[code]-7)); n++) {
         dist_code[256 + dist++] = (uint8_t)code;
      }
   }
   assert (dist == 256);

   if (!streesBuilt)
   {
       /* Construct the codes of the static literal tree */
       for (bits=0; bits <= MAX_BITS; bits++) bl_count[bits] = 0;
       n = 0;
       while (n <= 143) static_ltree[n++].Len = 8, bl_count[(unsigned int)8]++;
       while (n <= 255) static_ltree[n++].Len = 9, bl_count[(unsigned int)9]++;
       while (n <= 279) static_ltree[n++].Len = 7, bl_count[(unsigned int)7]++;
       while (n <= 287) static_ltree[n++].Len = 8, bl_count[(unsigned int)8]++;
       /* Codes 286 and 287 do not exist, but we must include them in the tree
          construction to get a canonical Huffman tree (longest code all ones) */
       gen_codes(static_ltree, L_CODES+1);

       /* The static distance tree is trivial: */
       for (n=0; n < D_CODES; n++) {
          static_dtree[n].Len = 5;
          static_dtree[n].Code = reverse(n, 5);
       }
       streesBuilt = true;
   }

   /* Initialize the first block of the first file: */
   init_block();
}

/* Initialize a new block. */
void CodeTree::init_block()
{
   unsigned int n; /* iterates over tree elements */

   /* Initialize the trees. */
   for (n=0; n < L_CODES;  n++) dyn_ltree[n].Freq = 0;
   for (n=0; n < D_CODES;  n++) dyn_dtree[n].Freq = 0;
   for (n=0; n < BL_CODES; n++) bl_tree[n].Freq = 0;

   dyn_ltree[(unsigned int)END_BLOCK].Freq = 1;
   opt_len = static_len = 0L;
   last_lit = last_dist = last_flags = 0;
   flags = 0; flag_bit = 1;
}

#define SMALLEST 1
/* Index within the heap array of least frequent node in the Huffman tree */

/*
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#define pqremove(tree, top) \
{\
    top = heap[(unsigned int)SMALLEST]; \
    heap[(unsigned int)SMALLEST] = heap[(unsigned int)heap_len--]; \
    pqdownheap(tree, SMALLEST); \
}

/*
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#define smaller(tree, n, m) \
   (tree[(unsigned int)n].Freq < tree[(unsigned int)m].Freq || \
   (tree[(unsigned int)n].Freq == tree[(unsigned int)m].Freq && depth[(unsigned int)n] <= depth[(unsigned int)m]))

/*
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
void CodeTree::pqdownheap(ct_data *tree, int k)
{
    unsigned int kk = (unsigned int) k;
    int v = heap[kk];
    unsigned int j = kk << 1;  /* left son of k */
    int htemp;       /* required because of bug in SASC compiler */

    while (j <= (unsigned int)heap_len) {
        /* Set j to the smallest of the two sons: */
        if (j < (unsigned int)heap_len && smaller(tree, heap[(unsigned int)(j+1)], heap[(unsigned int)j])) j++;

        /* Exit if v is smaller than both sons */
        htemp = heap[j];
        if (smaller(tree, v, htemp)) break;

        /* Exchange v with the smallest son */
        heap[(unsigned int)k] = htemp;
        k = j;

        /* And continue down the tree, setting j to the left son of k */
        j <<= 1;
    }
    heap[(unsigned int)k] = v;
}

/*
 * Compute the optimal bit lengths for a tree and update the total bit length
 * for the current block.
 * IN assertion: the fields freq and dad are set, heap[heap_max] and
 *    above are the tree nodes sorted by increasing frequency.
 * OUT assertions: the field len is set to the optimal bit length, the
 *     array bl_count contains the frequencies for each bit length.
 *     The length opt_len is updated; static_len is also updated if stree is
 *     not null.
 */
void CodeTree::gen_bitlen(tree_desc *desc)
{
    ct_data *tree  = desc->dyn_tree;
    const int *extra     = desc->extra_bits;
    int base            = desc->extra_base;
    int max_code        = desc->max_code;
    int max_length      = desc->max_length;
    const ct_data *stree = desc->static_tree;
    unsigned int h;              /* heap index */
    int n, m;           /* iterate over the tree elements */
    unsigned int bits;           /* bit length */
    int xbits;          /* extra bits */
    word16 f;              /* frequency */
    int overflow = 0;   /* number of elements with bit length too large */

    for (bits = 0; bits <= MAX_BITS; bits++) bl_count[bits] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    tree[heap[(unsigned int)heap_max]].Len = 0; /* root of the heap */

    for (h = heap_max+1; h < HEAP_SIZE; h++) {
        n = heap[h];
        bits = tree[tree[n].Dad].Len + 1;
        if (bits > (unsigned int)max_length) bits = max_length, overflow++;
        tree[n].Len = bits;
        /* We overwrite tree[n].Dad which is no longer needed */

        if (n > max_code) continue; /* not a leaf node */

        bl_count[bits]++;
        xbits = 0;
        if (n >= base) xbits = extra[n-base];
        f = tree[n].Freq;
        opt_len += (word32)f * (bits + xbits);
        if (stree) static_len += (word32)f * (stree[n].Len + xbits);
    }
    if (overflow == 0) return;

//    Trace((stderr,"\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    do {
        bits = max_length-1;
        while (bl_count[bits] == 0) bits--;
        bl_count[bits]--;      /* move one leaf down the tree */
        bl_count[bits+1] += 2; /* move one overflow item as its brother */
        bl_count[(unsigned int)max_length]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        overflow -= 2;
    } while (overflow > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    for (bits = max_length; bits != 0; bits--) {
        n = bl_count[bits];
        while (n != 0) {
            m = heap[--h];
            if (m > max_code) continue;
            if (tree[m].Len != (unsigned) bits) {
//                Trace((stderr,"code %d bits %d->%d\n", m, tree[m].Len, bits));
                opt_len += ((long)bits-(long)tree[m].Len)*(long)tree[m].Freq;
                tree[m].Len = bits;
            }
            n--;
        }
    }
}

/*
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
void CodeTree::gen_codes (ct_data *tree, int max_code)
{
    word16 next_code[MAX_BITS+1]; /* next code value for each bit length */
    word16 code = 0;              /* running code value */
    unsigned int bits;                  /* bit index */
    int n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    for (bits = 1; bits <= MAX_BITS; bits++) {
        next_code[bits] = code = (code + bl_count[bits-1]) << 1;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    assert (code + bl_count[MAX_BITS]-1 == (1<<MAX_BITS)-1);
//    Tracev((stderr,"\ngen_codes: max_code %d ", max_code));

    for (n = 0;  n <= max_code; n++) {
        int len = tree[n].Len;
        if (len == 0) continue;
        /* Now reverse the bits */
        tree[n].Code = reverse(next_code[len]++, len);

//        Tracec(tree != static_ltree, (stderr,"\nn %3d %c l %2d c %4x (%x) ",
//             n, (isgraph(n) ? n : ' '), len, tree[n].Code, next_code[len]-1));
    }
}

/*
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
void CodeTree::build_tree(tree_desc *desc)
{
    ct_data *tree   = desc->dyn_tree;
    const ct_data *stree  = desc->static_tree;
    int elems            = desc->elems;
    int n, m;          /* iterate over heap elements */
    int max_code = -1; /* largest code with non zero frequency */
    int node = elems;  /* next internal node of the tree */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n+1].
     * heap[0] is not used.
     */
    heap_len = 0, heap_max = HEAP_SIZE;

    for (n = 0; n < elems; n++) {
        if (tree[n].Freq != 0) {
            heap[(unsigned int)++heap_len] = max_code = n;
            depth[(unsigned int)n] = 0;
        } else {
            tree[n].Len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    while (heap_len < 2) {
        int _new = heap[(unsigned int)++heap_len] = (max_code < 2 ? ++max_code : 0);
        tree[_new].Freq = 1;
        depth[(unsigned int)_new] = 0;
        opt_len--; if (stree) static_len -= stree[_new].Len;
        /* new is 0 or 1 so it does not have extra bits */
    }
    desc->max_code = max_code;

    /* The elements heap[heap_len/2+1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    for (n = heap_len/2; n >= 1; n--) pqdownheap(tree, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    do {
        pqremove(tree, n);   /* n = node of least frequency */
        m = heap[(unsigned int)SMALLEST];  /* m = node of next least frequency */

        heap[(unsigned int)--heap_max] = n; /* keep the nodes sorted by frequency */
        heap[(unsigned int)--heap_max] = m;

        /* Create a new node father of n and m */
        tree[node].Freq = tree[n].Freq + tree[m].Freq;
        depth[(unsigned int)node] = (uint8_t) (MAX(depth[(unsigned int)n], depth[(unsigned int)m]) + 1);
        tree[n].Dad = tree[m].Dad = node;
#ifdef DUMP_BL_TREE
        if (tree == bl_tree) {
            fprintf(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
                    node, tree[node].Freq, n, tree[n].Freq, m, tree[m].Freq);
        }
#endif
        /* and insert the new node in the heap */
        heap[(unsigned int)SMALLEST] = node++;
        pqdownheap(tree, SMALLEST);

    } while (heap_len >= 2);

    heap[(unsigned int)--heap_max] = heap[(unsigned int)SMALLEST];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen(desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes (tree, max_code);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree. Updates opt_len to take into account the repeat
 * counts. (The contribution of the bit length codes will be added later
 * during the construction of bl_tree.)
 */
void CodeTree::scan_tree (ct_data *tree, int max_code)
{
    int n;                     /* iterates over all tree elements */
    int prevlen = -1;          /* last emitted length */
    int curlen;                /* length of current code */
    int nextlen = tree[0].Len; /* length of next code */
    int count = 0;             /* repeat count of the current code */
    int max_count = 7;         /* max repeat count */
    int min_count = 4;         /* min repeat count */

    if (nextlen == 0) max_count = 138, min_count = 3;
    tree[max_code+1].Len = (word16)-1; /* guard */

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = tree[n+1].Len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            bl_tree[(unsigned int)curlen].Freq += count;
        } else if (curlen != 0) {
            if (curlen != prevlen) bl_tree[(unsigned int)curlen].Freq++;
            bl_tree[(unsigned int)REP_3_6].Freq++;
        } else if (count <= 10) {
            bl_tree[(unsigned int)REPZ_3_10].Freq++;
        } else {
            bl_tree[(unsigned int)REPZ_11_138].Freq++;
        }
        count = 0; prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
        } else {
            max_count = 7, min_count = 4;
        }
    }
}

/* Send a literal or distance tree in compressed form,
   using the codes in bl_tree. */
void CodeTree::send_tree (ct_data *tree, int max_code)
{
   int n;                     /* iterates over all tree elements */
   int prevlen = -1;          /* last emitted length */
   int curlen;                /* length of current code */
   int nextlen = tree[0].Len; /* length of next code */
   int count = 0;             /* repeat count of the current code */
   int max_count = 7;         /* max repeat count */
   int min_count = 4;         /* min repeat count */

   /* tree[max_code+1].Len = -1; */  /* guard already set */
   if (nextlen == 0) max_count = 138, min_count = 3;

   for (n = 0; n <= max_code; n++) {
      curlen = nextlen; nextlen = tree[n+1].Len;
      if (++count < max_count && curlen == nextlen) {
         continue;
      } else if (count < min_count) {
         do {
            send_code(curlen, bl_tree);
         } while (--count != 0);
      } else if (curlen != 0) {
         if (curlen != prevlen) {
            send_code(curlen, bl_tree);
            count--;
         }
         assert(count >= 3 && count <= 6);
         send_code(REP_3_6, bl_tree);
         send_bits(count-3, 2);
      } else if (count <= 10) {
         send_code(REPZ_3_10, bl_tree);
         send_bits(count-3, 3);
      } else {
         send_code(REPZ_11_138, bl_tree);
         send_bits(count-11, 7);
      }
      count = 0; prevlen = curlen;
      if (nextlen == 0) {
         max_count = 138, min_count = 3;
      } else if (curlen == nextlen) {
         max_count = 6, min_count = 3;
      } else {
         max_count = 7, min_count = 4;
      }
   }
}

/* Construct the Huffman tree for the bit lengths and return the index in
   bl_order of the last bit length code to send. */
int CodeTree::build_bl_tree()
{
    int max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    scan_tree(dyn_ltree, l_desc.max_code);
    scan_tree(dyn_dtree, d_desc.max_code);

    /* Build the bit length tree: */
    build_tree(&bl_desc);
    /* opt_len now includes the length of the tree representations, except
     * the lengths of the bit lengths codes and the 5+5+4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    for (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        if (bl_tree[(unsigned int)bl_order[max_blindex]].Len != 0) break;
    }
    /* Update opt_len to include the bit length tree and counts */
    opt_len += 3*(max_blindex+1) + 5+5+4;
//    Tracev((stderr, "\ndyn trees: dyn %ld, stat %ld", opt_len, static_len));

    return max_blindex;
}

/* Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4. */
void CodeTree::send_all_trees(int lcodes, int dcodes, int blcodes)
{
   int rank;                    /* index in bl_order */

   assert (lcodes >= 257 && dcodes >= 1 && blcodes >= 4);
   assert (lcodes <= L_CODES && dcodes <= D_CODES && blcodes <= BL_CODES);
//   Tracev((stderr, "\nbl counts: "));
   send_bits(lcodes-257, 5);
   /* not +255 as stated in appnote.txt 1.93a or -256 in 2.04c */
   send_bits(dcodes-1,   5);
   /* not -3 as stated in appnote.txt */
   send_bits(blcodes-4,  4);
   for (rank = 0; rank < blcodes; rank++) {
//      Tracev((stderr, "\nbl code %2d ", bl_order[rank]));
      send_bits(bl_tree[(unsigned int)bl_order[rank]].Len, 3);
   }
//   Tracev((stderr, "\nbl tree: sent %ld", bits_sent));

   /* send the literal tree */
   send_tree(dyn_ltree, lcodes-1);
//   Tracev((stderr, "\nlit tree: sent %ld", bits_sent));

   /* send the distance tree */
   send_tree(dyn_dtree, dcodes-1);
//   Tracev((stderr, "\ndist tree: sent %ld", bits_sent));
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and output the encoded block to the zip file. This function
 * returns the total compressed length for the file so far.
 */
word32 CodeTree::flush_block(uint8_t *buf, word32 stored_len, int eof)
{
   word32 opt_lenb, static_lenb; /* opt_len and static_len in bytes */
   int max_blindex;  /* index of last bit length code of non zero freq */

   flag_buf[last_flags] = flags; /* Save the flags for the last 8 items */

   /* Construct the literal and distance trees */
   build_tree(&l_desc);
//   Tracev((stderr, "\nlit data: dyn %ld, stat %ld", opt_len, static_len));

   build_tree(&d_desc);
//   Tracev((stderr, "\ndist data: dyn %ld, stat %ld", opt_len, static_len));
   /* At this point, opt_len and static_len are the total bit lengths of
    * the compressed block data, excluding the tree representations.
    */

   /* Build the bit length tree for the above two trees, and get the index
    * in bl_order of the last bit length code to send.
    */
   max_blindex = build_bl_tree();

   /* Determine the best encoding. Compute first the block length in bytes */
   opt_lenb = (opt_len+3+7)>>3;
   static_lenb = (static_len+3+7)>>3;
   input_len += stored_len; /* for debugging only */

//   Trace((stderr, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u dist %u ",
//           opt_lenb, opt_len, static_lenb, static_len, stored_len,
//           last_lit, last_dist));

   if (static_lenb <= opt_lenb) opt_lenb = static_lenb;

#ifdef FORCE_METHOD
   if (level == 2 && buf) /* force stored block */
#else
   if (stored_len+4 <= opt_lenb && buf) /* 4: two words for the lengths */
#endif
   {
       /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
        * Otherwise we can't have processed more than WSIZE input bytes since
        * the last block flush, because compression would have been
        * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
        * transform a block into a stored block.
        */
       /* send block type */
       send_bits((STORED_BLOCK<<1)+eof, 3);
       compressed_len = (compressed_len + 3 + 7) & ~7L;
       compressed_len += (stored_len + 4) << 3;
       /* with header */
       copy_block(buf, (unsigned)stored_len, 1);
   }
#ifdef FORCE_METHOD
   else if (level == 3) /* force static trees */
#else
   else if (static_lenb == opt_lenb)
#endif
   {
       send_bits((STATIC_TREES<<1)+eof, 3);
       compress_block(static_ltree,static_dtree);
       compressed_len += 3 + static_len;
   } else {
       send_bits((DYN_TREES<<1)+eof, 3);
       send_all_trees(l_desc.max_code+1, d_desc.max_code+1, max_blindex+1);
       compress_block(dyn_ltree,dyn_dtree);
       compressed_len += 3 + opt_len;
   }
//   assert (compressed_len == bits_sent);
   init_block();

   if (eof) {
//      assert (input_len == isize);
      bi_windup();
      compressed_len += 7;  /* align on byte boundary */
   }
//   Tracev((stderr,"\ncomprlen %lu(%lu) ", compressed_len>>3,
//          compressed_len-7*eof));

   return compressed_len >> 3;
}

/* Save the match info and tally the frequency counts.
   Return true if the current block must be flushed. */
int CodeTree::ct_tally (int dist, int lc)
{
   l_buf[last_lit++] = (uint8_t)lc;
   if (dist == 0) {
      /* lc is the unmatched char */
      dyn_ltree[(unsigned int)lc].Freq++;
   } else {
      /* Here, lc is the match length - MIN_MATCH */
      dist--;             /* dist = match distance - 1 */
      assert((word16)dist < (word16)MAX_DIST &&
             (word16)lc <= (word16)(MAX_MATCH-MIN_MATCH) &&
             (word16)d_code(dist) < (word16)D_CODES);

      dyn_ltree[(unsigned int)length_code[(unsigned int)lc]+LITERALS+1].Freq++;
      dyn_dtree[(unsigned int)d_code(dist)].Freq++;

      d_buf[last_dist++] = dist;
      flags |= flag_bit;
   }
   flag_bit <<= 1;

   /* Output the flags if they fill a byte: */
   if ((last_lit & 7) == 0) {
      flag_buf[last_flags++] = flags;
      flags = 0, flag_bit = 1;
   }
   /* Try to guess if it is profitable to stop the current block here */
   if (deflate_level > 2 && (last_lit & 0xfff) == 0) {
      /* Compute an upper bound for the compressed length */
      word32 out_length = (word32)last_lit*8L;
      word32 in_length = (word32)strstart-block_start;
      unsigned int dcode;
      for (dcode = 0; dcode < D_CODES; dcode++) {
         out_length += (word32)dyn_dtree[dcode].Freq*(5L+extra_dbits[dcode]);
      }
      out_length >>= 3;
//      Trace((stderr,"\nlast_lit %u, last_dist %u, in %ld, out ~%ld(%ld%%) ",
//            last_lit, last_dist, in_length, out_length,
//            100L - out_length*100L/in_length));
       if (last_dist < last_lit/2 && out_length < in_length/2) return 1;
   }
   return (last_lit == LIT_BUFSIZE-1 || last_dist == (unsigned)DIST_BUFSIZE);
   /* We avoid equality with LIT_BUFSIZE because of wraparound at 64K
    * on 16 bit machines and because stored blocks are restricted to
    * 64K-1 bytes. */
}

/* Send the block data compressed using the given Huffman trees */
void CodeTree::compress_block(ct_data *ltree, ct_data *dtree)
{
   unsigned dist;      /* distance of matched string */
   int lc;             /* match length or unmatched char (if dist == 0) */
   unsigned lx = 0;    /* running index in l_buf */
   unsigned dx = 0;    /* running index in d_buf */
   unsigned fx = 0;    /* running index in flag_buf */
   uint8_t flag = 0;       /* current flags */
   unsigned code;      /* the code to send */
   int extra;          /* number of extra bits to send */

   if (last_lit != 0)
      do {
         if ((lx & 7) == 0) flag = flag_buf[fx++];
         lc = l_buf[lx++];
         if ((flag & 1) == 0) {
            /* send a literal byte */
            send_code(lc, ltree);
//            Tracecv(isgraph(lc), (stderr," '%c' ", lc));
         } else {
            /* Here, lc is the match length - MIN_MATCH */
            code = length_code[(unsigned int)lc];
            /* send the length code */
            send_code(code+LITERALS+1, ltree);
            if ((extra = extra_lbits[code]) != 0) {
               lc -= base_length[code];
               /* send the extra length bits */
               send_bits(lc, extra);
            }
            dist = d_buf[dx++];
            /* Here, dist is the match distance - 1 */
            code = d_code(dist);
            assert(code < D_CODES);

            /* send the distance code */
            send_code(code, dtree);
            if ((extra = extra_dbits[code]) != 0) {
               dist -= base_dist[code];
               /* send the extra distance bits */
               send_bits(dist, extra);
             }
         } /* literal or match pair ? */
         flag >>= 1;
     } while (lx < last_lit);

   send_code(END_BLOCK, ltree);
}
