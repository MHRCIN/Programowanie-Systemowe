#ifndef TIMER_H
#define TIMER_H
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>


static void freeMemory(void *buffer);
static void createKey(void);

void start();
double stop();
#endif 
