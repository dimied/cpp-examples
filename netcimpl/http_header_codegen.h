#ifndef HTTP_PARSER_HEADER_CODEGEN
#define HTTP_PARSER_HEADER_CODEGEN

#define DECL_ENTRY(x) {x, 0, 0}
#define DECL_ENTRY_SYN(x, syn) {x, 0, syn}
#define DECL_ENTRY_RESP(x) {x, 1, 0}
#define DECL_ENTRY_RESP_SYN(x, syn) {x, 1, syn}

struct trp
{
    char *name;
    int also_response;
    char *synonym;
};

static struct trp codegen_header_constants[] = {
    DECL_ENTRY_RESP("ACCEPT"),
    DECL_ENTRY("ACCEPT_ENCODING"),
    DECL_ENTRY("ACCEPT_LANGUAGE"),
    DECL_ENTRY("ACCESS_CONTROL_REQUEST_HEADERS"),
    /**/
    DECL_ENTRY("ACCESS_CONTROL_REQUEST_METHOD"),
    DECL_ENTRY("ALT_USED"),
    DECL_ENTRY("AUTHORIZATION"),
    DECL_ENTRY("AVAILABLE_DICTIONARY"),
    /**/
    DECL_ENTRY_RESP("CACHE_CONTROL"),
    DECL_ENTRY("CONNECTION"),
    DECL_ENTRY_RESP("CONTENT_DIGEST"),
    DECL_ENTRY_RESP("CONTENT_DISPOSITION"),
    /**/
    DECL_ENTRY_RESP("CONTENT_ENCODING"),
    DECL_ENTRY_RESP("CONTENT_LENGTH"),
    DECL_ENTRY_RESP("CONTENT_LOCATION"),
    DECL_ENTRY_RESP("CONTENT_TYPE"),
    /**/
    DECL_ENTRY("COOKIE"),
    DECL_ENTRY_RESP("DATE"),
    DECL_ENTRY("DICTIONARY_ID"),
    DECL_ENTRY("EARLY_DATA"),
    /**/
    DECL_ENTRY("EXPECT"),
    DECL_ENTRY("FORWARDED"),
    DECL_ENTRY("FROM"),
    /* Host skipped, it's always checked (e.g. for HTTP1.1)*/
    DECL_ENTRY("IDEMPOTENCY_KEY"),
    /**/
    DECL_ENTRY("IF_MATCH"),
    DECL_ENTRY("IF_MODIFIED_SINCE"),
    DECL_ENTRY("IF_NONE_MATCH"),
    DECL_ENTRY("IF_RANGE"),
    /**/
    DECL_ENTRY("IF_UNMODIFIED_SINCE"),
    DECL_ENTRY_RESP_SYN("KEEP_ALIVE", "ALIVE"),
    DECL_ENTRY("LINK"),
    DECL_ENTRY("MAX_FORWARDS"),
    /**/
    DECL_ENTRY("ORIGIN"),
    DECL_ENTRY("PRAGMA"),
    DECL_ENTRY("PREFER"),
    DECL_ENTRY_RESP("PRIORITY"),
    /**/
    DECL_ENTRY("PROXY_AUTHORIZATION"),
    DECL_ENTRY("RANGE"),
    DECL_ENTRY_SYN("REFERER", "REFERRER"),
    DECL_ENTRY("REPR_DIGEST"),
    /**/
    DECL_ENTRY("RTT"),
    DECL_ENTRY("SAVE_DATA"),
    DECL_ENTRY("SERVICE_WORKER"),
    DECL_ENTRY("SERVICE_WORKER_NAVIGATION_PRELOAD"),
    /**/
    DECL_ENTRY("TE"),
    DECL_ENTRY_RESP("TRAILER"),
    DECL_ENTRY_RESP("TRANSFER_ENCODING"),
    DECL_ENTRY_RESP("UPGRADE"),
    /**/
    DECL_ENTRY("UPGRADE_INSECURE_REQUESTS"),
    DECL_ENTRY("USER_AGENT"),
    DECL_ENTRY_RESP("VIA"),
    DECL_ENTRY_RESP("WANT_CONTENT_DIGEST"),
    /**/
    DECL_ENTRY_RESP("WANT_REPR_DIGEST"),
    DECL_ENTRY("X_FORWARDED_FOR"),
    DECL_ENTRY("X_FORWARDED_HOST"),
    DECL_ENTRY("X_FORWARDED_PROTO"),
};
#if 0

#define REQ_HEADER_KEEP_LAST REQ_HEADER_KEEP_X_FORWARDED_PROTO
#endif

#undef DECL_ENTRY_RESP
#undef DECL_ENTRY_RESP_SYN
#undef DECL_ENTRY
#undef DECL_ENTRY_SYN

char *codegen_generate_header_constants()
{
    size_t totalSize = 0, totalSizeForNever = 0;
    TestStringBuffer string_buffer;
    TestStringBuffer string_buffer_for_never;

    clearTestStringBuffer(&string_buffer);
    clearTestStringBuffer(&string_buffer_for_never);

#define SZZZ (sizeof(codegen_header_constants) / sizeof(struct trp))

    int runs = 2;
    int hasError = 0;

    const char *neverPrefix = "NEVER_REQ_HEADER_KEEP_";
    const char *reqPrefix = "REQ_HEADER_KEEP_";
    const char *warningCheck = "ENABLE_EMPTY_HEADER_MASK_WARNING";

#define ADD_AND_CHECK(ERR)                                     \
    addRes = addToTestStringBuffer(&string_buffer, buffer, r); \
    do                                                         \
    {                                                          \
        if (addRes != 0)                                       \
        {                                                      \
            printf("//ERROR: %s.(CODE:%d)\n", ERR, r);         \
            hasError++;                                        \
            break;                                             \
        }                                                      \
    } while (0)

#define ADD_AND_CHECK_NEVER(ERR)                                         \
    addRes = addToTestStringBuffer(&string_buffer_for_never, buffer, r); \
    do                                                                   \
    {                                                                    \
        if (addRes != 0)                                                 \
        {                                                                \
            printf("//ERROR: %s.(CODE:%d)\n", ERR, r);                   \
            hasError++;                                                  \
            break;                                                       \
        }                                                                \
    } while (0)

#define __BL 2048
#define __BL2 128

    char buffer[__BL];
    memset(buffer, 0, __BL);
    char name_buffer[__BL2];
    char never_buffer[__BL2];
    char prev_name_buffer[__BL2];

    char *names_processed[SZZZ];
    char names_skip[SZZZ];
    memset(names_processed, 0, sizeof(names_processed));
    memset(names_skip, 0, sizeof(names_skip));

    const char *never_template = "#ifndef %s\n#define %s 0\n#endif\n\n";
    const char *template_if = "#if %s\n\
#if %s\n#define %s _Pragma(\"GCC warning \\\"%s empty. Reason: %s!\\\"\") 0\n\
#endif\n";
    const char *template_else = "#else\n#define %s (0x1)%s\n#endif\n\n";

    // never,cur,prev, warning, name,name, never
    const char *template_if2 = "#if %s\n\
#define __IMPL_RHK_%s_LS (__IMPL_RHK_%s_LS)\n\
#if %s\n#define %s _Pragma(\"GCC warning \\\"%s empty. Reason: %s!\\\"\") 0\n\
#endif\n";

    // cur,prev,name,cur, resp
    const char *template_else2 = "#else\n#define __IMPL_RHK_%s_LS (__IMPL_RHK_%s_LS + 1)\n#define %s (0x1ul << __IMPL_RHK_%s_LS)%s\n#endif\n\n";

    while (runs != 0 && hasError == 0)
    {
        runs--;
        int r = initTestStringBuffer(&string_buffer, totalSize);
        if (0 != r)
        {
            printf("//ERROR: no string buffer.(CODE:%d)\n", r);
            break;
        }

        r = initTestStringBuffer(&string_buffer_for_never, totalSizeForNever);
        if (0 != r)
        {
            printf("//ERROR: no string buffer for never.(CODE:%d)\n", r);
            break;
        }

        size_t i;
        int addRes;
#if 0
        for (i = 0; i < SZZZ; i++)
        {
            char *pName = codegen_header_constants[i].name;
            // printf("CHECK|%s|\n", pName);

            memset(buffer, 0, __BL);
            r = sprintf(buffer, "#define %s%s 0\n", neverPrefix, pName);
            ADD_AND_CHECK("adding never ");
            if (runs == 1)
            {
                size_t j;
                int e = 0;
                for (j = 0; j < i; j++)
                {
                    if (names_processed[j] != 0 && (0 == strcmp(names_processed[j], pName)))
                    {
                        fprintf(stderr, "Name seen: %s\n", pName);
                        names_skip[i] = 1;
                        e++;
                    }
                }
                if (e == 0)
                {
                    names_processed[i] = pName;
                }
            }
        }
#endif


        r = sprintf(buffer, "\n\n");
        ADD_AND_CHECK("adding newlines");

        r = sprintf(buffer, "/*----*/\n");
        ADD_AND_CHECK("adding separator");

        char *prevName = codegen_header_constants[0].name;

        r = sprintf(buffer, "#define __IMPL_RHK_%s_LS 0\n\n", prevName);
        ADD_AND_CHECK("adding first LS");

        memset(name_buffer, 0, __BL2);
        memset(never_buffer, 0, __BL2);

        char *pAlsoResp = codegen_header_constants[0].also_response ? " /*!< also response */" : "";

        int name_len = sprintf(name_buffer, "%s%s", reqPrefix, prevName);
        sprintf(never_buffer, "%s%s", neverPrefix, prevName);

        // NEVER...
        memset(buffer, 0, __BL);
        r = sprintf(buffer, never_template, never_buffer, never_buffer);
        ADD_AND_CHECK_NEVER("adding never template");

        memset(buffer, 0, __BL);
        r = sprintf(buffer, template_if, never_buffer, warningCheck, name_buffer, name_buffer, never_buffer);
        ADD_AND_CHECK("adding if template");

        memset(buffer, 0, __BL);
        r = sprintf(buffer, template_else, name_buffer, pAlsoResp);
        ADD_AND_CHECK("adding else template");

        memset(prev_name_buffer, 0, __BL2);
        memcpy(prev_name_buffer, name_buffer, name_len);

        for (i = 1; i < SZZZ; i++)
        {
            if (names_skip[i])
            {
                continue;
            }

            char *pCurrentName = codegen_header_constants[i].name;
            pAlsoResp = codegen_header_constants[i].also_response ? " /*!< also response */" : "";

            memset(name_buffer, 0, __BL2);
            memset(never_buffer, 0, __BL2);

            name_len = sprintf(name_buffer, "%s%s", reqPrefix, pCurrentName);
            sprintf(never_buffer, "%s%s", neverPrefix, pCurrentName);

            // NEVER...
            memset(buffer, 0, __BL);
            r = sprintf(buffer, never_template, never_buffer, never_buffer);
            ADD_AND_CHECK_NEVER("adding never template");

            // never,cur,prev, warning, name,name, never
            memset(buffer, 0, __BL);
            r = sprintf(buffer, template_if2, never_buffer, pCurrentName, prevName, warningCheck, name_buffer, name_buffer, never_buffer);
            ADD_AND_CHECK("adding if template");

            memset(buffer, 0, __BL);
            r = sprintf(buffer, template_else2, pCurrentName, prevName, name_buffer, pCurrentName, pAlsoResp);
            ADD_AND_CHECK("adding else template");

            memset(prev_name_buffer, 0, __BL2);
            memcpy(prev_name_buffer, name_buffer, name_len);

            prevName = pCurrentName;
        }
        const char *last_template = "#define REQ_HEADER_LAST_VALUE (0x1ul << __IMPL_RHK_%s_LS)\n\n";

        memset(buffer, 0, __BL);
        r = sprintf(buffer, last_template, prevName);
        ADD_AND_CHECK("adding last template");

        totalSize = string_buffer.used + 10;
        totalSizeForNever = string_buffer_for_never.used + 10;
        printf("//RUN:%d |size:%ld |size(never):%ld\n", runs, totalSize, totalSizeForNever);
    }

#undef ADD_AND_CHECK
#undef SZZZ

    char *pResult = 0;

    if (hasError == 0)
    {
        size_t s1 = strlen(string_buffer_for_never.data);
        size_t s2 = strlen(string_buffer.data);
        size_t sum = s1 + s2 + 10;
        pResult = (char *)calloc(sum, 1);
        if (pResult != 0)
        {
            memcpy(pResult, string_buffer_for_never.data, s1);
            memcpy(pResult + s1, "\n\n", 2);
            memcpy(pResult + s1 + 2, string_buffer.data, s2);
        }
        // return string_buffer.data;
    }

    freeTestStringBuffer(&string_buffer);
    freeTestStringBuffer(&string_buffer_for_never);

    return pResult;
}

#endif