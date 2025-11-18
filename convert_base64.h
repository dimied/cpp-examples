#ifndef __CONVERT_BASE64_HEADER_INCLUDED__
#define __CONVERT_BASE64_HEADER_INCLUDED__


#define CONVERT_BASE64_BYTR_ORDER_LITTLE_ENDIAN 1
#define CONVERT_BASE64_BYTR_ORDER_BIG_ENDIAN 2
#define CONVERT_BASE64_BYTR_ORDER_UNKNOWN 4

#define MAP_BYTE_TO_BASE64_MACRO(x) (((x > 61) ? (43 + ((x & 0x1)<<2)) : (x + 65 + ((x/26)*6) - ((x/52)*81))))

#define MAP_BYTE_TO_BASE64_MACRO_EX(x) (x + 65 + ((x/26)*6) - ((x/52)*81) - (x/62)*( 3*(5 - (x & 0x1)) ))

//58 would be enough, we take 65
#define MAP_BASE64_TO_BYTE(x) ((x < 58) ? ( (x < 48) ? (62 + (x/47)) : (x+4) ) : (x - 65 - ((x-65)/26 - (x-65)/52)*6) )

#define IS_BASE64_SYMBOL(x) ( (x == 43) || (x >= 47 && x < 58) || (x > 64 && x < 91) || (x > 96 && x < 123) )

#ifdef __BYTE_ORDER__

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

#define CONVERT_BASE64_BYTR_ORDER CONVERT_BASE64_BYTR_ORDER_LITTLE_ENDIAN
#define CONVERT_BASE64_BYTR_ORDER_STR "LITTLE"

#elif (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

#define CONVERT_BASE64_BYTR_ORDER CONVERT_BASE64_BYTR_ORDER_BIG_ENDIAN
#define CONVERT_BASE64_BYTR_ORDER_STR "BIG"

#else

#define CONVERT_BASE64_BYTR_ORDER CONVERT_BASE64_BYTR_ORDER_UNKNOWN
#define CONVERT_BASE64_BYTR_ORDER_STR "UNKNOWN"

#endif

#else

#define CONVERT_BASE64_BYTR_ORDER CONVERT_BASE64_BYTR_ORDER_UNKNOWN
#define CONVERT_BASE64_BYTR_ORDER_STR "UNKNOWN"

//Byte order end
#endif

unsigned char* convert_base64_to_str(void* pData, unsigned int dataLength, unsigned char* dst);

unsigned char* convert_base64_to_bytes(void* pData, unsigned int dataLength, unsigned char* dst);

char do_convert(unsigned int byteValue);

unsigned char do_convert_to_byte(char base64char);

char do_convert_raw(unsigned int byteValue);

void convert_base64_set_cache_size(int cacheSize);

void convert_base64_remove_cache();

//Header guard end
#endif
