#ifndef HTTP_PARSER_HEADER_EMPTY_DATA
#define HTTP_PARSER_HEADER_EMPTY_DATA

#ifndef HEADER_DATA_SORTED_NAMES
#define HEADER_DATA_SORTED_NAMES 1
#endif

struct
{
    /*!< pointers to header_names_lowercase_mem */
    char *header_names_lowercase[(NUM_HEADER_HANDLERS + 1)];

    uint64_t header_names_offsets8[HEADER_NAME_MAX_LEN + 1];

    uint64_t header_names_values8[HEADER_NAME_MAX_LEN + 1];

    uint64_t header_names_signatures8[HEADER_NAME_MAX_LEN + 1];

    #if HEADER_DATA_SORTED_NAMES
    uint64_t header_names_offsets8_sorted[HEADER_NAME_MAX_LEN + 1];
#endif

    /*!< lowercase header names sorted by name, 0-delimited. */
    char header_names_lowercase_mem[760];

    /*!< lowercase header names sorted by name, aligned to 8-byte boundaries, not always 0-delimited. */
    char header_names_aligned8[872];
#if HEADER_DATA_SORTED_NAMES
    char header_names_aligned8_sorted[872];
#endif
} header_comp_data;

#endif