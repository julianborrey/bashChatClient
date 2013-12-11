/*
 * chatProg.h
 * by Julian Borrey
 * 01/12/13
 * The interfaceing header file for the chat server and client.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define SERVER_PORT 2000
#define BUF_SIZE 1024
#define SMALL_BUF_SIZE 128
#define HELLO_REQUEST "SEND REGISTER"
#define BYE_REQUEST   "I AM LEAVING!"
#define REQUEST_SIZE 13 //num chars in request string
#define BAR "============================="
#define END_LIST "END OF LIST"
#define SIZE_OF_UNKNOWN 15 //str len for "unknown client"
#define ADDR_STR_LEN 45 //addition of max IP and port str
//<name> has IP ---.---.---.--- and is on port -----.

//define pointer to linked list node
typedef struct ConnectedClient* CC;

//stucture of linked list node
typedef struct ConnectedClient {
   char name[SMALL_BUF_SIZE];
   struct sockaddr_in addr;
   CC next;
};

//push client into linked list
void push(CC reg, char* name, struct sockaddr_in cliAddr);

//remove the client from the register by name
bool removeCli(CC reg, char* name);

//give client at position i
char* get(CC reg, int i);

//get the address of the i'th client
struct sockaddr_in* getAddr(CC reg, int i);

//get the name given an address - based on port number
char* getName(CC reg, struct sockaddr_in* addr);

//list destructor
void clearList(CC reg);
