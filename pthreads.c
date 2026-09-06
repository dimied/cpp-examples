

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *job(void *param)
{
    //(void)(param);
    int *p = (int *)param;
    printf("Thread: %d\n", *p);

    return (void *)0;
}

const char *none = "???";

int main(int argc, char *argv[])
{
    //(void)(argc);
    (void)(argv);
    const char *firstParam = argc > 0 ? argv[0] : none;
    printf("Hello: #args = %d | argv[0]=\'%s\'\n", argc, firstParam);

    pthread_t threadIds[3];

    int params[4] = {1, 5, 4, 0};
    int results[4];

    for (size_t i = 0; i < 3; i++)
    {
        int res = pthread_create(&threadIds[i], NULL, job, &params[i]);
        printf("Thread created:RES=%d:@%ld |ID: %ld\n", res, i, threadIds[i]);
        results[i] = res;
    }
    //Waiting for the end.
    for (size_t i = 0; i < 3; i++)
    {
        if (results[i] == 0)
        {
            printf("Wait:@%d: %d\n", i, threadIds[i]);
            pthread_join(threadIds[i], NULL); //No result needed.
            printf("Joined:@%d: %d\n", i, threadIds[i]);
        }
    }

    return 0;
}