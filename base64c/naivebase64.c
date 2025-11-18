

const char* encodeStd = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void naivebase64(unsigned char *pSrc, int length, unsigned char *pDst)
{
    int rest = length & 0x3;
    int nlength = length - rest;

    unsigned int val, resVal;
    int byteVal;
    unsigned int *pDstUint = (unsigned int *)pDst;
    int si;

    for (si = 0; si < nlength; si++)
    {
        val = pSrc[si] << 16 | pSrc[si + 1] << 8 | pSrc[si + 2];

        byteVal = val >> 18 & 0x3F;
        *pDst = encodeStd[byteVal];
        pDst++;

        byteVal = val >> 12 & 0x3F;
        *pDst = encodeStd[byteVal];
        pDst++;

        byteVal = val >> 6 & 0x3F;
        *pDst= encodeStd[byteVal];
        pDst++;

        byteVal = val & 0x3F;
        *pDst= encodeStd[byteVal];
        pDst++;

        si += 3;
    }


    if (rest > 0)
    {
        val = *pSrc << 16;
        if (rest == 2)
        {
            val |= pSrc[si + 1] << 8;
        }
        byteVal = (val >> 18 & 0x3F);
        *pDst = encodeStd[byteVal];
        pDst++;

        byteVal = (val >> 12 & 0x3F);
        *pDst = encodeStd[byteVal];
        pDst++;

        switch (rest)
        {
        case 2:
            byteVal = (val >> 6 & 0x3F);
            *pDst = encodeStd[byteVal];
            pDst++;
            *pDst = '=';
        case 1:
            *pDst = '=';
            pDst++;
            *pDst = '=';
            pDst++;
        }
    }
}

