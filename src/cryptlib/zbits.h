#ifndef ZBITS_H
#define ZBITS_H

#include "cryptlib.h"

class BitOutput
{
public:
    BitOutput(BufferedTransformation &outQ);

    void send_bits  (unsigned value, int length);
    void bi_windup  (void);
    void bi_putsh   (unsigned short);
    void copy_block (byte *buf, unsigned len, int header);

private:
    BufferedTransformation &outQ;
    unsigned bitbuff;
    int boffset;
};

#endif

