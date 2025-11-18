
#include <stdlib.h>

#include "convert_base64.h"

#ifndef MIN_BLOCK_LENGTH
#define MIN_BLOCK_LENGTH (4*12)
#endif

#if (CONVERT_BASE64_BYTR_ORDER == CONVERT_BASE64_BYTR_ORDER_LITTLE_ENDIAN)

const char* bas64_all_codepoints = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char* bas64_codepoints_numbers = "0123456789+/";
const char* bas64_codepoints_numbers2 = "0123456789+/----";
const unsigned int* uintPtr = (unsigned int*)bas64_codepoints_numbers2;

unsigned int minDataBlockSizeForBlockProcessing = MIN_BLOCK_LENGTH;

char do_convert(unsigned int byteValue) {
	return bas64_all_codepoints[byteValue];
}

unsigned char do_convert_to_byte(char base64char) {	
	
	return MAP_BASE64_TO_BYTE(base64char);
	
	if(base64char < 58) {
		
		if(base64char < 48) {
			return 62 + (base64char/47);
		}
		
		return base64char + 4;
		 //+ ((100-base64char)/53)*3*(5 - ((base64char&0x4)>>2));
	}
	
	base64char -= 65;
	
	base64char -= ((base64char/26) - (base64char/52))*6;
	
	return (unsigned char)base64char;
}

char do_convert_raw_algo(unsigned int result, int algo) {
	switch(algo) {
		case 1:
			if(result < 26) {
				result += 65; //offset of 'A'
			} else if(result < 52) {
				result += 71; //offset of 'a'
			} else {
				result-=52;
				result = bas64_codepoints_numbers2[result];
				result = (((*(uintPtr + (result>>2))) >> (result&0x3))&0xFF);
			}
			return result;
		case 2:
			if(result < 52) {
				result += 65 + ((result/26)*6);
			} else {
				result-=52;
				result = bas64_codepoints_numbers2[result];
			}
			return result;
		case 3:
			if(result < 52) {
				result += 65 + ((result/26)*6);
			} else {
				result -= 52;
				result = (((*(uintPtr + (result>>2))) >> ((result&0x3)*8))&0xFF);
			}
			return result;
		case 4:
			if(result > 61) {
				return 43 + ((result & 0x1)<<2);
			}
			
			result += 65 + ((result/26)*6) - ((result/52)*81);
			return result;
		case 5:
			return MAP_BYTE_TO_BASE64_MACRO(result);
		case 6:
			return MAP_BYTE_TO_BASE64_MACRO_EX(result);
		default:
			//! Actually an error, no conversion !
			break;
	}

	return result;
}

char do_convert_raw(unsigned int byteValue) {
	return do_convert_raw_algo(byteValue, 6);	
}


unsigned char* convert_base64_to_str(void* pData, 
	unsigned int dataLength, 
	unsigned char* dst) {
				
	//TODO check input data alinment
	//Copy to aligned cache
	
	unsigned char* bytes_to_encode = (unsigned char*)pData;
	unsigned int in_len = dataLength;
	
	if(dataLength >= minDataBlockSizeForBlockProcessing) {
		unsigned int num12ByteBlocks = (int) (dataLength / 12);
		
		unsigned int* srcPtr = (unsigned int*)pData;
		unsigned int* dstPtr = (unsigned int*)dst;
		
		unsigned int fourBytes, fourBytes2;
		unsigned int result;
		unsigned int result2;
		
		in_len = dataLength - num12ByteBlocks*12;
		
		while(num12ByteBlocks > 0) {
			
			fourBytes = *srcPtr;
			
			int pt = 0;

start0:						
			//Handle 3 bytes
			result = ((fourBytes & 0xFC) >> 2) ;
			
			result = MAP_BYTE_TO_BASE64_MACRO(result);
			
			result2 = (((fourBytes & 0x3) << 4) |  ((fourBytes & 0xF000) >> 12));
					
start1:
			result2 = MAP_BYTE_TO_BASE64_MACRO(result2);
			
			if(pt > 0) {
				if(pt & 0x2) {
					goto end3;
				}
				
				goto end2;			
			}
			
			result |= (result2 )<<8;	
				
			result2 = (((fourBytes & 0xF00) >> 6) | ((fourBytes & 0xC00000) >> 20));
			pt |= 1;
			goto start1;

end2:
			result |= (result2 )<<16;
			
			result2 = (fourBytes & 0x3F0000)>>16;
			pt |= 2;
			goto start1;
			
end3:					
			result |= (result2)<<24;
			
			if( pt & 0x8) {
				if(pt & 0x10) {
					if(pt & 0x20) {
						goto endbyte4;
					}
					
					goto endbyte3;
				}
				goto endbyte2;
			}
			
			*dstPtr = result;
			
			++dstPtr;
			++srcPtr;
			////-///
			
			//2.nd 24bit block
			fourBytes2 = *srcPtr;
			fourBytes = (fourBytes >> 24) | ((fourBytes2 & 0xFFFF) << 8);

			pt = 0x8;
			goto start0;
endbyte2:
			*dstPtr = result;
			//->
			++dstPtr;
			++srcPtr;
			////-///
			
			//3.rd 24bit block
			fourBytes = fourBytes2 >> 16;
			
			fourBytes2 = *srcPtr;
			
			fourBytes |= (fourBytes2 << 16);
			
			pt = 0x10;
			goto start0;
endbyte3:			
			*dstPtr = result;
			//->
			++dstPtr;
			++srcPtr;
			////-///
			
			//4.th 24bit block
			//No additional read
			fourBytes = fourBytes2 >> 8;
			
			pt = 0x20;
			goto start0;
endbyte4:
			*dstPtr = result;
			//->
			++dstPtr;
			
			--num12ByteBlocks;
		}
		//
		bytes_to_encode = (unsigned char*)srcPtr;
		dst = (unsigned char*)dstPtr;
	}
	
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
		  char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		  char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		  char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		  char_array_4[3] = char_array_3[2] & 0x3f;

		  for(i = 0; i < 4 ; i++) {
			*dst = bas64_all_codepoints[char_array_4[i]];
			++dst;
		  }
		  i = 0;
		}
	}

	if (i) {
		for(j = i; j < 3; j++) {
			char_array_3[j] = '\0';
		}

		char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

		for (j = 0; (j < i + 1); j++) {
			*dst = bas64_all_codepoints[char_array_4[j]];	
			++dst;
		}

		while((i++ < 3)) {
			*dst = '=';
			++dst;
		}
	}
	
	return nullptr;
}

unsigned char* convert_base64_to_bytes(void* pData, 
	unsigned int dataLength, 
	unsigned char* dst) {

	unsigned char* ucPtr = (unsigned char*)pData;
	unsigned char* dstPtr = dst;
	
	int i = 0, j = 0, in_ = 0;
	int bits;
	union bytesOrUint {
		unsigned int fourBytes;
		unsigned char char_array_4[4];
	};
	
	bytesOrUint bou;
	
	unsigned char char_array_4[4], char_array_3[3];

	unsigned char currentChar = ucPtr[in_];
	int decoded = MAP_BASE64_TO_BYTE(currentChar);
	int otherBits = ~0x3F;
	
	//is_base64(encoded_string[in_])
	while (dataLength-- && ( ucPtr[in_] != '=') && 
		 (IS_BASE64_SYMBOL(currentChar)) ) {

		//(decoded >= 0 && decoded < 64)
		//char_array_4[i++] = 
		bou.char_array_4[i++] = ucPtr[in_]; 
    
		currentChar = ucPtr[in_];
		
		decoded = MAP_BASE64_TO_BYTE(currentChar);
		in_++;
    
    if (i == 4) {
		
      for (i = 0; i <4; i++) {
		bits = bou.char_array_4[i];
		
		bits = MAP_BASE64_TO_BYTE(bits);
		
        bou.char_array_4[i] = (unsigned char)bits;
	  }

		if(false) {
			char_array_3[0] = ( char_array_4[0] << 2       ) | ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) | ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) |   char_array_4[3];
		} else {
			char_array_3[0] = (bou.fourBytes & 0x3F) << 2 | (bou.fourBytes & 0x3000) >> 12;
			char_array_3[1] = (bou.fourBytes & 0xF00) >> 4 | (bou.fourBytes & 0x3C0000) >> 18;
			char_array_3[2] = (bou.fourBytes & 0x30000) >> 10 | (bou.fourBytes & 0xFF000000) >> 24;
		}

      for (i = 0; (i < 3); i++) {
        *dstPtr = char_array_3[i];
        ++dstPtr;
	  }
      i = 0;
    }
  }

  if (i) {
    for (j = 0; j < i; j++) {
		bits = char_array_4[j];
		
		bits = MAP_BASE64_TO_BYTE(bits);
		
		char_array_4[j] = (unsigned char)bits;
	}
	
    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

    for (j = 0; (j < i - 1); j++) {
		*dstPtr = char_array_3[j];
		++dstPtr;
	}
  }

	return nullptr;
}


#elif 
#if (CONVERT_BASE64_BYTR_ORDER == CONVERT_BASE64_BYTR_ORDER_BIG_ENDIAN)

unsigned char* convert_base64_to_str(void* pData, unsigned int dataLength, unsigned char* dst) {
	return nullptr;
}

unsigned char* convert_base64_to_bytes(void* pData, unsigned int dataLength, unsigned char* dst) {
	return nullptr;
}

#else

unsigned char* convert_base64_to_str(void* pData, unsigned int dataLength, unsigned char* dst) {
	return nullptr;
}

unsigned char* convert_base64_to_bytes(void* pData, unsigned int dataLength, unsigned char* dst) {
	return nullptr;
}

#endif

static int convert_base64_cacheSize = 0;
static void* convert_base64_cache_ptr = nullptr;

void convert_base64_set_cache_size(int cacheSize) {
	
	
	void* oldPtr = convert_base64_cache_ptr;
	
	convert_base64_cache_ptr = malloc(cacheSize);
	
	if(convert_base64_cache_ptr != nullptr) {
		
		convert_base64_cacheSize = cacheSize;
		
		if(oldPtr != nullptr) {
			free(oldPtr);
		}
	}
}

void convert_base64_remove_cache() {
	if(convert_base64_cache_ptr != nullptr) {
		free(convert_base64_cache_ptr);
		convert_base64_cache_ptr = nullptr;
	}
}

