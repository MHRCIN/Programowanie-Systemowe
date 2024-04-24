#include <stdio.h>
#include <stdlib.h>
#include <utmpx.h>
#include <pwd.h>
#include <stdbool.h>
#include <grp.h>
void hgfunction(){
    int n=50; //maksymalna liczba grup 
    int ngroups;
    struct utmpx *user = getutxent();
    struct passwd *uid;
    gid_t *groups = malloc(sizeof(*groups) * n); if(groups == NULL) return;


    while (user != NULL) {
        if (user->ut_type == USER_PROCESS) {
            //Login i hosty
            printf("%s ", user->ut_user);
            printf("%s ", user->ut_host);
            
            //Grupy
            uid = getpwnam(user->ut_user);
            ngroups = getgrouplist(user->ut_user, uid->pw_gid, groups,&n );
            printf(" [");
            int i=0;
            while(i<ngroups){
                printf("%s, ", getgrgid(groups[i])->gr_name );
                i++;
            }
            printf("]\n");

            free(groups);
        }
        user = getutxent();
    }
};

void hfunction(){
    //Kopiuj wklej z zadania 1. 
    struct utmpx *user = getutxent();
    while (user != NULL) {
        if (user->ut_type == USER_PROCESS) {
            printf("%s ", user->ut_user);
            printf("%s \n", user->ut_host);
        }
        user = getutxent();
    }
}
void gfunction(){
    int n=50; //maksymalna liczba grup 
    int ngroups;
    struct utmpx *user = getutxent();
    struct passwd *uid;
    gid_t *groups = malloc(sizeof(*groups) * n); if(groups == NULL) return;


    while (user != NULL) {
        if (user->ut_type == USER_PROCESS) {
            //Login
            printf("%s ", user->ut_user);
            uid = getpwnam(user->ut_user);
            //Grupy
            ngroups = getgrouplist(user->ut_user, uid->pw_gid, groups,&n );
            printf(" [");
            int i=0;
            while(i<ngroups){
                printf("%s, ", getgrgid(groups[i])->gr_name );
                i++;
            }
            printf("]\n");

            free(groups);
        }
        user = getutxent();
    }
}
void nullfunction(){
    //Kopiuj wklej z zadania 1.  
    struct utmpx *user = getutxent();
    while (user != NULL) {
        if (user->ut_type == USER_PROCESS) {
            printf("%s \n", user->ut_user);
        }
        user = getutxent();
    }
}