#include <stdio.h>
#include <stdlib.h>
#include <utmpx.h>
#include <pwd.h>

void display_users(){
    struct utmpx *user = getutxent();
    struct passwd *uid;
    while(user !=NULL){
        if(user->ut_type == USER_PROCESS){
            uid = getpwnam(user->ut_user);
            printf("%d ",uid->pw_uid);
            printf("%s ",user->ut_user);
            printf("%s ",user->ut_line);
            printf("%s ",user->ut_host);
        }
        user=getutxent();
    }
}