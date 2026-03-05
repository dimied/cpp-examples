#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>

#define KB (1024)

#define CACHE_ALIGNMENT 64

#define CACHE_ALIGNED __attribute__((__aligned__(64)))

// Base64 character set
const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

typedef struct
{
    char data[64];

} Base64Characters;

Base64Characters base64Chars CACHE_ALIGNED = {
    data : {
        'A', 'B', 'C', 'D',
        'E', 'F', 'G', 'H',
        /**/
        'I', 'J', 'K', 'L',
        'M', 'N', 'O', 'P',
        /**/
        'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X',
        /**/
        'Y', 'Z', 'a', 'b',
        'c', 'd', 'e', 'f',
        /**/
        'g', 'h', 'i', 'j',
        'k', 'l', 'm', 'n',
        /**/
        'o', 'p', 'q', 'r',
        's', 't', 'u', 'v',
        /**/
        'w', 'x', 'y', 'z',
        '0', '1', '2', '3',
        /**/
        '4', '5', '6', '7',
        '8', '9', '+', '/'}
};
//

// Function to encode a binary buffer into Base64
char *base64_encode(const unsigned char *data, size_t input_length)
{
    size_t output_length = 4 * ((input_length + 2) / 3);
    char *encoded_data = (char *)malloc(output_length + 1);
    if (encoded_data == NULL)
    {
        return NULL; // Memory allocation failed
    }

    // Loop through the input data and encode it to Base64
    for (size_t i = 0, j = 0; i < input_length;)
    {
        // Extract three octets from input data (or use zero padding)
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;

        // Combine three octets into a 24-bit triple
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        // Encode triple as Base64 characters
        encoded_data[j++] = base64_chars[(triple >> 18) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 12) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 6) & 0x3F];
        encoded_data[j++] = base64_chars[triple & 0x3F];
    }

    // Handle padding and null-terminate the encoded string
    while (output_length > 0 && encoded_data[output_length - 1] == '=')
    {
        output_length--;
    }
    encoded_data[output_length] = '\0';

    return encoded_data;
}

// Function to decode a Base64 encoded string into binary data
unsigned char *base64_decode(const char *input, size_t *output_length)
{
    size_t input_length = strlen(input);
    if (input_length % 4 != 0)
    {
        return NULL; // Invalid Base64 input length
    }

    // Calculate the expected output length
    *output_length = (3 * input_length) / 4;
    if (input[input_length - 1] == '=')
    {
        (*output_length)--;
    }
    if (input[input_length - 2] == '=')
    {
        (*output_length)--;
    }

    // Allocate memory for the decoded data
    unsigned char *decoded_data = (unsigned char *)malloc(*output_length);
    if (decoded_data == NULL)
    {
        return NULL; // Memory allocation failed
    }

    // Initialize variables for decoding process
    size_t j = 0;
    uint32_t sextet_bits = 0;
    int sextet_count = 0;

    // Loop through the Base64 input and decode it
    for (size_t i = 0; i < input_length; i++)
    {
        // Convert Base64 character to a 6-bit value
        uint32_t base64_value = 0;
        if (input[i] == '=')
        {
            base64_value = 0;
        }
        else
        {
            const char *char_pointer = strchr(base64_chars, input[i]);
            if (char_pointer == NULL)
            {
                free(decoded_data);
                return NULL; // Invalid Base64 character
            }
            base64_value = char_pointer - base64_chars;
        }

        // Combine 6-bit values into a 24-bit sextet
        sextet_bits = (sextet_bits << 6) | base64_value;
        sextet_count++;

        // When a sextet is complete, decode it into three bytes
        if (sextet_count == 4)
        {
            decoded_data[j++] = (sextet_bits >> 16) & 0xFF;
            decoded_data[j++] = (sextet_bits >> 8) & 0xFF;
            decoded_data[j++] = sextet_bits & 0xFF;
            sextet_bits = 0;
            sextet_count = 0;
        }
    }

    return decoded_data;
}

char *my_base64_encode(
    const unsigned char *data,
    size_t input_length,
    char *encoded_data,
    size_t output_length)
{
    if (data == NULL || encoded_data == NULL || input_length == 0 || output_length < 5)
    {
        return NULL;
    }
    output_length -= 5; // 4 for result and 1 for '\0'
    size_t j = 0;
    // TODO:
    //  - is encoded_data 4 byte aligned?
    //  Loop through the input data and encode it to Base64
    for (size_t i = 0; i < input_length;)
    {
        // Extract three octets from input data (or use zero padding)
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;

        // Combine three octets into a 24-bit triple
        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        if (j <= output_length)
        {
            // Encode triple as Base64 characters
            encoded_data[j++] = base64_chars[(triple >> 18) & 0x3F];
            encoded_data[j++] = base64_chars[(triple >> 12) & 0x3F];
            encoded_data[j++] = base64_chars[(triple >> 6) & 0x3F];
            encoded_data[j++] = base64_chars[triple & 0x3F];
        }
        else
        {
            return NULL;
        }
    }
    output_length += 4;

    // Handle padding and null-terminate the encoded string
    while (j < output_length)
    {
        encoded_data[j++] = '=';
    }
    encoded_data[j] = '\0';

    return encoded_data;
}

typedef struct _base64Allocation
{
    size_t length;
    char *pData;
} base64Allocation;

#define INIT_BASE64_SLOTS (4096 / sizeof(base64Allocation))

//base64Allocation base64Allocs[INIT_BASE64_SLOTS];

#define MAX_ALLOWED_ALLOC (SIZE_MAX >> 2)

base64Allocation *allocForBase64Encoded(size_t input_length, base64Allocation *pOutput)
{
    if (pOutput != NULL)
    {
        size_t output_length = ((input_length + 2) / 3);
        if (output_length < MAX_ALLOWED_ALLOC)
        {
            char *encoded_data = (char *)malloc(output_length + 1);
            if (encoded_data == NULL)
            {
                return NULL;
            }
            pOutput->length = output_length;
            pOutput->pData = encoded_data;
            return pOutput;
        }
    }
    return NULL;
}

int main()
{
    memcpy(&base64Chars.data, base64_chars, 64);
    unsigned long long offset1 = (unsigned long long)&base64Chars;
    unsigned long long offset2 = (unsigned long long)&base64_chars[0];

    for (int i = 0; i < 64; i++)
    {
        char c = base64_chars[i];
        printf(" (%2d) %c:%3d/%2d ||", i, c, c, (i + 12) % 64);
        if ((i + 1) % 8 == 0 && i > 0)
        {
            printf("\n");
        }
    }
    printf("Offset:1: %lld | %lld\n", offset1, offset1 % 64);
    printf("Offset:2: %lld | %lld\n", offset2, offset2 % 64);
    malloc_stats();

    return 0;
}
