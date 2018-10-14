#ifndef ZTREES_H
#define ZTREES_H

#include "misc.h"
#include "zbits.h"

class CodeTree : private BitOutput
{
public:
    CodeTree(int deflate_level, BufferedTransformation &outQ);

    int  ct_tally (int dist, int lc);
    word32  flush_block (uint8_t *buf, word32 stored_len, int eof);

    long block_start;       /* window offset of current block */
    unsigned int strstart; /* window offset of current string */
    const int deflate_level;

    enum {
#ifdef SMALL_MEM
        WSIZE = 0x2000,
#else
#  ifdef MEDIUM_MEM
        WSIZE = 0x4000,
#  else
        WSIZE = 0x8000,
#  endif
#endif
        MIN_MATCH = 3,
        MAX_MATCH = 258,
        MIN_LOOKAHEAD = (MAX_MATCH+MIN_MATCH+1),
        MAX_DIST = (WSIZE-MIN_LOOKAHEAD)
    };

    enum {
        MAX_BITS=15,
        MAX_BL_BITS=7,
        LENGTH_CODES=29,
        LITERALS=256,
        END_BLOCK=256,
        L_CODES=(LITERALS+1+LENGTH_CODES),
        D_CODES=30,
        BL_CODES=19,
        HEAP_SIZE=(2*L_CODES+1)
    };

private:
    enum {
        STORED_BLOCK=0,
        STATIC_TREES=1,
        DYN_TREES   =2
    };

    enum {
#ifdef SMALL_MEM
        LIT_BUFSIZE = 0x2000,
#else
#  ifdef MEDIUM_MEM
        LIT_BUFSIZE = 0x4000,
#  else
        LIT_BUFSIZE = 0x8000,
#  endif
#endif
        DIST_BUFSIZE = LIT_BUFSIZE
    };

    enum {
        REP_3_6     =16,
        REPZ_3_10   =17,
        REPZ_11_138 =18
    };

    static const int extra_lbits[LENGTH_CODES];
    static const int extra_dbits[D_CODES];
    static const int extra_blbits[BL_CODES];
    static const uint8_t bl_order[BL_CODES];

public:
    // Data structure describing a single value and its code string. */
    struct ct_data
    {
        union
        {
            word16  Freq;       /* frequency count */
            word16  Code;       /* bit string */
        };
        union
        {
            word16  Dad;        /* father node in Huffman tree */
            word16  Len;        /* length of bit string */
        };
    };

private:
    SecBlock<ct_data> dyn_ltree, dyn_dtree;

    static ct_data static_ltree[L_CODES+2];
    /* The static literal tree. Since the bit lengths are imposed, there is no
     * need for the L_CODES extra codes used during heap construction. However
     * The codes 286 and 287 are needed to build a canonical tree (see ct_init
     * below).
     */

    static ct_data static_dtree[D_CODES];
    /* The static distance tree. (Actually a trivial tree since all codes use
     * 5 bits.)
     */

    static bool streesBuilt;

    SecBlock<ct_data> bl_tree;
    /* Huffman tree for the bit lengths */

    SecBlock<word16> bl_count;
    /* number of codes at each bit length for an optimal tree */

    struct tree_desc
    {
        tree_desc(ct_data *d, ct_data *s, const int *e, int eb, int el, int ml, int mc)
            : dyn_tree(d), static_tree(s), extra_bits(e), extra_base(eb),
              elems(el), max_length(ml), max_code(mc) {}
        ct_data *const dyn_tree;      /* the dynamic tree */
        const ct_data *const static_tree;   /* corresponding static tree or NULL */
        const int     *extra_bits;    /* extra bits for each code or NULL */
        const int     extra_base;          /* base index for extra_bits */
        const int     elems;               /* max number of elements in the tree */
        const int     max_length;          /* max bit length for the codes */
        int     max_code;            /* largest code with non zero frequency */
    };

    tree_desc l_desc;
    tree_desc d_desc;
    tree_desc bl_desc;

    SecBlock<int> heap; /* heap used to build the Huffman trees */
    int heap_len;               /* number of elements in the heap */
    int heap_max;               /* element of largest frequency */
    /* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
     * The same heap array is used to build all trees.
     */

    SecByteBlock depth;
    /* Depth of each subtree used as tie breaker for trees of equal frequency */

    SecByteBlock length_code;
    /* length code for each normalized match length (0 == MIN_MATCH) */

    SecByteBlock dist_code;
    /* distance codes. The first 256 values correspond to the distances
     * 3 .. 258, the last 256 values correspond to the top 8 bits of
     * the 15 bit distances.
     */

    SecBlock<int> base_length;
    /* First normalized length for each code (0 = MIN_MATCH) */

    SecBlock<int> base_dist;
    /* First normalized distance for each code (0 = distance of 1) */

    SecByteBlock l_buf;
    SecBlock<word16> d_buf;

    SecByteBlock flag_buf;
    /* flag_buf is a bit array distinguishing literals from lengths in
     * l_buf, and thus indicating the presence or absence of a distance.
     */

    unsigned last_lit;    /* running index in l_buf */
    unsigned last_dist;   /* running index in d_buf */
    unsigned last_flags;  /* running index in flag_buf */
    uint8_t flags;            /* current flags not yet saved in flag_buf */
    uint8_t flag_bit;         /* current bit used in flags */
    /* bits are filled in flags starting at bit 0 (least significant).
     * Note: these flags are overkill in the current code since we don't
     * take advantage of DIST_BUFSIZE == LIT_BUFSIZE.
     */

    word32 opt_len;        /* bit length of current block with optimal trees */
    word32 static_len;     /* bit length of current block with static trees */
    word32 compressed_len; /* total bit length of compressed file */
    word32 input_len;      /* total byte length of input file */
    /* input_len is for debugging only since we can get it by other means. */

    void init_block     (void);
    void pqdownheap     (ct_data *tree, int k);
    void gen_bitlen     (tree_desc *desc);
    void gen_codes      (ct_data *tree, int max_code);
    void build_tree     (tree_desc *desc);
    void scan_tree      (ct_data *tree, int max_code);
    void send_tree      (ct_data *tree, int max_code);
    int  build_bl_tree  (void);
    void  send_all_trees (int lcodes, int dcodes, int blcodes);
    void  compress_block (ct_data *ltree, ct_data *dtree);
};

#endif

