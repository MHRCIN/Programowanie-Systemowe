#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <crypt.h>
#include <stdbool.h>
unsigned long nlines=0;
unsigned long checkedlines=0;
int test = 1000;
int benchmark = 1; 
char* password;
bool password_found = false;
typedef struct{
    char *start; //początek bloku który ma wykonac watek
    char *end; //koniec -||- 
    char *salt; //wskaznik na skrót hasła
    int thread_id; //id wątku
    pthread_mutex_t *mutex;
} thread_data;
char *salt(char *password) {
    /*
    wyjmuje z całości skrótu ciąg który znajduje się między drugim a trzecim symbolem $
    */
    
    char *start = strchr(password, '$');
    start = strchr(start + 1, '$');
    char *end = strchr(start + 1, '$');
    size_t len = end - (start + 1);  
    char *salt = malloc(len + 1);
    strncpy(salt, start + 1, len);  
    salt[len] = '\0'; 
    return salt;
}
void print_progress(){
    /*
    wyswietlanie progresu
    */
    double progress = (double)checkedlines / nlines *100;
    printf("\rProgress: %3.2f%%", progress);
    fflush(stdout);
}
void *check_password(void *data){
    /*
    funkcja wątku, tworzy skrót z linii i porównuje z hasłem
    */
    thread_data *t_data = (thread_data *)data;
    char *start = t_data->start;
    char *end = t_data->end;
    char *salt = t_data->salt;
    int thread_id = t_data->thread_id;
    char buffer[256];

    struct crypt_data cdata;
    cdata.initialized = 0;

    while(start<end && !password_found){
        int i = 0;
        while (*start != '\n' && start < end) {
            buffer[i++] = *start;
            start++;
        }
        if (start < end) {
            start++;
        }

        buffer[i] = '\0';

        
        
        char hash[32];
        snprintf(hash, sizeof(hash), "$6$%s$", salt);
        char *skrot = crypt_r(buffer, hash,&cdata);
        //printf("Skrót hasła w formacie /etc/shadow: %s\n", skrot);
        
        
        if (strcmp(skrot,password)==0) {
            pthread_mutex_lock(t_data->mutex);
            printf("\n\nPassword found !\n");
            printf("%d   %s\n",thread_id,buffer);
            password_found = true;
            pthread_mutex_unlock(t_data->mutex);
        }else{
            
            pthread_mutex_lock(t_data->mutex);
            checkedlines++;
            if(!password_found && !benchmark) print_progress();
            // Pomocnicze wyswietlanie skrotow 
            //printf("%s   \n%s %d\n\n",skrot,password,strlen(buffer));
            pthread_mutex_unlock(t_data->mutex);
        }
        if(checkedlines>=test && benchmark) password_found = true;
    }


}
void dehashing(char* password, char* file_name, int num_threads){
    /*
    główna funkcja programu
    */
    char* salt_str = salt(password);
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    if(!benchmark){
    printf("--------------------\n   Starting attack    \n--------------------\n");
    printf("Salt: %s \n",salt_str);
    printf("File name: %s \n", file_name);
    printf("Number of threads: %d \n", num_threads);
    }
    
   

    int fd = open(file_name, O_RDONLY);
    struct stat sb;
    fstat(fd, &sb);
    char *file= mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    //zliczanie wszystkich linii w pliku
    for(size_t i=0;i<sb.st_size;i++){
        if(file[i] == '\n'){
            nlines++;
        }
    }

    pthread_t threads[num_threads];
    thread_data thread_args[num_threads];
    size_t chunk_size = sb.st_size / num_threads;
    size_t file_size = (size_t)sb.st_size;
    // ? Sprawdzic czy nie wyjdzie że wskoczy w połowie ?

    for(int i=0;i<num_threads;i++){
        thread_args[i].salt = salt_str;
        thread_args[i].thread_id = i+1;
        thread_args[i].end = file + (i+1) * chunk_size;
        thread_args[i].mutex = &mutex;
        if (i == 0) {
            thread_args[i].start = file;
        } else {
            char *start_ptr = file + (i * chunk_size);
            while (*start_ptr != '\n' && start_ptr > file) {
                start_ptr--;
            }
            thread_args[i].start = start_ptr + 1;
        }
        
            char *end_ptr = file + ((i + 1) * chunk_size);
            while (*end_ptr != '\n' && end_ptr < file + file_size) {
                end_ptr++;
            }
            thread_args[i].end = end_ptr;
        

        pthread_create(&threads[i],NULL, check_password, (void *)&thread_args[i]);

    }

    for(int i=0;i<num_threads;i++){
        pthread_join(threads[i],NULL);
    }
    pthread_mutex_destroy(&mutex);
    if(!password_found) printf("Password not found ;( ");
    
    munmap(file_name, sb.st_size);
    close(fd);
}  
void benchmark_function(char* password, char* file_name, int max_threads){
    
    printf("Testing on %d lines\n",test);
    for(int i=1;i<=max_threads;i++){
        printf("Number of threads: %d\n",i);
        clock_t start = clock();
        dehashing(password,file_name,i);
        password_found = false;
        checkedlines = 0;
        clock_t end = clock();

        double time = ((double)(end-start)) / CLOCKS_PER_SEC;
        printf("    Done in: %.2f seconds\n",time);

    }
}
int main(int argc, char **argv){
    char* file_name;
    char c;
    
    int num_threads;
    while ((c = getopt(argc, argv, "s:f:t:")) != -1){
        switch (c)
        {
        case 's':
            password = optarg;
            break;
        case 'f':
            file_name = optarg;
            break;
        case 't':
            benchmark = 0;
            num_threads = atoi(optarg);
        default:
            break;
        }
    }
    int max_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (max_threads < num_threads && !benchmark)
    {
        printf("Too many threads to create\n");
        return 1;
    }
    if(benchmark){
        printf("--------------------\n   Benchmark    \n--------------------\n");
        printf("Max number of threads: %d \n",max_threads);
        benchmark_function(password,file_name,max_threads);
    }
    else
    {
        dehashing(password,file_name,num_threads);
    }



    return 0;
}