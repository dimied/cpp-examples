#ifndef HTTP_PARSER_HEADER_EMPTY_DATA
#define HTTP_PARSER_HEADER_EMPTY_DATA

struct
{
    /*!< pointers to header_names_lowercase_mem */
    char *header_names_lowercase[(NUM_HEADER_HANDLERS + 1)];

    uint64_t header_names_offsets8[HEADER_NAME_MAX_LEN + 1];

    uint64_t header_names_values8[HEADER_NAME_MAX_LEN + 1];

    uint64_t header_names_signatures8[HEADER_NAME_MAX_LEN + 1];

    uint64_t header_names_offsets8_sorted[HEADER_NAME_MAX_LEN + 1];

    /*!< lowercase header names sorted by name, 0-delimited. */
    char header_names_lowercase_mem[760];

    /*!< lowercase header names sorted by name, aligned to 8-byte boundaries, not always 0-delimited. */
    char header_names_aligned8[872];

    char header_names_aligned8_sorted[872];
} header_comp_data;

#endif