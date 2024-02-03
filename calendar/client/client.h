
/* gcc specific optimization... */
#define GCC_PACKED __attribute__((packed))

/* define some magic numbers */
#define MIN_PORT 25000
#define MAX_PORT 25099

#define CMD_ADD    "add"
#define CMD_REMOVE "remove"
#define CMD_UPDATE "update"
#define CMD_GET    "get"
#define CMD_GETALL "getall"
#define NUM_COMMANDS 5
#define ALLOWED_COMMANDS (char*[]){CMD_ADD, CMD_REMOVE, CMD_UPDATE, CMD_GET, CMD_GETALL}

/*
 * use the command as enum, since that makes the most readable,
 * also the packed attribute to ensure smallest possible size.
 * */
enum command {ADD, REMOVE, UPDATE, GET, GETALL} GCC_PACKED;
static const enum command cmd_map[]={
	ADD, REMOVE, UPDATE, GET, GETALL };

struct request{
    char* hostname;
    int port;
    char* myName;
    enum command cmd;
    time_t startTime;
    time_t endTime;
};

typedef struct request request_t;

int parseArguments(int argc, char* argv[], request_t* req);
