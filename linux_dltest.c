
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <GL/gl.h>
//
#include <stdio.h>

#include "linux_dl.h"
#include <stdint.h>
#include <stdlib.h>


int main()
{
    GLenum err = glGetError();
    printf("E: %d\n", err);

    // E.g.        dl_iterate_phdr
    int res = iterate();
    printf("IRES: %d\n", res);

    findModule();

#if 0
    const GLubyte *s = glGetString(GL_VERSION);
    printf("R: %s\n", s);

    s = glGetString(GL_RENDERER);
    printf("R2: %s\n", (char *)s);

    s = glGetString(GL_VENDOR);
    printf("R3: %s\n", s);
#endif

    return 0;
}