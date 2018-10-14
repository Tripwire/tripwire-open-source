// zbits.cpp - modified by Wei Dai from:
// Distributed with Jean-loup Gailly's permission.

/*
 The following sorce code is derived from Info-Zip 'zip' 2.01
 distribution copyrighted by Mark Adler, Richard B. Wales,
 Jean-loup Gailly, Kai Uwe Rommel, Igor Mandrichenko and John Bush.
*/

#include "pch.h"
#include "zbits.h"
#include <assert.h>

#ifdef DEBUG
unsigned long bits_sent;   /* bit length of the compressed data */
#endif

// #define putbyte(b) outQ.Put(b)

BitOutput::BitOutput(BufferedTransformation &outQ)
    : outQ(outQ)
{
   bitbuff = 0;
   boffset = 0;
#ifdef DEBUG
   bits_sent = 0L;
#endif
}

void BitOutput::send_bits(unsigned int value, int length) /* Send a value on a given number of bits. */
{
#ifdef DEBUG
   assert(length > 0 && length <= 15);
   assert(boffset < 8);
   bits_sent += (unsigned long)length;
#endif
   bitbuff |= value << boffset;
   if ((boffset += length) >= 8) {
      outQ.Put(bitbuff);
      value >>= length - (boffset -= 8);
      if (boffset >= 8) {
         boffset -= 8;
         outQ.Put(value);
         value >>= 8;
      }
      bitbuff = value;
   }
}

/* Write out any remaining bits in an incomplete byte. */
void BitOutput::bi_windup()
{
   assert(boffset < 8);
   if (boffset) {
      outQ.Put(bitbuff);
      boffset = 0;
      bitbuff = 0;
#ifdef DEBUG
      bits_sent = (bits_sent+7) & ~7;
#endif
   }
}

void BitOutput::bi_putsh(word16 x)
{
    outQ.Put((uint8_t)x);
    outQ.Put(uint8_t(x>>8));
}

/* Copy a stored block to the zip file, storing first the length and its
   one's complement if requested. */
void BitOutput::copy_block(uint8_t *buf, unsigned int len, int header)
{
   /* align on byte boundary */
   bi_windup();

   if (header) {
        bi_putsh(len);
        bi_putsh(~len);
#ifdef DEBUG
      bits_sent += 2*16;
#endif
   }
   outQ.Put(buf, len);
#ifdef DEBUG
   bits_sent += (unsigned long)len<<3;
#endif
}
