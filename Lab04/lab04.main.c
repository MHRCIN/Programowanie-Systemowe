#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <getopt.h>
#include <sys/resource.h>

int main(int argc, char **argv) {
    int c, n = 1;
    bool vflag = false;

    while ((c = getopt(argc, argv, "vt:")) != -1) {
        switch (c) {
            case 'v':
                vflag = true;
                break;
            case 't':
                n = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Blad, uzyj opcji -t lub -v");
                return 0;
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "Brakuje argumentu.\n");
        return 0;
    }

    

    double real_total = 0, user_total = 0, system_total = 0;
    struct timespec start_time, end_time;
    
    for(int i=0;i<n;i++){
        pid_t pid = fork();
        if (pid == -1) {
            return 1;
        } else if (pid == 0) {
            // Child process
            close(1);
            int h = open("/dev/null", O_WRONLY);
            dup2(h, 1);
            close(2);
            h = open("/dev/null", O_WRONLY);
            dup2(h, 2);

        } else {
            // Parent process
            int status;
            struct rusage usage;
            clock_gettime(CLOCK_MONOTONIC, &start_time);

            wait4(pid, &status, 0, &usage);


            clock_gettime(CLOCK_MONOTONIC, &end_time);

            double real_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
            double user_time = (double) usage.ru_utime.tv_sec + (double) usage.ru_utime.tv_usec / 1e6;
            double system_time = (double) usage.ru_stime.tv_sec + (double) usage.ru_stime.tv_usec / 1e6;

            real_total +=real_time;
            user_total +=user_time;
            system_total +=system_time;
            if (vflag) {
                printf("Command output:\n");
                system(argv[optind]);
                printf("\n");
            }
            printf("real\t%.2f\nuser\t%.2f\nsys\t%.2f\n\n", real_time, user_time, system_time);
        }
    }
    if(n>1){
        printf("\n Average: \n");
        printf("real\t%.2f\nuser\t%.2f\nsys\t%.2f\n", real_total/n, user_total/n, system_total/n);
    }


    return 0;
}