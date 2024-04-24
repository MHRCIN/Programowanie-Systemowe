#include "lab06.timer.h"
#include <stdbool.h>

int maxLifeTime = 2;
int maxThreads  = 2;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_key_t start_time_key;

void my_signal(){
    printf("        Id:%ld Time: %f\n", pthread_self(), stop());
    pthread_exit(NULL);
}


void* thread_func(void* arg){
    start();
    pthread_setspecific(start_time_key, (void *)start);
    long long silnia = 1;
    int i = 2;
    while (true)
    {
        silnia*=i;
    }
}

int main(int argc, char **argv)
{
    
    int arg;

    while((arg = getopt(argc, argv, "t:m:")) != -1){
        switch (arg){
        case 't':
            maxThreads = atoi(optarg);
            break;
        case 'm':
            maxLifeTime = atoi(optarg);
            break;
        case '?':
            return 1;
        }
    }

    struct sigaction act;
    act.sa_handler = my_signal;
    act.sa_flags = 0;
    sigemptyset(&(act.sa_mask));
    sigaction(SIGUSR1, &act, NULL);

    pthread_t* threads = malloc(maxThreads * sizeof(pthread_t));
    int* thread_time = malloc(maxThreads * sizeof(int));
    srand(time(NULL));
    
    for(int i=0; i<maxThreads;i++){
        thread_time[i] = rand() % maxLifeTime + 1;
        pthread_key_create(&start_time_key, NULL);
        pthread_create(&threads[i], NULL, thread_func, NULL);
        printf("ID: %ld   LifeTime: %ds  \n", threads[i], thread_time[i]);
    }
    start();
    int pom = maxThreads;
    while (pom != 0){
        for (int i = 0; i < maxThreads; i++){
            if(thread_time[i]==0) continue;
            if (stop() >= thread_time[i]){
                pthread_kill(threads[i], SIGUSR1);
                pthread_join(threads[i], NULL);
                thread_time[i] = 0;
                pom--;
            }
        }
    } 



    free(threads);
    free(thread_time);

    return 0;
}

