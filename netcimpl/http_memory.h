#ifndef HTTP_MEMORY
#define HTTP_MEMORY

typedef int (*free_function)(int type, void *data);

free_function free_functions[128];

#define SET_FREE_FUNCTION_OK 0
#define SET_FREE_FUNCTION_ERROR_INVALID_ARGS 1

static void init_memory()
{
    memset(free_functions, 0, sizeof(free_functions));
}

static int set_free_function(int type, free_function ff)
{
    if (type > 0 && type < 128 && ff != 0)
    {
        free_functions[type] = ff;
        return SET_FREE_FUNCTION_OK;
    }
    return SET_FREE_FUNCTION_ERROR_INVALID_ARGS;
}

static int unset_free_function(int type)
{
    if (type > 0 && type < 128)
    {
        free_functions[type] = 0;
        return SET_FREE_FUNCTION_OK;
    }
    return SET_FREE_FUNCTION_ERROR_INVALID_ARGS;
}

#endif