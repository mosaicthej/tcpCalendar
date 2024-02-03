#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "protocol.h"
#include "client.h"

int main(int argc, char* argv[]) {

    request_t request;
    /* nah i'd use a separate parser here so it looks nicer */
    int parseRes = parseArguments(argc, argv, &request);
    if (parseRes != 0) {
        fprintf(stderr, "Error parsing arguments, return code: %d\n", parseRes);
        return -1;
    }
    // Establish TCP connection to server
    // [Socket creation, connection code here]

    // Format request based on operation and arguments
    // [Call to protocol formatting function]

    // Send request to server
    // [Send function call]

    // Receive response from server
    // [Receive function call]

    // Process and display response
    // [Response processing code]

    // Close connection
    // [Socket close code]

    return 0;
}

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
    request->myName = argv[3]; /* this can't fail */
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
