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
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "protocolc.h"
#include "client.h"

/* get sockaddr, IPv4 or IPv6: */
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[]) {

    request_t request;
    char req_as_stream[REQLEN]; /* encoding the stream as 16 bytes stream */
    memset(req_as_stream, 0, REQLEN);
    char res_as_stream[RESLEN]; /* encoding the stream as 16 bytes stream */
    memset(res_as_stream, 0, RESLEN);
    /* nah i'd use a separate parser here so it looks nicer */
    int parseRes = parseArguments(argc, argv, &request);
    if (parseRes != 0) {
        fprintf(stderr, "Error parsing arguments, return code: %d\n", parseRes);
        return -1;
    }


    /* TCP stuff from here on */
    struct addrinfo hints, *servinfo, *p;
    int sockfd; int rv;
    int bytesSent, bytesRecv;
    char s[INET6_ADDRSTRLEN];
    const char* hostname = argv[1];
    const char* port = argv[2];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    /* loop through all the results and connect to the first we can */
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, 
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return -2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), 
                s, sizeof s);
    printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo);

    if((bytesSent = send(sockfd, req_as_stream, REQLEN, 0)) == -1) {
        perror("send");
        close(sockfd);
        return -3;
    }
    
    if (bytesSent < REQLEN) {
        /* handle partial send here */
    }

    /* at this point, the request is sent,
        and we are waiting for the response */
    response_t res;
    processSingleRes(sockfd, res_as_stream, &res);
    /*
        there might be more responses, 
        needs to check and handle them.

        the first response would have a field `numNext`
        indicating how many more responses to expect.

        Since we only have 5 bits there, the max `numNext`
        is 31. So in case `numNext` is RESBATCHSIZE, we need to check
        that last response as well to see if there are more batches.
    */
    int numNext = res.numNext; 
    /* code below should not execute if numNext is 0 */
    bool isLastBatch; 
    do{
        isLastBatch = (numNext < RESBATCHSIZE);
        for (int i = 0; i < numNext; i++) { /* if numNext is 0, no-op */
            processSingleRes(sockfd, res_as_stream, &res);
        }
        /* here, res holds the last response of this batch, 
            check if there are more batches */
        numNext = res.numNext;
    } while (!isLastBatch);

    /*
     On a second thought, this is still bad as we are still reading
        only 1 response at a time. The batch property is not utilized
        at all. We should have a:
        processBatchRes(int sockfd, char** reses_as_stream, int batchSize,
                        response_t** reses);
        function that reads and processes a batch of responses at a time,
        so directly reading a batch from TCP.

        TODO: I'll do this if I have time.
    */

    /*
        at this point, all responses are processed.
        Nothing else to do, so close the socket and finish.
    */
    if (close(sockfd) == -1) {
        perror("close");
        return -4;
    }
    return 0;
}

/*
 * Process a single response from the server:
    Given a socket file descriptor, process a single response from
        the server, display the response, and fill response_t struct.
 * args:
    - sockfd: the socket file descriptor
    - res_as_stream: the response as a stream of bytes
    - pRes: a pointer to the response_t struct to be filled
 * returns:
    - 0 on success
    - -4 on recv error
    - -5 on parsing error
    - -6 on conversion error
*/
int processSingleRes(int sockfd, char* res_as_stream, response_t *pRes) {
    int bytesRecv;
    if ((bytesRecv = recv(sockfd, res_as_stream, RESLEN, 0)) == -1) {
        perror("recv");
        close(sockfd); return -4;
    }

    if (bytesRecv < REQLEN) {
        /* handle partial recv here */
    }

    if ((streamToRes(res_as_stream, pRes)) != 0) {
        fprintf(stderr, "Error parsing response\n");
        close(sockfd); return -5;
    }
    char respond_str[256];
    if ((resToStr(*pRes, respond_str)) != 0) {
        fprintf(stderr, "Error converting response to string.\n");
        close(sockfd); return -6;
    }
    printf("%s\n", respond_str);
    return 0;
}

/*
 This one is bad... 
 If a procedure is longer than 80 lines, it's probably fucked....
*/
int parseArguments(int argc, char* argv[], request_t* request) {
    // Parse command-line arguments
    // [Argument parsing code here]
    /*
    * allowed operations:
    Adding an event:    
    Removing an event:  
    Updating an event:  
    Getting an event 1: 
    Getting an event 2: 
    Getall events: 
    ./myCal hostname port myName add    date start-time end-time Event-title |8
    ./myCal hostname port myName remove date start-time                      |6
    ./myCal hostname port myName update date start-time end-time Event-Title |8
    ./myCal hostname port myName get    date time                            |6
    ./myCal hostname port myName get    date                                 |5
    ./myCal hostname port myName getall                                      |4
    */

    if (argc < 5) {
        fprintf(stderr, "usage: %s hostname port myname operation [args]\n", argv[0]);
        return -1;
    }
    /* at this point, at least 5 arguments are present:
    * 1. hostname -> this can't fail
    * 2. port -> this has to be a valid port number (MIN_PORT, MAX_PORT)
    * 3. myName -> this can't fail
    * 4. operation -> this has to be one of:
    *   add, remove, update, get, getall
    * 5. args -> to be checked later
    */
    request->hostname = argv[1];  /* this can't fail */
    request->port = atoi(argv[2]); /* need to be checked, between MIN_PORT and MAX_PORT */
    if (request->port < MIN_PORT || request->port > MAX_PORT) {
        fprintf(stderr, "Invalid port number: %d\n\
            Port number must be between %d and %d\n", request->port, MIN_PORT, MAX_PORT);
        return -2;
    }
    request->myName = argv[3]; /* this requires to be at most 5 characters long,
                                  only alphanumeric, '-' and '_' */
    if (strlen(request->myName) > 5) {
        fprintf(stderr, "Invalid name: %s\n\
            Name must be at most 5 characters long\n", request->myName);
        return -2;
    }
    for (unsigned int i = 0; i < strlen(request->myName); i++) {
        if (!isalnum(request->myName[i]) 
            && request->myName[i] != '-' 
            && request->myName[i] != '_') {
            fprintf(stderr, "Invalid name: %s\n\
                Name must contain only alphanumeric characters, '-' or '_'\n", 
                request->myName);
            return -2;
        }
    }

    /* at this point, the first 4 arguments are parsed correctly .
      now we need to parse the operation based on the ALLOWED_COMMANDS macro */

    char* cmd_inp = argv[4]; /* this need to be one of the allowed operations */

    /* check against the ALLOWED_COMMANDS macro, has to be one of the allowed commands
    * check it in a loop, if it's not found, return -3
    */
    bool found = false;
    int cmd_nu; /* probably no need for this var, 
                   just to show how powerful my reversed-Dullahan loop can be */

    for (
        int i = 0;

        /* use the condition to do action, fun begin :P */
        (i < NUM_COMMANDS) 
        &&   /* keeps looping when: 
                - there are more commands to check 
                - the command is not find a match */
        !(  /* assign found <- bool */
            (found= (strcmp(cmd_inp, ALLOWED_COMMANDS[i])==0)) 
            && /* above line is bool::Found, only execute next if TRUE */
            ( cmd_nu=i, true /* assign cmd_nu, eval it to T */   
            )
         ) /*only proceed if not match */
        ; 
        i++
    ); /*
        here is an example of a loop without a body.
            I call that "reverse-Dullahan loop" because it's like Dullahan, 
            but instead of having no head, this loop has no body.

            lol.
            I have a theory that all loops can be written in this form.
            The only thing it can't 
            (this is bad practice in the way that is hard to
                -> debug -> maintain -> scale
             and is NOT meant to be efficient as the complexity prevents
             compiler to do some optimization....
             It's just fun
             )
        */
    if (!found) {
        fprintf(stderr, "Invalid operation: %s\n\
            Operation must be one of: add, remove, update, get, getall\n", cmd_inp);
        return -3;
    }
    /* 
    * at this point, the first 5 arguments are parsed correctly .
      now we need to parse the rest of the arguments based on the operation
      based on cmd_map[cmd_nu] case-by-case basis    */
    request->cmd = cmd_map[cmd_nu];
    char* fErr_badArgLen 
        = "Bad argument length for operation %s, expected %d, got %d\n";
    switch (request->cmd) {
        case ADD: 
        /* args => date start-time end-time Event-title |8 */
            if (argc != 9) {
                fprintf(stderr, fErr_badArgLen, ALLOWED_COMMANDS[cmd_nu], 4, argc-5);
                return -4;
            }
            /*
            * 1. date -> this has to be in the format YYYY-MM-DD
            * 2. start-time -> this has to be in the format HH:MM
            * 3. end-time -> this has to be in the format HH:MM
            */
        
        case REMOVE:
        /* args => date start-time |6 */
            if (argc != 7) {
                fprintf(stderr, fErr_badArgLen, ALLOWED_COMMANDS[cmd_nu], 2, argc-5);
                return -4;
            }

        case UPDATE:
        /* args => date start-time end-time Event-Title |8 */
            if (argc != 9) {
                fprintf(stderr, fErr_badArgLen, ALLOWED_COMMANDS[cmd_nu], 4, argc-5);
                return -4;
            }

        case GET:
        /* args => date time |6
           args => date |5 */
            if (argc != 6 && argc != 5) {
                fprintf(stderr, fErr_badArgLen, ALLOWED_COMMANDS[cmd_nu], 2, argc-5);
                fprintf(stderr, "Or\n");
                fprintf(stderr, fErr_badArgLen, ALLOWED_COMMANDS[cmd_nu], 1, argc-5);
                /* yea just realized it's not accurate, it can be both 5 and 6 */
                return -4;
            }

        case GETALL:
        /* args => none |4 */
            if (argc != 5) {
                fprintf(stderr, fErr_badArgLen, ALLOWED_COMMANDS[cmd_nu], 0, argc-5);
                return -4;
            }
    } 
    // Fill in request structure
    // [Structure filling code here]

    return 0;
}

int formatDate(char* date, date*);