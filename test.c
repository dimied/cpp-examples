#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define KB (1024)

#define CACHE_ALIGNMENT 64

#define CACHE_ALIGNED __attribute__((__aligned__(64)))

typedef struct
{
    uint64_t a;
    float b;
} SA;

typedef struct
{
    float a;
    uint8_t c;
    double b;
} SB;

union SAa
{
    SA data;
    unsigned char padded[CACHE_ALIGNMENT];
};

union SBa
{
    SB data;
    unsigned char padded[CACHE_ALIGNMENT];
};

typedef struct
{
    unsigned int size;
    char data[13];
} SC;

typedef unsigned int uint;

static union SAa sau;
static union SBa sbu;

static SA saS CACHE_ALIGNED;
static SB sbS; // CACHE_ALIGNED;

char data[KB * KB * 20] = {0};

SA data2[KB * KB];

// extern void *_end;

void printValues(void *pData, unsigned int size)
{
    if (pData == 0)
    {
        return;
    }
    char *pCData = (char *)pData;
    for (uint i = 0; i < size; i++)
    {
        printf("%d(%c) ", pCData[i], pCData[i]);
    }
    printf("\n");
}

void alignmentView(void *pSA, void *pSB, size_t sizeA, size_t sizeB)
{

    printf("Alignment:\n");

    unsigned long int limit = 1ul << 32;
    unsigned long int a1 = (unsigned long int)pSA;
    unsigned long int a2 = (unsigned long int)pSB;
    printf("Size: %ld | Aligned: %ld | Addr: %lx %p\n", sizeA, a1 % CACHE_ALIGNMENT, a1, pSA);
    printf("Size: %ld | Aligned: %ld | Addr: %lx %p\n", sizeB, a2 % CACHE_ALIGNMENT, a2, pSB);
    printf("Limit: %ld \n", a1 > limit ? (a1 - limit) : 0);
    if (a1 > limit)
    {
        unsigned long int diff = (a1 - limit);
        printf("Offset: %ld KB | %ld MB | %ld GB\n", diff >> 10, diff >> 20, diff >> 30);
    }
}

void print_heap_line()
{
    int mapsfd = open("/proc/self/maps", O_RDONLY);
    if (mapsfd == -1)
    {
        fprintf(stderr, "open() failed: %s.\n", strerror(errno));
        exit(1);
    }
    char maps[BUFSIZ] = "";
    if (read(mapsfd, maps, BUFSIZ) == -1)
    {
        fprintf(stderr, "read() failed: %s.\n", strerror(errno));
        exit(1);
    }
    if (close(mapsfd) == -1)
    {
        fprintf(stderr, "close() failed: %s.\n", strerror(errno));
        exit(1);
    }

    char *line = strtok(maps, "\n");
    while ((line = strtok(NULL, "\n")) != NULL)
    {
        if (strstr(line, "heap") != NULL)
        {
            printf("\n\nfrom /proc/self/maps:\n%s\n", line);
            return;
        }
    }
}

// Base64 character set
const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

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

#define INIT_BASE64_SLOTS (4096/sizeof(base64Allocation))

base64Allocation base64Allocs[INIT_BASE64_SLOTS];

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

#define M 3

struct BTreeNode {
    int num_keys; // Number of keys currently in the node
    int keys[M-1]; // Array of keys
    struct BTreeNode *children[M]; // Array of child pointers
    int is_leaf; // True if node is a leaf
};

// Function to create a new node
struct BTreeNode *createNode(bool is_leaf) {
    struct BTreeNode *newNode = (struct BTreeNode *)malloc(sizeof(struct BTreeNode));
    if (newNode == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    newNode->num_keys = 0;
    newNode->is_leaf = is_leaf;
    memset(newNode->children, 0, sizeof(struct BTreeNode *)*(M-1));
    /*
    for (int i = 0; i < M; i++) {
        newNode->children[i] = NULL;
    }
        */
    return newNode;
}

// Function to split a full child node
void splitChild(struct BTreeNode *parent, int index) {
    struct BTreeNode *child = parent->children[index];
    struct BTreeNode *newNode = createNode(child->is_leaf);
    
    newNode->num_keys = M/2 - 1;
    
    // Move keys and children to the new node
    for (int i = 0; i < M/2 - 1; i++) {
        newNode->keys[i] = child->keys[i + M/2];
    }
    
    if (!child->is_leaf) {
        for (int i = 0; i < M/2; i++) {
            newNode->children[i] = child->children[i + M/2];
        }
    }
    
    child->num_keys = M/2 - 1;
    
    // Shift parent's children to make space for the new node
    for (int i = parent->num_keys; i > index; i--) {
        parent->children[i + 1] = parent->children[i];
    }
    
    parent->children[index + 1] = newNode;
    
    // Shift parent's keys to insert the middle key from the child
    for (int i = parent->num_keys - 1; i >= index; i--) {
        parent->keys[i + 1] = parent->keys[i];
    }
    
    parent->keys[index] = child->keys[M/2 - 1];
    parent->num_keys++;
}

// Function to insert a key into a non-full node
void insertNonFull(struct BTreeNode *node, int key) {
    int i = node->num_keys - 1;
    
    if (node->is_leaf) {
        // Insert key into the sorted order
        while (i >= 0 && node->keys[i] > key) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->num_keys++;
    } else {
        // Find the child to insert the key
        while (i >= 0 && node->keys[i] > key) {
            i--;
        }
        i++;
        
        if (node->children[i]->num_keys == M - 1) {
            // Split child if it's full
            splitChild(node, i);
            
            // Determine which of the two children is the new one
            if (node->keys[i] < key) {
                i++;
            }
        }
        insertNonFull(node->children[i], key);
    }
}

// Function to insert a key into the B-tree
void insert(struct BTreeNode **root, int key) {
    struct BTreeNode *node = *root;

    if (node == NULL) {
        // Create a new root node
        *root = createNode(true);
        (*root)->keys[0] = key;
        (*root)->num_keys = 1;
    } else {
        if (node->num_keys == M - 1) {
            // Split the root if it's full
            struct BTreeNode *new_root = createNode(false);
            new_root->children[0] = node;
            splitChild(new_root, 0);
            *root = new_root;
        }
        insertNonFull(*root, key);
    }
}

// Function to traverse and print the B-tree in-order
void traverse(struct BTreeNode *root) {
    if (root != NULL) {
        int i;
        for (i = 0; i < root->num_keys; i++) {
            traverse(root->children[i]);
            printf("%d ", root->keys[i]);
        }
        traverse(root->children[i]);
    }
}

int main()
{
    // SA *a;
    // SB *b;

    // TODO: try to allocate and to test ranges
    char *startbreak = sbrk(0);

    printf("pid: %d\n", getpid()); // printf is allocating memory
    char *lastbreak = sbrk(0);
    printf("heap: [%p - %p]\n", startbreak, lastbreak);

    long pagesize = sysconf(_SC_PAGESIZE);
    long diff = lastbreak - startbreak;
    printf("diff: %ld (%ld pages of %ld bytes)\n", diff, diff / pagesize, pagesize);

    SC sc = {5, "Hello du da "};

    double *pd;
    float *pf;
    char *pc;
    char *pData = &data[0];

    int i = 0;

    while (i < 100)
    {
        *pData = i % 12;
        ++pData;
        data2[i].a = i;
        i++;
    }

    printf("UL(bytes): %ld\n", sizeof(unsigned long int));

    printf("SA:%ld\n", sizeof(SA));
    printf("SB:%ld\n", sizeof(SB));
    printf("PD:%ld\n", sizeof(*pd));
    printf("PF:%ld\n", sizeof(*pf));
    printf("PC:%ld\n", sizeof(*pc));
    printf("SC:%ld\n", sizeof(SC));

    printValues(&sc, sizeof(SC));

    SA *pSA = &saS;
    SB *pSB = &sbS;

    printf("Heap?: sbrk: %p\n", startbreak);
    print_heap_line();
    printf("\nStructs:\n");

    alignmentView(pSA, pSB, sizeof(*pSA), sizeof(*pSB));

    union SAa *pSAa = &sau;
    union SBa *pSBa = &sbu;

    printf("Unions:\n");

    alignmentView(pSAa, pSBa, sizeof(*pSAa), sizeof(*pSBa));

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
    printf("Offset: %ld | %ld\n", offset2, offset2%64);

    return 0;
}
