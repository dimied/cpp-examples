#ifndef IP_UTIL
#define IP_UTIL

#include <string.h>

#define PARSE_IP_RESULT_ARG_ERROR 1
#define PARSE_IP_RESULT_PARSE_ERROR 2
#define PARSE_IP_RESULT_PARSE_ERROR_INVALID_CHARACTER 3
#define PARSE_IP_IPV4 0x10
#define PARSE_IP_IPV6 0x20
#define PARSE_IP_STORED 0x8
#define PARSE_IP_IPV4_STORED (PARSE_IP_IPV4 | PARSE_IP_STORED)
#define PARSE_IP_IPV6_STORED (PARSE_IP_IPV6 | PARSE_IP_STORED)

#ifndef IP_UTIL_IPV4_STR
#define IP_UTIL_IPV4_STR "IPv4"
#endif

#ifndef IP_UTIL_IPV6_STR
#define IP_UTIL_IPV6_STR "IPv6"
#endif

const char *parse_ip_errors[] = {"Argument error.", "Parse error.", "Parse error, invalid character."};
const struct
{
    int code;
    const char *text;
} results[2] = {
    {PARSE_IP_IPV4, IP_UTIL_IPV4_STR},
    {PARSE_IP_IPV6, IP_UTIL_IPV6_STR}};

const char *parse_ip_error(int error_code)
{
    if (error_code >= 1 && error_code <= 3)
    {
        return parse_ip_errors[error_code - 1];
    }
    return (char *)0;
}

const char *parse_ip_success(int return_code)
{
    for (size_t i = 0; i < 2; i++)
    {
        if (results[i].code == return_code)
        {
            return results[i].text;
        }
    }
    return (char *)0;
}

#define VALID_MASK_SHIFT(SRC) (0x1ul << (SRC - '.'))

#define PARSE_IP_VALID_MASK0 (VALID_MASK_SHIFT('.') | VALID_MASK_SHIFT(':'))
#define PARSE_IP_VALID_MASK1 (VALID_MASK_SHIFT('0') | VALID_MASK_SHIFT('1') | VALID_MASK_SHIFT('2') | VALID_MASK_SHIFT('3') | VALID_MASK_SHIFT('4') | VALID_MASK_SHIFT('5') | VALID_MASK_SHIFT('6') | VALID_MASK_SHIFT('7') | VALID_MASK_SHIFT('8') | VALID_MASK_SHIFT('9'))
#define PARSE_IP_VALID_MASK2 (VALID_MASK_SHIFT('A') | VALID_MASK_SHIFT('B') | VALID_MASK_SHIFT('C') | VALID_MASK_SHIFT('D') | VALID_MASK_SHIFT('E') | VALID_MASK_SHIFT('F'))
#define PARSE_IP_VALID_MASK3 (VALID_MASK_SHIFT('a') | VALID_MASK_SHIFT('b') | VALID_MASK_SHIFT('c') | VALID_MASK_SHIFT('d') | VALID_MASK_SHIFT('e') | VALID_MASK_SHIFT('f'))

#define PARSE_IP_VALID_MASK (PARSE_IP_VALID_MASK0 | PARSE_IP_VALID_MASK1 | PARSE_IP_VALID_MASK2 | PARSE_IP_VALID_MASK3)

int parse_ip(const char *ipString, unsigned char *pResult)
{
    if (ipString != 0)
    {
#if 0
        if (pResult == 0)
        {
            pResult = &res[0];
        }
#endif
        const char *pData = ipString;
        size_t len = 0, numSeparators = 0;

        while (*pData != 0)
        {
            ++len;
            char val = *pData;
            if (len > 40)
            {
                return PARSE_IP_RESULT_PARSE_ERROR;
            }
            if ((val < '.') || (val > 'f'))
            {
                return PARSE_IP_RESULT_PARSE_ERROR_INVALID_CHARACTER;
            }
            val -= '.';
            const uint64_t checkVal = 0x1ul << val;

            if ((checkVal & PARSE_IP_VALID_MASK) == 0)
            {
                return PARSE_IP_RESULT_PARSE_ERROR_INVALID_CHARACTER;
            }

            // for '.'
            numSeparators += checkVal & 0x1;
            // for ':'
            numSeparators += ((checkVal >> (':' - '.')) & 0x1) << 8;

            ++pData;
        }

        if (len < 3)
        {
            return PARSE_IP_RESULT_PARSE_ERROR;
        }

        unsigned char res[16];

        if (len == 3)
        {
            printf("LEN:3: %p\n", pResult);
            if (ipString[0] == ':' && ipString[1] == ':')
            {
                const char val = ipString[2];
                if (val >= '0' && val <= '9')
                {
                    if (pResult == 0)
                    {
                        return PARSE_IP_IPV6;
                    }
                    memset(res, 0, 16);
                    res[15] = val - '0';
                    memcpy(pResult, res, 16);
                    return PARSE_IP_IPV6_STORED;
                }
                return PARSE_IP_RESULT_PARSE_ERROR;
            }
        }
        len = numSeparators & 0xFF;
        if (len > 0)
        {
            if (len != 3)
            {
                return PARSE_IP_RESULT_PARSE_ERROR;
            }
            else
            {
                len = pData - ipString;
                printf("Parse IPV4\n");
                if (len < 7)
                {
                    return PARSE_IP_RESULT_PARSE_ERROR;
                }
                --pData;

                int num = -1, resIdx = 15;

                char val = *pData;
                if (val >= '0' && val <= '9')
                {
                    num = (val - '0');
                }
                else
                {
                    return PARSE_IP_RESULT_PARSE_ERROR;
                }

                --pData;
                int ipv6 = 0;
                int mIdx = 0;

#define MULS (10 | (100 << 10) | (1000 << 20))

                while (pData >= ipString)
                {
                    char val = *pData;
                    // printf("V: %c", val);
                    if (val == '.')
                    {
                        if (pData[1] == '.')
                        {
                            return PARSE_IP_RESULT_PARSE_ERROR;
                        }
                        res[resIdx] = num;
                        resIdx--;
                        num = -1;
                    }
                    else if (val >= '0' && val <= '9')
                    {
                        if (num < 0)
                        {
                            num = (val - '0');
                            mIdx = 0;
                        }
                        else
                        {
                            // printf("|VIN: %d (>>%d, *%d)", num, mIdx, ((MULS >> mIdx) & 0x3FF));
                            num += (val - '0') * ((MULS >> mIdx) & 0x3FF);
                            // printf("|VOUT: %d", num);
                            mIdx += 10;
                            if (num > 255)
                            {
                                return PARSE_IP_RESULT_PARSE_ERROR;
                            }
                        }
                    }
                    else
                    {
                        if (val == ':')
                        {
#if 0
                            if (num < 0 || num > 255 || resIdx != 12)
                            {
                                return PARSE_IP_RESULT_PARSE_ERROR;
                            }
#endif
                            // res[resIdx] = num;
                            // resIdx--;
                            --pData;
                            ++ipv6;
                            break;
                        }
                        return PARSE_IP_RESULT_PARSE_ERROR;
                    }
                    // printf("\n");
                    --pData;
                }

                if (num < 0 || num > 255 || resIdx != 12)
                {
                    return PARSE_IP_RESULT_PARSE_ERROR;
                }
                res[resIdx] = num;
                resIdx--;

                if (ipv6 != 0)
                {
                    printf("?V6\n");
                    if (pData < ipString || (*pData != ':'))
                    {
                        return PARSE_IP_RESULT_PARSE_ERROR;
                    }
                    else
                    {
                        // parse as much as possible left to right
                        const char *pV6 = ipString;
                        num = 0;
                        resIdx = 0;

                        while (pV6 <= pData)
                        {
                            char val = *pV6;
                            if (val == ':')
                            {
                                res[resIdx] = num >> 16;
                                ++resIdx;

                                if (resIdx >= 12)
                                {
                                    return PARSE_IP_RESULT_PARSE_ERROR;
                                }
                                res[resIdx] = num & 0xFF;
                                ++resIdx;
                                num = 0;
                            }
                            else if (val >= 'a')
                            {
                                val -= ('a' - 10);
                            }
                            else if (val >= 'A')
                            {
                                val -= ('A' - 10);
                            }
                            else
                            {
                                val -= '0';
                            }
                            num = (num << 8) | val;
                            if (num > 0xFFFF)
                            {
                                return PARSE_IP_RESULT_PARSE_ERROR;
                            }
                            ++pV6;
                        }
                        while (resIdx < 12)
                        {
                            res[resIdx] = 0;
                            ++resIdx;
                        }

                        printf("???\n");
                    }

                    int val1 = res[0] << 16 | res[1];
                    int val2 = res[2] << 16 | res[3];
                    int val3 = res[4] << 16 | res[5];
                    int val4 = res[6] << 16 | res[7];
                    int val5 = res[8] << 16 | res[9];
                    int val6 = res[10] << 16 | res[11];
                    printf("ADDR: %x:%x:%x:%x:%x:%x:%d.%d.%d.%d\n", val1, val2, val3, val4, val5, val6, res[12], res[13], res[14], res[15]);
                    if (pResult == 0)
                    {
                        return PARSE_IP_IPV6;
                    }
                    memcpy(pResult, &res[12], 4);
                    return PARSE_IP_IPV6_STORED;
                }
                else
                {
                    printf("ADDR: %d.%d.%d.%d\n", res[12], res[13], res[14], res[15]);
                    if (pResult == 0)
                    {
                        return PARSE_IP_IPV4;
                    }
                    memcpy(pResult, &res[12], 4);
                    return PARSE_IP_IPV4_STORED;
                }

                // parse (mapped) IPv4
            }
        }

        printf("CHECK IPv6\n");
        numSeparators >>= 8;

        if (numSeparators < 2 || numSeparators > 8)
        {
            return PARSE_IP_RESULT_PARSE_ERROR;
        }
        return PARSE_IP_RESULT_PARSE_ERROR;

        return PARSE_IP_RESULT_PARSE_ERROR;
    }
    return PARSE_IP_RESULT_ARG_ERROR;
}

#endif
