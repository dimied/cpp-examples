

int parseAcceptCC(MyServerClientConnection *pCC, char *data, int len);
int parseAcceptEncodingCC(MyServerClientConnection *pCC, char *data, int len);
int parseAcceptLanguageCC(MyServerClientConnection *pCC, char *data, int len);
int parseNotImplemented(MyServerClientConnection *pCC, char *data, int len);

#define DESCRIBE_HEADER_HANDLER(S, H) {S, H}

HeaderHandler headerHandlers[] = {
    DESCRIBE_HEADER_HANDLER("accept", parseAcceptCC),
    DESCRIBE_HEADER_HANDLER("accept-encoding", parseAcceptEncodingCC),
    DESCRIBE_HEADER_HANDLER("accept-language", parseAcceptLanguageCC),
    DESCRIBE_HEADER_HANDLER("access-control-request-headers", parseAcceptLanguageCC),
    DESCRIBE_HEADER_HANDLER("access-control-request-method", parseAcceptLanguageCC),
    DESCRIBE_HEADER_HANDLER("alt-used", parseAcceptLanguageCC),
    DESCRIBE_HEADER_HANDLER("authorization", parseAcceptLanguageCC),
    // HH("Available-Dictionary", parseAcceptLanguageCC),
    DESCRIBE_HEADER_HANDLER("cache-control", parseAcceptLanguageCC),
    DESCRIBE_HEADER_HANDLER("connection", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("content-digest", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("cookie", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("date", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("device-memory", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("dictionary-id", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("dnt", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("downlink", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("dpr", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("early-data", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("ect", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("etag", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("expect", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("forwarded", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("from", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("idempotency-key", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("if-match", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("if-modified-since", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("if-none-match", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("if-range", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("if-unmodified-since", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("keep-alive", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("max-forwards", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("origin", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("pragma", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("prefer", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("priority", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("proxy-authorization", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("range", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("referer", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("repr-digest", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("rtt", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("save-data", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("service-worker", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("service-worker-navigation-preload", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("te", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("trailer", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("transfer-encoding", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("upgrade", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("upgrade-insecure-requests", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("user-agent", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("via", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("viewport-width", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("want-content-digest", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("want-repr-digest", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("width", parseNotImplemented),
};

#define NUM_HEADER_HANDLERS (sizeof(headerHandlers) / sizeof(headerHandlers[0]))

// RESPONSE-headers
// Accept-CH, Accept-Patch, Accept-Post, Accept-Ranges, ...
//

HeaderHandler headerHandlersSec[] = {
    DESCRIBE_HEADER_HANDLER("sec-fetch-dest", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-fetch-mode", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-fetch-site", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-fetch-user", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-gpc", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-purpose", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-webSocket-extensions", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-webSocket-key", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-websocket-protocol", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-websocket-version", parseNotImplemented),
};

#undef DESCRIBE_HEADER_HANDLER

#define MAX_RECOGNITION_TYPES 32
#define MAX_UNKNOWN_TYPES 32

#ifndef COLLECT_UNKNOWN_TYPES
#define COLLECT_UNKNOWN_TYPES 1
#endif

#ifndef PARSE_ACCEPT_PARAM_CHECK
#define PARSE_ACCEPT_PARAM_CHECK 0
#endif

//#define UNKNOWN_TYPE_OFFSET (128)

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
    char *pStart = pAccept;
    char *pEnd = pAccept + len;
    char *pQ = pEnd;
    unsigned int type = 0;
    int typesGot = 0, typeIdx = 0;
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
            long int diff = (pAccept - pStart);
            // at least 'asterisk/asterisk' and need one separator '/'
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
                    // for (int i = 0; i < (int)NUM_ACCEPT_TYPES; i++)

                    {
                        int i;
                        for (i = 0; i < (int)NUM_ACCEPT_TYPES; i++)
                        {
                            if (acceptTypes[i].size == (unsigned int)diff)
                            {
                                if (0 == strncmp(pStart, acceptTypes[i].str, (size_t)diff))
                                {
                                    type = acceptTypes[i].type;
                                }
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
                                q = 10 * (int)((*pQ) - '0');
                                ++pQ;
                                if (*pQ >= '0' && *pQ <= '9')
                                {
                                    q += (int)((*pQ) - '0');
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
                        types[typesGot] = ((unsigned)q << 8) | type;
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
            pAcceptRes[i] = (unsigned short)types[i];
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

int parseAcceptCC(MyServerClientConnection *pCC, char *data, int len)
{
    (void)(pCC);
    (void)(data);
    (void)(len);
    return 0;
}

int parseAcceptEncodingCC(MyServerClientConnection *pCC, char *data, int len)
{
    (void)(pCC);
    (void)(data);
    (void)(len);
    return 0;
}

int parseAcceptLanguageCC(MyServerClientConnection *pCC, char *data, int len)
{
    (void)(pCC);
    (void)(data);
    (void)(len);
    return 0;
}

int parseNotImplemented(MyServerClientConnection *pCC, char *data, int len)
{
    (void)(pCC);
    (void)(data);
    (void)(len);
    return 0;
}