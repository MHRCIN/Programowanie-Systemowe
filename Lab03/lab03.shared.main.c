#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utmpx.h>
#include <pwd.h>
#include <stdbool.h>
#include <grp.h>
#include <dlfcn.h>
void (*hgfunction)();
void (*hfunction)();
void (*gfunction)();
void (*nullfunction)();

int main(int argc, char *argv[]){
    bool gflag = false;
    bool hflag = false;
    char c;

    void *handle = dlopen("./lib.so",RTLD_LAZY);
    if(!handle){
    //Kopiuj wklej z zadania 1.  
    struct utmpx *user = getutxent();
    while (user != NULL) {
        if (user->ut_type == USER_PROCESS) {
            printf("%s \n", user->ut_user);
        }
        user = getutxent();
    }
    return 0;
    }


    while ((c = getopt(argc, argv, "hg")) != -1) {
        switch (c) {
            case 'h':
                hflag = true;
                break;
            case 'g':
                gflag = true;
                break;
            default:
                printf("1.Niezdefiniowany przełącznik, spróbuj -h lub -g\n");
                return 1;
        }
    }
    hgfunction=dlsym(handle,"hgfunction");
    hfunction=dlsym(handle,"hfunction");
    gfunction=dlsym(handle,"gfunction");
    nullfunction=dlsym(handle,"nullfunction");

    if(hflag && gflag) hgfunction(); 
    else if(hflag) hfunction();
    else if(gflag) gfunction();
    else if(argc==1) nullfunction();
    else printf("3.Niezdefiniowany przełącznik spróbuj -h lub -g");

    if(handle) dlclose(handle);
    
    
    
}
