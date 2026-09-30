#ifndef SERVER_BASIC_TYPES_INCLUDED
#define SERVER_BASIC_TYPES_INCLUDED

#ifndef S_SPLINT_S
#include <stdint.h>
#endif

#ifdef S_SPLINT_S
typedef unsigned long int ui64_type;
#else
typedef uint64_t ui64_type;
#endif

#endif