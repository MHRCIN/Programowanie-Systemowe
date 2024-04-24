#include "lab06.timer.h"
#include <time.h>
#include <stddef.h>
#include <stdlib.h>



static pthread_key_t counterKey;
static pthread_once_t counterOnce = PTHREAD_ONCE_INIT;

static void freeMemory(void *buffer)
{
    free(buffer);
}

static void createKey(void)
{
    pthread_key_create(&counterKey, freeMemory);
}

void start()
{
    double *counter;
    pthread_once(&counterOnce, createKey);
    counter = pthread_getspecific(counterKey);
    if (counter == NULL)
    {
        counter = malloc(sizeof(double));
        pthread_setspecific(counterKey, counter);
    }
    *counter = time(NULL);
}

double stop()
{
    double stop = time(NULL);
    double *counter = pthread_getspecific(counterKey);
    return stop - *counter;
}
