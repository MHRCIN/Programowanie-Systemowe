#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utmpx.h>
#include <pwd.h>
#include <stdbool.h>
#include <grp.h>
void hgfunction();
void hfunction();
void gfunction();
void nullfunction();

int main(int argc, char *argv[]){
    bool gflag = false;
    bool hflag = false;
    char c;

    
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


    if(hflag && gflag) hgfunction();
    else if(hflag) hfunction();
    else if(gflag) gfunction();
    else if(argc==1) nullfunction();
    else printf("3.Niezdefiniowany przełącznik spróbuj -h lub -g");


    
    
    
}
