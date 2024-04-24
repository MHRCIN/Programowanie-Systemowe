#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
// Struktura do przekazania danych do wątku
typedef struct {
    int sock;
    struct sockaddr_in clientAddr;
} thread_data_t;

char filename[256];
char filename_org[256];
char *ROOT;
char *PORT;

bool is_port_valid(char *port);
void Log(const char *ip, const char *status, char *const *request);
void Initialize();
void *handle_client(void *data);
const char *get_mime_type(char *filename);

int main(int argc, char **argv){
    bool start = false;
    bool stop = false;
    int c;
    if(argc <2){
        printf(" Niepoprawne argumenty \n");
        printf(" Poprawne argumenty to [-s|-q] [-p port] [-d document_root]\n");
        return 1;
    }
    while((c = getopt(argc, argv, "qsp:d:")) != -1){
        switch(c){
            case 's':
                start = true;
                break;
            case 'q':
                stop = true;
                break;
            case 'p':
                PORT = optarg;
                if(!is_port_valid(PORT)) return 1;
                break;
            case 'd':
                ROOT = optarg;
                break;
            default:
                printf(" Niepoprawne argumenty \n");
                printf(" Poprawne argumenty to [-s|-q] [-p port] [-d document_root]\n");
                return -1;
        }
    }
    if(start){
        
        if(daemon(-1, 0) == -1){
            perror("Błąd podczas tworzenia demona");
            return 1;
        } 
        
    
        //Dodanie .log i usuniecie dwoch pierwszych znakow ./ 
        
        strcpy(filename,argv[0]);
        memmove(filename, filename + 2, strlen(filename) - 1);
        strcpy(filename_org,filename);
        strcat(filename,".log");
        //printf("%s",filename);

        //Start serwera log początkowy testowy
        const char* ip = "127.0.0.1";
        const char* status = "Start serwera log testowy";
        char* request[2] = { "GET /index.html", "Request details" };
        Log(ip, status, request);
        Initialize();
        

    }
    if(stop){
        printf("Zatrzymanie serwera");
        char command[512];
        snprintf(command, sizeof(command), "pkill -o -f %s", argv[0]);
        int status = system(command);
        if (status == -1) printf("Błąd podczas zatrzymywania pracy serwera \n sprawdź czy serwer jest uruchomiony ");

    }

}
// Funkcja sprawdzająca poprawność portu
bool is_port_valid(char *port_org){
    int port = atoi(port_org);
    if(port>0 && port<=65535){
        return true;
    }
    else{
        return false;
    }
}
// Funkcja sprawdzająca dostępność katalogu
bool is_directory_readable(char *dir) {
    struct stat st;
    if (stat(dir, &st) < 0) {
        return false;
    }
    return S_ISDIR(st.st_mode) && access(dir, R_OK) == 0;
}
// Funkcja do zapisu logów
void Log(const char *ip, const char *status, char *const *request) {
    time_t rawtime;
    time(&rawtime);
    char timestamp[50];
    char pid[50];
    char *message = malloc(sizeof(char) * 256);
    snprintf(timestamp, sizeof(timestamp), "%s", strtok(ctime(&rawtime), "\n"));
    snprintf(pid, sizeof(pid), "%lu", pthread_self());
    snprintf(message, sizeof(char) * 256, "%s\t%s\t%s\t%s\t%s\t%s\n",
        timestamp, pid, ip, status, request[0], request[1]);


    FILE* file = fopen(filename, "a");
    if (file == NULL) {
        printf("Nie można otworzyć pliku log.\n");
        free(message);
        return;
    }

    fprintf(file, "%s", message);
    fclose(file);
    free(message);
}
//Funkcja inijalizująca serwer
void Initialize() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Błąd inicjalizacji gniazda.\n");
        Log("127.0.0.1", "Błąd inicjalizacji gniazda", (char*[]){"-", "-"});
        exit(1);
    }

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(PORT));
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        close(sock);
        printf("Błąd wiązania gniazda.\n");
        Log("127.0.0.1", "Błąd wiązania gniazda", (char*[]){"-", "-"});
        exit(1);
    }

    listen(sock, 5);
    printf("Serwer nasłuchuje na porcie %s...\n", PORT);
    while (1) {
        struct sockaddr_in clientAddr = {0};
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSock = accept(sock, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSock < 0) {
            perror("Błąd akceptacji połączenia");
            continue;
        }

        thread_data_t *data = malloc(sizeof(thread_data_t));
        data->sock = clientSock;
        data->clientAddr = clientAddr;
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, data) != 0) {
            perror("Błąd tworzenia wątku");
            free(data);
        }
    }

}
// Funkcja do obsługi żądań klienta
void *handle_client(void *data) {
    char buffer[1024];
    int bytes;
    thread_data_t *thread_data = (thread_data_t *)data;

    // Odczytaj żądanie klienta
    bytes = read(thread_data->sock, buffer, sizeof(buffer) - 1);
    if (bytes < 0) {
        perror("Błąd odczytu żądania");
    } else {
        buffer[bytes] = '\0';
        printf("Otrzymane żądanie: %s\n", buffer);

        // Obsługa żądania GET
        char method[5];
        char url[1024];
        sscanf(buffer, "%s %s", method, url);
        if (strcasecmp(method, "GET") != 0) {
            write(thread_data->sock, "HTTP/1.1 501 Not Implemented\r\n\r\n", 33);
        } else {
            if (url[strlen(url) - 1] == '/') {
                strcat(url, "index.html"); // domyślna strona, jeśli nie jest określona
            }

            char filePath[1024];
            strcpy(filePath, ROOT);
            strcat(filePath, url);

            int file = open(filePath, O_RDONLY);
            if (file == -1) {
                // Plik nie istnieje, zwróć błąd 404
                write(thread_data->sock, "HTTP/1.1 404 Not Found\r\nConnection: keep-alive\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Error 404: File not found</h1></body></html>", 132);
                pthread_mutex_lock(&lock);
                char* request[2];
                char* url_copy = strdup(url);
                request[0] = "GET";
                request[1] = url_copy;
                Log(inet_ntoa(thread_data->clientAddr.sin_addr), "404 Not Found", request);
                free(url_copy);
                pthread_mutex_unlock(&lock);
                } else {
                // Plik istnieje, zwróć go klientowi

                const char *mime_type = get_mime_type(filePath);
                char response[1024];
                snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Type: %s\r\n\r\n", mime_type);
                write(thread_data->sock, response, strlen(response));
                char fileBuffer[1024];
                int bytesRead;
                while ((bytesRead = read(file, fileBuffer, sizeof(fileBuffer))) > 0) {
                    write(thread_data->sock, fileBuffer, bytesRead);
                }
                close(file);
                 pthread_mutex_lock(&lock);
                char* request[2];
                char* url_copy = strdup(url);
                request[0] = "GET";
                request[1] = url_copy;
                Log(inet_ntoa(thread_data->clientAddr.sin_addr), "200 OK", request);
                free(url_copy);
                pthread_mutex_unlock(&lock);
            }
        }
    }

    close(thread_data->sock);
    free(data);
    pthread_exit(NULL);
}
//obsluga mime
const char *get_mime_type(char *filename) {
    char *ext = strrchr(filename, '.');
    if (!ext) {
        return "application/octet-stream";
    } else if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
        return "text/html";
    } else if (strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".jpg") == 0) {
        return "image/jpeg";
    } else if (strcmp(ext, ".png") == 0) {
        return "image/png";
    } else if (strcmp(ext, ".gif") == 0) {
        return "image/gif";
    } else {
        return "application/octet-stream";
    }
}