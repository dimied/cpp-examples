
#include "b64.h"

#define lookup (65 | 71 << 8 | (-4) << 16)

#define E(X) (X + (int)(((lookup >> ((X / 26) << 3)) & 0xFF)))

#define ENC(X) \
    ((X > 61) ? (43 + ((X & 0x1) << 2)) : E(X))

void to64(unsigned char *pSrc, int length, unsigned char *pDst)
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
        resVal = ENC(byteVal);

        byteVal = val >> 12 & 0x3F;
        resVal |= (ENC(byteVal))<<8;

        byteVal = val >> 6 & 0x3F;
        resVal |= (ENC(byteVal))<<16;

        byteVal = val & 0x3F;
        resVal |= (ENC(byteVal))<<24;

        *pDstUint = resVal;
        si += 3;
        pDstUint++;
    }

    pDst = (unsigned char*)pDstUint;

    if (rest > 0)
    {
        val = *pSrc << 16;
        if (rest == 2)
        {
            val |= pSrc[si + 1] << 8;
        }
        byteVal = (val >> 18 & 0x3F);
        *pDst = ENC(byteVal);
        pDst++;

        byteVal = (val >> 12 & 0x3F);
        *pDst = ENC(byteVal);
        pDst++;

        switch (rest)
        {
        case 2:
            byteVal = (val >> 6 & 0x3F);
            *pDst = ENC(byteVal);
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

