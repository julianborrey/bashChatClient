/* client.c
 * by Julian Borrey
 * 01/12/13
 * Client to connect to UDP server to find who else is online.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define SERVER_PORT 2000
#define SEND_BUF_LEN 1024
#define RECV_BUF_LEN 1024
#define ENTER_KEY 10
#define CLIENT_KEY 38294

int main(int argc, char* argv[]){
   printf("Running client %s.\n", argv[1]);
   
   /*
    * AF_INET refers to the usual socket families for internet.
    * SOCK_DGRAM refers to UDP.
    */

   int sockNo;             //the socket we are using
   sockNo = socket(AF_INET, SOCK_DGRAM, 0); //get socket
   printf("We got sockNo = %d\n", sockNo);
   if(sockNo < 0){         //if we get an invalid socket
      perror("We have an error: ");
      return EXIT_FAILURE; //terminate program
   }
   
   int cliPort = 4030; //getCliPort(argv[1]); //get the port based on name
   
   struct sockaddr_in cliAddr;
   cliAddr.sin_family = AF_INET;
   cliAddr.sin_port = htons(cliPort); //port based on name
   //to allow for multiple clients on the same machine

   struct sockaddr_in servAddr;   //this is the server's address
   servAddr.sin_family = AF_INET; //set to UDP server

   //htonl() converts IP (a long) to big-endian number
   //INADDR_ANY (0) will allow packets arriving to any interface
   //to go to the server
   servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servAddr.sin_port = htons(SERVER_PORT); //set the port

   //try to bind the specified address to our socket
   //if failure it returns -1
   if(bind(sockNo, (struct sockaddr*) &cliAddr, sizeof(cliAddr)) < 0){
      perror("We have an error: ");
      close(sockNo);
      return EXIT_FAILURE;
   }
   
   char sendBuf[SEND_BUF_LEN];
   char recvBuf[RECV_BUF_LEN];
   int addrLen = sizeof(servAddr);
   char currChar;

   bool sending = true;
   while(sending){
      printf("Type message to send: ");
      memset(&sendBuf, 0, SEND_BUF_LEN); //clear buffer
      
      currChar = getc(stdin);            //get message to send
      int i = 0;

      //while we done hit enter and less than max buff (-1 for '\0')
      while(((int)currChar != ENTER_KEY) && (i < (SEND_BUF_LEN - 1))){
         sendBuf[i] = currChar;
         i++;
         currChar = getc(stdin);
      }
      sendBuf[i] = '\0';
      
      //if typed in command to end client, end client
      if(strcmp(sendBuf, "END CLIENT") == 0){
         sending = false;
      } else { //send message to server
         printf("Sending \"%s\"\n", sendBuf);
         sendto(sockNo, sendBuf, SEND_BUF_LEN, 0, (struct sockaddr*) &servAddr, addrLen);
      }
   }
   
   close(sockNo);
   printf("Ending client %s.\n", argv[1]);
   return EXIT_SUCCESS;
}
