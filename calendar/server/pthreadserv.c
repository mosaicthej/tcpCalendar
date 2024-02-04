/*
---
- Name: Mark Jia
- NSID: mij623
- StuN: 11271998
---
Minimal modification from Beej's Guide to Network Programming
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>



#include <pthread.h> /* threading */
#include <semaphore.h> /* semaphores */

#define REQLEN 16
#define RESLEN 13

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int childThread(LIST* userslist, char* reqbuf) {
    res_t res;
    int numRes = processSingleReq(reqbuf, &res);
    if (numRes == -1) {
        perror("Error processing request");
        return -1;
    }
    // send the response
    int nbytes = send(newfd, res, RESLEN, 0);
    if (nbytes == -1) {
        perror("send");
    }
    return 0;
}


int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

        /* once accept, make a new pthread to handle the request */
        pthread_t tid;
        pthread_create(&tid, NULL, childThread, (void *)new_fd);
        
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}
int processSingleReq(char *reqbuf, res_t *res) {
    // process the request
    request_t req;
    streamToReq(reqbuf, &req);
    // fill the response buffer
    int numRes = reqToRes(&req, &res);
    /*
        there might be more than 1 response.
        and numRes might be -1 if error.
    */
    return numRes;
}

/* child thread to handle the request */
void *childThread(void *arg) {
    int newfd = (int)arg;
    char reqbuf[REQLEN];
    int nbytes = recv(newfd, reqbuf, REQLEN, 0);
    if (nbytes == -1) {
        perror("recv");
    }
    if (nbytes == 0) {
        printf("selectserver: socket %d hung up\n", newfd);
    }
    res_t res;
    int numRes = processSingleReq(reqbuf, &res);
    if (numRes == -1) {
        perror("Error processing request");
        return -1;
    }
    // send the response
    nbytes = send(newfd, res, RESLEN, 0);
    if (nbytes == -1) {
        perror("send");
    }
    return 0;
}