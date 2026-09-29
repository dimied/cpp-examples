
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <unistd.h>

#include "http_header_codegen.h"
#include "../colors.h"

void signalHandler(int sig)
{
    //(void)(sig);
    const char msg[] = "Caught SIGINT\n";

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
#pragma GCC diagnostic pop

    _exit(sig);
}

int main()
{
    signal(SIGINT, signalHandler);

    printf("//#http_methods: %ld\n", sizeof(http_methods) / (sizeof(http_methods[0])));
    printf("//#header_names: %ld\n", sizeof(header_names) / (sizeof(header_names[0])));
#if 1
    {
        char *code = codegen_generate_header_constants();
        if (code != 0)
        {
            printf("\n\n//CODE\n");
            printf("%s", code);
            printf("//CODE-END\n\n");
            free(code);
        }
        else
        {
            color_text(COLOR_RED | TEXT_EFFECT_UNDERLINE, "//ERROR: header constants codegen failed.\n\n");
        }
    }
#endif

#if 0
    printHeaderConstants();
#endif
    return 0;
}