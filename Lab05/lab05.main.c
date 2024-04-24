#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <execinfo.h>
#include <wait.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
bool endflag = true;
int flag = true;
int lifeTime;
int counter = 0;
char* czas(time_t time){
    char* c_time_string = ctime(&time);
    c_time_string[strcspn(c_time_string, "\n")] = '\0';
    return c_time_string;
}
unsigned long long silnia(unsigned int n){
    unsigned long long silnia = 1;
    for(int i=1;i<=n;i++){
    silnia*=i;  
    }
    
    return silnia;
}

void end(int no, siginfo_t *info, void *ucontext){
    endflag = false;
}
void child(int no, siginfo_t *info, void *ucontext){
    time_t end_time = time(NULL);
    printf("         [%d] [%d] [%s]\n", info->si_pid, info->si_status, czas(end_time));
    counter--;
    if(!endflag && counter == 0) exit(lifeTime);
    
}
void child_USR1(int no, siginfo_t *info, void *ucontext){
    time_t create_time = time(NULL);
    printf("[%d] [%d] [%s]\n", info->si_pid, info->si_status ,czas(create_time));
}

void child_alarm_signal() //sigalrm
{
    flag=false;
    exit(lifeTime);
}


int main(int argc, char **argv)
{
    
    int arg;
    int maxLifeTime = 2;
    int przerwa  = 2;

    while((arg = getopt(argc, argv, "w:m:")) != -1){
        switch (arg){
        case 'w':
            przerwa = atoi(optarg);
            break;
        case 'm':
            maxLifeTime = atoi(optarg);
            break;
        case '?':
            return 0;
        }
    }
    

    struct sigaction act;
    act.sa_sigaction = end;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);

    struct sigaction act2;
    act2.sa_sigaction = child;
    act2.sa_flags = SA_SIGINFO;
    sigemptyset(&act2.sa_mask);
    sigaction(SIGCHLD, &act2, NULL);
    
    struct sigaction act3;
    act3.sa_sigaction = child_USR1;
    act3.sa_flags = SA_SIGINFO;
    sigemptyset(&act3.sa_mask);
    sigaction(SIGUSR1, &act3, NULL);

    pid_t pid;
    

    while (1){
        if(endflag){
            
            pid = fork();
            counter++;

            if (pid == -1) return 1;
            if (pid == 0){
                // Kod procesu potomnego
                srand(time(NULL)^getpid());
                lifeTime = (rand() % maxLifeTime+1);
                if(lifeTime==0) lifeTime++;
                signal(SIGALRM,child_alarm_signal);
                alarm(lifeTime);
                union sigval value;
                value.sival_int = lifeTime;
                sigqueue(getppid(),SIGUSR1,value);
                while(flag) silnia(lifeTime);
                exit(lifeTime);
            }


        }
        time_t start_time = time(NULL);
        time_t current_time = time(NULL);
        
        while (difftime(current_time, start_time) < przerwa) current_time = time(NULL);




        if(!endflag && counter == 0){
            break;
        }
        

    }

    return 0;
}

