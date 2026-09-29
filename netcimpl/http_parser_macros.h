#ifndef HTTP_PARSER_MACROS
#define HTTP_PARSER_MACROS

#define WORD_ES(BC) ((uint64_t)' ' << BC)

#define WORD3_TO_UINT64(C1, C2, C3) ((uint64_t)C1 | ((uint64_t)C2 << 8) | ((uint64_t)C3 << 16))
#define WORD3_TO_UINT64_ES_END(C1, C2, C3) (WORD3_TO_UINT64(C1, C2, C3) | WORD_ES(24))
#define WORD4_TO_UINT64(C1, C2, C3, C4) (WORD3_TO_UINT64(C1, C2, C3) | ((uint64_t)C4) << 24)
#define WORD4_TO_UINT64_ES_END(C1, C2, C3, C4) (WORD4_TO_UINT64(C1, C2, C3, C4) | WORD_ES(32))
#define WORD5_TO_UINT64_ES_END(C1, C2, C3, C4, C5) (WORD3_TO_UINT64(C1, C2, C3) | ((uint64_t)C4 << 24) | ((uint64_t)C5 << 32) | WORD_ES(40))
#define WORD6_TO_UINT64_ES_END(C1, C2, C3, C4, C5, C6) (WORD3_TO_UINT64(C1, C2, C3) | ((uint64_t)C4 << 24) | ((uint64_t)C5 << 32) | ((uint64_t)C6 << 40) | WORD_ES(48))
#define WORD7_TO_UINT64_ES_END(C1, C2, C3, C4, C5, C6, C7) (WORD3_TO_UINT64(C1, C2, C3) | ((uint64_t)C4 << 24) | ((uint64_t)C5 << 32) | ((uint64_t)C6 << 40) | ((uint64_t)C7 << 48) | WORD_ES(56))

#define WORD8_TO_UINT64(C1, C2, C3, C4, C5, C6, C7, C8) (WORD3_TO_UINT64(C1, C2, C3) | ((uint64_t)C4 << 24) | ((uint64_t)C5 << 32) | ((uint64_t)C6 << 40) | ((uint64_t)C7 << 48) | ((uint64_t)C8 << 56))

#endif