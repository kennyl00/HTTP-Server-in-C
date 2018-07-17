#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFERSIZE 1024
#define REQ_Q 5


void error_msg(char *msg){
    fprintf(stderr, "%s", msg);
    exit(EXIT_FAILURE);

}


int service_request(int newsockfd, char *path){
    char buffer[BUFFERSIZE];    /* message buffer */
    char *version;
    char *uri;  /* filetype from path */
    char filename[BUFFERSIZE];
    char filetype[BUFFERSIZE];
    char *temp;
    int n;
    int resource = 0;
    char response[BUFFERSIZE];
    char c;




    /* Read the Request Header */
    n = read(newsockfd, buffer, BUFFERSIZE);
    if (n < 0){
        error_msg("ERROR on read\n");
    }

    /* Check if it's the incoming Request is a GET Request */
    if(!strncmp(buffer, "GET ", 4)){

        /* Get both URI and HTTP Version from buffer*/
        temp = strstr(buffer, "/");
        int temp_length  = strlen(temp);
        version = strstr(temp, " ");
        version++;
        int version_length = strlen(version);
        int uri_length = temp_length - version_length;

        /* Arrays didn't work, hence the use of malloc */
        uri = malloc(uri_length - 1);
        strncpy(uri, temp, uri_length - 1);

    /* If it is not a GET Request */
    }else{
       sprintf(response, "HTTP/1.0 501 Not Implemented\r\n\r\n");
       write(newsockfd, response, strlen(response));
       exit(EXIT_FAILURE);

   }

    /* Get the Filename from URI and Path */
    strcpy(filename, path);
    strcat(filename, uri);

    /* Get the Filetype from URI */
    if(strstr(filename, ".html")){
        strcpy(filetype, "text/html");

    }else if(strstr(filename, ".jpg")){
        strcpy(filetype, "image/jpeg");

    }else if(strstr(filename, ".css")){
        strcpy(filetype, "text/css");

    }else if(strstr(filename, ".js")){
        strcpy(filetype, "application/javascript");

    }else{
        strcpy(filetype, "text/plain");
    }

    /* Check if the File exist */
    if((resource = open(filename, O_RDONLY)) < 0){
        sprintf(response, "HTTP/1.0 404 Not Found\r\nContent-Type: %s\r\n\r\n"
        "<html><body>404 NOT FOUND</body></html>\r\n", filetype);
        write(newsockfd, response, strlen(response));
        exit(EXIT_FAILURE);

    }else{
        /* Response Header */
        sprintf(response, "HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n",
        filetype);
        write(newsockfd, response, strlen(response));

    }

    /* Return resource */
    while((n = read(resource, &c, 1))){
        if(n < 0){
            error_msg("ERROR reading from file\n");
        }

        if(write(newsockfd, &c, 1) < 1){
            error_msg("ERROR sending file\n");
        }
    }

    /* Close the resource */
    if(resource > 0){
        if(close(resource) < 0){
            error_msg("ERROR closing resource\n");
        }
    }

    return 0;
}


int main(int argc, char *argv[]){
	int sockfd, newsockfd;
    int portno;
	struct sockaddr_in serv_addr;
    char *path;
    pid_t pid;
    int optval;

    /* Validating Argument Line */
    if(argc < 2){
        error_msg("ERROR argument line invalid\n");

    }else{
    	portno = atoi(argv[1]);
        path = argv[2];

    }

    /* Create a socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
        error_msg("ERROR on opening socket\n");
    }

    optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval ,
    sizeof(int));

    /* Populate Socket Address structure */
    memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

    /* Bind address to the socket */
	if (bind(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		error_msg("ERROR on binding\n");
	}

	/* Make socket listen */
    if (listen(sockfd, REQ_Q) < 0){
        error_msg("ERROR on listening socket\n");
    }

    while(1){

        /* Accept a Connection Request */
        newsockfd = accept(sockfd, NULL, NULL);
        if(newsockfd < 0){

            error_msg("ERROR on calling accept\n");
        }

        /* Parent process creates a Child process */
        if((pid = fork()) == 0){

            if(close(sockfd) < 0){
                error_msg("ERROR closing socket\n");
            }

            /* Succssful child process service request */
            service_request(newsockfd, path);

            if(close(newsockfd) < 0){
                error_msg("ERROR closing new socket\n");
            }

            /* Completed Child Process */
            exit(EXIT_SUCCESS);

        }

        if(close(newsockfd) < 0){
            error_msg("ERROR closing new socket in socket\n");
        }

        waitpid(-1, NULL, WNOHANG);
    }

    return EXIT_FAILURE;
}
