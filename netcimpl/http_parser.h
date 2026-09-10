#ifndef HTTP_PARSER
#define HTTP_PARSER

#ifdef HTTP_PARSER_DECLARATIONS
#if HTTP_PARSER_DECLARATIONS

typedef struct RequestHttpHeaders
{
    // pairs (quality,type)
    unsigned short acceptedTypes[8];
} RequestHttpHeaders;

typedef struct UnknownHeaderResult
{
    size_t capacity;
    void **values;
    size_t size;
} UnknownHeaderResult;
#endif
#endif

#ifdef HTTP_PARSER_IMPLEMENTATION
#if HTTP_PARSER_IMPLEMENTATION

/*
   //{
    "requestHeaders": {
        "headers": [
            {
                "name": "Accept",
                "value": "text/html,application/xhtml+xml,application/xml;q=0.9,*/
/*;q=0.8"
},
{
"name": "Accept-Encoding",
"value": "gzip, deflate, br, zstd"
},
{
"name": "Accept-Language",
"value": "de,en-US;q=0.9,en;q=0.8"
},
{
"name": "Cache-Control",
"value": "no-cache"
},
{
"name": "Connection",
"value": "keep-alive"
},
{
"name": "DNT",
"value": "1"
},
{
"name": "Host",
"value": "www.dimaum.de"
},
{
"name": "Pragma",
"value": "no-cache"
},
{
"name": "Priority",
"value": "u=0, i"
},
{
"name": "Sec-Fetch-Dest",
"value": "document"
},
{
"name": "Sec-Fetch-Mode",
"value": "navigate"
},
{
"name": "Sec-Fetch-Site",
"value": "none"
},
{
"name": "Sec-Fetch-User",
"value": "?1"
},
{
"name": "Sec-GPC",
"value": "1"
},
{
"name": "Upgrade-Insecure-Requests",
"value": "1"
},
{
"name": "User-Agent",
"value": "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:155.0) Gecko/20100101 Firefox/155.0"
}
]
}
}
*/

typedef struct AcceptType
{
    int size;
    int type;
    const char *str;
} AcceptType;

#define ENTRY_AcceptType(T, S) {sizeof(S), T, S}

#define MIME_TYPE_HTML 1
#define MIME_TYPE_XHTML_XML 2
#define MIME_TYPE_XML 3

#define MIME_TYPE_IMAGE_ANY 126
#define MIME_TYPE_ANY 127

// TODOs: accept empty spaces

// to speed up decide by first char (range: '*' - 'z')
AcceptType acceptTypes[] = {
    ENTRY_AcceptType(MIME_TYPE_ANY, "*/*"),
    ENTRY_AcceptType(MIME_TYPE_XHTML_XML, "application/xhtml+xml"),
    ENTRY_AcceptType(MIME_TYPE_XML, "application/xml"),
    ENTRY_AcceptType(MIME_TYPE_HTML, "text/html"),
    ENTRY_AcceptType(MIME_TYPE_IMAGE_ANY, "image/*"),
};

#define NUM_ACCEPT_TYPES (sizeof(acceptTypes) / sizeof(acceptTypes[0]))

#define UNKNOWN_TYPE_OFFSET (128)
#define MAX_RECOGNITION_TYPES 32
#define MAX_UNKNOWN_TYPES 32

#ifndef COLLECT_UNKNOWN_TYPES
#define COLLECT_UNKNOWN_TYPES 1
#endif

#ifndef PARSE_ACCEPT_PARAM_CHECK
#define PARSE_ACCEPT_PARAM_CHECK 0
#endif

void parseAccept(char *pAccept, int len, unsigned short *pAcceptRes, int acceptResLen, UnknownHeaderResult *pU)
{
#ifdef COLLECT_UNKNOWN_TYPES
#if COLLECT_UNKNOWN_TYPES
#define DO_COLLECT_UNKNOWN_TYPES 1
#else
#define DO_COLLECT_UNKNOWN_TYPES 0
#endif
#else
#define DO_COLLECT_UNKNOWN_TYPES 0
#endif

#if PARSE_ACCEPT_PARAM_CHECK
    if (pAcceptRes == 0 || acceptResLen <= 0)
    {
        return 0;
    }
#endif

    // Accept	text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
    char *pStart = pAccept;
    char *pEnd = pAccept + len;
    char *pQ = pEnd;
    int type = 0, typesGot = 0, typeIdx = 0;
    unsigned int types[MAX_RECOGNITION_TYPES];

#if DO_COLLECT_UNKNOWN_TYPES
    int unknownTypesGot = 0;
    void *unknownTypes[MAX_UNKNOWN_TYPES];
#endif

    while (pAccept <= pEnd)
    {
        char val = *pAccept;

        if (pAccept == pEnd)
        {
            val = ',';
        }

        if (val == '\r' || val == '\n')
        {
            // should be impossible
            break;
        }

        if (val == '/')
        {
            ++type; // should happen only once for valid types
        }
        else if (val == ';')
        {
            pQ = pAccept;
        }
        else if (val == ',')
        {
            size_t diff = (pAccept - pStart);
            // at least */* and need one separator '/'
            if (type == 1 && diff > 2)
            {
                int q = 100;
                if (pQ > pAccept)
                {
                    pQ = pAccept;
                }
                else
                {
                    q = 0;
                }
                // parse type first
                diff = pQ - pStart;
                type = 0;

                if (diff < 120)
                {
                    for (size_t i = 0; i < NUM_ACCEPT_TYPES; i++)
                    {
                        if (acceptTypes[i].size == (int)diff)
                        {
                            printf("Size fit!\n");
                            if (0 == strncmp(pStart, acceptTypes[i].str, diff))
                            {
                                type = acceptTypes[i].type;
                            }
                        }
                    }

                    if (q == 0)
                    {
                        q = 50;
                        ++pQ;
                        if (*pQ == 'q' && pQ[1] == '=' && pQ[2] == '0' && pQ[3] == '.')
                        {
                            pQ += 4;

                            if (*pQ >= '0' && *pQ <= '9')
                            {
                                q = 10 * ((*pQ) - '0');
                                ++pQ;
                                if (*pQ >= '0' && *pQ <= '9')
                                {
                                    q += (*pQ) - '0';
                                    // forget third decimal
                                }
                            }
                        }
                    }
                }

                if (type != 0)
                {
                    if (typesGot < MAX_RECOGNITION_TYPES)
                    {
                        types[typesGot] = (q << 8) | type;
                        ++typesGot;
                    }
                }
                else
                {
                    if (unknownTypesGot < MAX_UNKNOWN_TYPES)
                    {
                        unknownTypes[unknownTypesGot] = pStart;
                        ++unknownTypesGot;
                    }
                }
                ++typeIdx;
            }
            type = 0;
            pStart = pAccept + 1;
            pQ = pEnd;
        }
        ++pAccept;
    }
    if (typesGot > 0)
    {
        int i = 0;
        while (i < typesGot && acceptResLen > 0)
        {
            *pAcceptRes = (unsigned short)types[i];
            ++i;
            --acceptResLen;
        }
    }

#if DO_COLLECT_UNKNOWN_TYPES
    if (unknownTypesGot > 0 && pU != 0 && pU->size < pU->capacity && pU->values != 0)
    {
        int i = 0;
        while (i < unknownTypesGot && pU->size < pU->capacity)
        {
            pU->values[pU->size] = unknownTypes[i];
            ++i;
            pU->size++;
        }
    }
#endif

#undef DO_COLLECT_UNKNOWN_TYPES
}

void parseAcceptEncoding(char *pAcceptEncoding, unsigned long *pAcceptRes)
{
    // gzip, deflate, br, zstd
    (void)(pAcceptEncoding);
    (void)(pAcceptRes);
}

void parseAcceptLanguage(char *pAcceptLanguage, unsigned long *pAcceptRes)
{
    (void)(pAcceptLanguage);
    (void)(pAcceptRes);
}

#endif
#endif
#endif