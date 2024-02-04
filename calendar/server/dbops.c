#include "server.h"
#include <list.h>
/* do -llist on linker */

/*
    Doing the database operations for each request.
    
    The lock state would be transparent to dbops.
*/

int db_newUser(char *username, LIST **users) {
    if (users == NULL) {
        return -1;
    }
    if ((ListSearch(users, strcmp, username)) != NULL) {
        perror("User already exists");
    } else {
        ListInsert(users, username);
        return 0;
    }
}

/*
    Given a username and list of users, find the user and return NULL if not found.
*/
LIST* db_findUser(char *username, LIST **users) {
    if (users == NULL) {
        return NULL;
    }
    userlists = ListSearch(users, strcmp, username);
    if (userlists == NULL) {
        perror("User not found");
        return NULL;
    } else {
        return userlists;
    }
}

/*
    Given a user's table, find the event by date and time.
    Returns status code.
    On success, will load the event into the event struct.
*/
int db_findEvent_DT(LIST *userlist, int clDate, int startTime, char * eventName, clEvent_t *event);


/*
    Given a user's table, find all events by date only
    Return a linked list of events since there can be multiple events.
    Returns number of events found or -1 if error.

    In case of more than 1 event should be returned, the events struct will be like
    a linked list, the `next` pointer will point to the next event.
*/
int db_findEvent_D(LIST *userlist, int clDate, clEvent_t *event);

/* 
    Given a user's table, find all events for that user.
    Return the number of events found, or -1 if error.
    The events struct will be be loaded like a linked list, 
    the `next` pointer will point to the next event.
*/
int db_getall(LIST *userlist, clEvent_t *event);

/*
    Given a user's table, add an event to the user's table.
    Note that we need to maintain the order of the events,
    also, look ahead to see if the event is conflicting with any other event.
    Return 0 on success, -1 on error.
    On success, the event will be added to the user's table.
    On error, the conflicting event will be loaded into the event struct.
    (so always make a copy of the event struct before calling this function)
*/
int db_addEvent(LIST *userlist, clEvent_t *event);

/*
    Given a user's table, delete an event from the user's table.
    Return 0 on success, -1 on error.
    On success, the event will be deleted from the user's table.
    On error, the event will be loaded into the event struct.
    (so always make a copy of the event struct before calling this function)
*/
int db_delEvent(LIST *userlist, clEvent_t *event);