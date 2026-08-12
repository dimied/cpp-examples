#include <stdio.h>
#include <stdint.h>
#include <string.h>

const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void convert(unsigned char* in, size_t len, char* out) {

    if(sizeof(in) == 4) {
        return;
    }
    if(sizeof(in) >= 8) {
    for(size_t i =0; i < len; i++) {        
        *out = base64_chars[*in];
        ++in;
        ++out;
    }
    }
}

void convert2(unsigned char* in, size_t len, char* out) {
    const uint64_t* base64asUINT64 = (const uint64_t*)base64_chars;

    for(size_t i =0; i < len; i++) {
        unsigned char iv = *in;
        ++in;

        uint32_t shift = ((iv&0x7)<<3);

        *out = (char)((base64asUINT64[iv>>3])>>shift);
        ++out;
    }
}

int main() {


    unsigned char input[] = {12,2,65,41,3,6,4};

    char result[192] = {0};
    
    convert(input, sizeof(input), result);

    printf("A: %s\n", result);

    memset(result, 0, 192);

    convert2(input, sizeof(input), result);

    printf("B: %s\n", result);


    return 0;
}