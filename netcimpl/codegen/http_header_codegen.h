#ifndef HTTP_PARSER_HEADER_CODEGEN
#define HTTP_PARSER_HEADER_CODEGEN

#include <stdint.h>

#include "../test_string_buffer.h"

#include "../http_parser_header_constants.h"
#include "../http_parser_macros.h"
#include "../http_parser_header_assigned_data.h"
#include "../http_parser_header_empty_data.h"

struct trp
{
    char *name;
    int also_response;
    char *synonym;
};

#define DECL_ENTRY(x) {x, 0, 0}
#define DECL_ENTRY_SYN(x, syn) {x, 0, syn}
#define DECL_ENTRY_RESP(x) {x, 1, 0}
#define DECL_ENTRY_RESP_SYN(x, syn) {x, 1, syn}

#if 0
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
#endif

#undef DECL_ENTRY_RESP
#undef DECL_ENTRY_RESP_SYN
#undef DECL_ENTRY
#undef DECL_ENTRY_SYN

const char *neverPrefix = "NEVER_REQ_HEADER_KEEP_";
const char *reqPrefix = "REQ_HEADER_KEEP_";
const char *warningCheck = "ENABLE_EMPTY_HEADER_MASK_WARNING";

const char *never_template = "#ifndef %s\n#define %s 0\n#endif\n\n";

char *also_resp[] = {
    "Accept"};

#define __BL 2048
#define __BL2 128

char buffer[__BL];
char name_buffer[__BL2];
char never_buffer[__BL2];
char prev_name_buffer[__BL2];
char buffer_for_const_name[2][128];

void noop() {}

void clear_name_buffer()
{
    memset(name_buffer, 0, __BL2);
}
void clear_never_buffer()
{
    memset(never_buffer, 0, __BL2);
}

char *create_constant_name(const char *name, int buffer_idx)
{
    if (name != 0)
    {
        size_t s = strlen(name);
        if (s > 127)
        {
            printf("//ERROR: Name too long. '%s' |#len=%ld\n", name, s);
            return 0;
        }
        char *buf = buffer_for_const_name[buffer_idx];
        memset(buf, 0, 128);
        size_t i;
        for (i = 0; i < s; i++)
        {
            char c = name[i];
            if (c >= 'a' && c <= 'z')
            {
                c -= 32;
            }
            else if (c == '-')
            {
                c = '_';
            }
            buf[i] = c;
        }
        return buf;
    }
    return 0;
}

const char *empty_string = "";
const char *also_response_string = " /*!< also response */";

const char *also_response_comment(const char *name)
{
    size_t i;
    for (i = 0; i < (sizeof(also_resp) / sizeof(also_resp[0])); i++)
    {
        if (0 == strcasecmp(also_resp[i], name))
        {
            return also_response_string;
        }
    }
    return empty_string;
}

const char *get_header_name(int i)
{
    return header_names[i];
}

void copy_name_to_prev_name(size_t name_len)
{
    memset(prev_name_buffer, 0, __BL2);
    memcpy(prev_name_buffer, name_buffer, name_len);
}

int fill_name_buffer(char *name)
{
    clear_name_buffer();
    return sprintf(name_buffer, "%s%s", reqPrefix, name);
}
int fill_never_buffer(char *name)
{
    clear_never_buffer();
    return sprintf(never_buffer, "%s%s", neverPrefix, name);
}

int fill_buffer_for_never_ifndef(char *buffer)
{
    // NEVER...
    memset(buffer, 0, __BL);
    return sprintf(buffer, never_template, never_buffer, never_buffer);
}

size_t codegen_aligned_size(const char *name)
{
    const size_t s = strlen(name);
    size_t padding = (s & 0x7);
    if (padding != 0)
    {
        padding = 8 - padding;
    }
    return (s + padding);
}

int fill_buffer_for_if_first()
{
    static const char *template_if = "#if %s\n\
#define __IMPL_RHK_%s_AB8S 0\n\
#if %s\n#define %s _Pragma(\"GCC warning \\\"%s empty. Reason: %s!\\\"\") 0\n\
#endif\n";
    memset(buffer, 0, __BL);
    return sprintf(buffer, template_if, never_buffer, name_buffer, warningCheck, name_buffer, name_buffer, never_buffer);
}

int fill_buffer_for_else_first(const char *name)
{
    static const char *template_else = "#else\n#define __IMPL_RHK_%s_AB8S %d\n\
#define %s (0x1)%s\n#endif\n\n";

    size_t s = codegen_aligned_size(name);
    const char *pAlsoResp = also_response_comment(name);
    memset(buffer, 0, __BL);
    return sprintf(buffer, template_else, name_buffer, s, name_buffer, pAlsoResp);
}

int fill_buffer_for_if(const char *pCurrentName, char *prevName)
{
    // never,cur,prev, warning, name,name, never
    static const char *template_if2 = "#if %s\n\
#define __IMPL_RHK_%s_LS (__IMPL_RHK_%s_LS)\n\
#define __IMPL_RHK_%s_AB8S (__IMPL_RHK_%s_AB8S)\n\
#if %s\n#define %s _Pragma(\"GCC warning \\\"%s empty. Reason: %s!\\\"\") 0\n\
#endif\n";

    memset(buffer, 0, __BL);
    return sprintf(buffer, template_if2, never_buffer, pCurrentName, prevName, pCurrentName, prevName, warningCheck, name_buffer, name_buffer, never_buffer);
}

int fill_buffer_for_else(const char *pCurrentName, char *prevName, const char *name)
{
    // cur,prev,name,cur, resp
    static const char *template_else2 = "#else\n#define __IMPL_RHK_%s_LS (__IMPL_RHK_%s_LS + 1)\n\
#define __IMPL_RHK_%s_AB8S (__IMPL_RHK_%s_AB8S + %ld)\n\
#define %s (0x1ul << __IMPL_RHK_%s_LS)%s\n#endif\n\n";

    size_t s = codegen_aligned_size(name);
    const char *pAlsoResp = also_response_comment(name);
    memset(buffer, 0, __BL);
    return sprintf(buffer, template_else2, pCurrentName, prevName, pCurrentName, prevName, s, name_buffer, pCurrentName, pAlsoResp);
}

char *codegen_generate_header_constants()
{
    size_t totalSize = 0, totalSizeForNever = 0;
    TestStringBuffer string_buffer;
    TestStringBuffer string_buffer_for_never;

#define SZZZ (sizeof(header_names) / sizeof(header_names[0]))

#define ADD_AND_CHECK(ERR)                                     \
    addRes = addToTestStringBuffer(&string_buffer, buffer, r); \
    if (addRes != 0)                                           \
    {                                                          \
        printf("//ERROR: %s.(CODE:%d)\n", ERR, r);             \
        printf("//buffer:'%s'", buffer);                       \
        hasError++;                                            \
        break;                                                 \
    }                                                          \
    else                                                       \
    {                                                          \
        memset(buffer, 0, r);                                  \
    }                                                          \
    noop()

#define ADD_AND_CHECK_NEVER(ERR)                                         \
    addRes = addToTestStringBuffer(&string_buffer_for_never, buffer, r); \
    if (addRes != 0)                                                     \
    {                                                                    \
        printf("//ERROR: %s.(CODE:%d)\n", ERR, r);                       \
        printf("//buffer:'%s'", buffer);                                 \
        hasError++;                                                      \
        break;                                                           \
    }                                                                    \
    else                                                                 \
    {                                                                    \
        memset(buffer, 0, r);                                            \
    }                                                                    \
    noop()

#define CHECK_R_OR_BREAK(MSG)                     \
    if (0 != r)                                   \
    {                                             \
        printf("//ERROR: %s(CODE:%d)\n", MSG, r); \
        break;                                    \
    }                                             \
    noop()

    clearTestStringBuffer(&string_buffer);
    clearTestStringBuffer(&string_buffer_for_never);

    int runs = 2;
    int hasError = 0;

    char *names_processed[SZZZ];
    char names_skip[SZZZ];

    memset(buffer, 0, __BL);

    memset(names_processed, 0, sizeof(names_processed));
    memset(names_skip, 0, sizeof(names_skip));
    int addRes;

    while (runs != 0 && hasError == 0)
    {
        runs--;
        int r = initTestStringBuffer(&string_buffer, totalSize);
        CHECK_R_OR_BREAK("no string buffer");

        r = initTestStringBuffer(&string_buffer_for_never, totalSizeForNever);
        CHECK_R_OR_BREAK("no string buffer for never");

        size_t i;

        r = sprintf(buffer, "\n\n");
        ADD_AND_CHECK("adding newlines");

        r = sprintf(buffer, "/*----*/\n");
        ADD_AND_CHECK("adding separator");

        const char *prevName_orig = get_header_name(0);

        char *prevName = create_constant_name(prevName_orig, 0);

        if (prevName == 0)
        {
            printf("//ERROR: No constant name part!\n");
            break;
        }

        r = sprintf(buffer, "#define __IMPL_RHK_%s_LS 0\n\n", prevName);
        ADD_AND_CHECK("adding first LS");

        int name_len = fill_name_buffer(prevName);
        fill_never_buffer(prevName);

        // NEVER...
        r = fill_buffer_for_never_ifndef(buffer);
        ADD_AND_CHECK_NEVER("adding never template");

        r = fill_buffer_for_if_first();
        ADD_AND_CHECK("adding if template");

        r = fill_buffer_for_else_first(prevName_orig);
        ADD_AND_CHECK("adding else template");

        copy_name_to_prev_name(name_len);

        for (i = 1; i < SZZZ; i++)
        {
            if (names_skip[i])
            {
                continue;
            }

            const char *pCurrentName_orig = get_header_name(i);
            if (pCurrentName_orig == 0)
            {
                continue;
            }
            if (0 == strlen(pCurrentName_orig))
            {
                continue;
            }

            // const char *pAlsoResp_now = also_response_comment(pCurrentName_orig);

            const char *pCurrentName = create_constant_name(pCurrentName_orig, 1);

            if (pCurrentName == 0)
            {
                printf("//ERROR: No constant name part!\n");
                break;
            }
            clear_name_buffer();
            clear_never_buffer();

            name_len = sprintf(name_buffer, "%s%s", reqPrefix, pCurrentName);
            sprintf(never_buffer, "%s%s", neverPrefix, pCurrentName);

            // NEVER...
            r = fill_buffer_for_never_ifndef(buffer);
            ADD_AND_CHECK_NEVER("adding never template");

            // never,cur,prev, warning, name,name, never
            r = fill_buffer_for_if(pCurrentName, prevName);
            ADD_AND_CHECK("adding if template");

            r = fill_buffer_for_else(pCurrentName, prevName, pCurrentName_orig);
            ADD_AND_CHECK("adding else template");

            copy_name_to_prev_name(name_len);

            memcpy(buffer_for_const_name[0], buffer_for_const_name[1], sizeof(buffer_for_const_name[0]));
            // prevName = pCurrentName;
        }

        memset(buffer, 0, __BL);
        const char *last_template_size = "#define __IMPL_RHK_MINIMAL_ALIGNED_SPACE (__IMPL_RHK_%s_AB8S)\n\n";
        r = sprintf(buffer, last_template_size, prevName);
        ADD_AND_CHECK("adding last template for size");

        memset(buffer, 0, __BL);
        const char *last_template_ls = "#define __IMPL_RHK_LAST_LS (__IMPL_RHK_%s_LS)\n\n";
        r = sprintf(buffer, last_template_ls, prevName);
        ADD_AND_CHECK("adding last ls template");

        memset(buffer, 0, __BL);
        const char *last_template = "#define REQ_HEADER_LAST_VALUE (0x1ul << __IMPL_RHK_%s_LS)\n\n";
        r = sprintf(buffer, last_template, prevName);
        ADD_AND_CHECK("adding last template");

        totalSize = string_buffer.used + 10;
        totalSizeForNever = string_buffer_for_never.used + 10;
        printf("//RUN:%d |size:%ld |size(never):%ld\n", runs, totalSize, totalSizeForNever);
    }

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
#undef ADD_AND_CHECK
#undef SZZZ
}

#endif