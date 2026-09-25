#ifndef HTTP_PARSER_HEADER_EMPTY_DATA
#define HTTP_PARSER_HEADER_EMPTY_DATA


char header_names_aligned8[872];

uint64_t header_names_offsets8[HEADER_NAME_MAX_LEN+1];

uint64_t header_names_values8[HEADER_NAME_MAX_LEN+1];

uint64_t header_names_signatures8[HEADER_NAME_MAX_LEN+1];
#endif