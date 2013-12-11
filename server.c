/* 
 * server.c
 * by Julian Borrey
 * 01/12/13
 * A UPD server to give a list of clients that are online.
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
#include <time.h>
#include "chatProg.h"

#define WAIT_TO_RESPOND 1 //in seconds, wait for client to get read to receive

void wait(int t); //wait t seonds

int main(int argc, char* argv[]){
   printf("\n" BAR " UDP Chat Server Starting " BAR "\n\n");
   
   /*
    * AF_INET refers to the usual socket families for internet.
    * SOCK_DGRAM refers to UDP.
    */
   
   printf("Server setting up ...\n");

   int sockNo;             //the socket we are using
   printf("Fetching socket ... ");
   sockNo = socket(AF_INET, SOCK_DGRAM, 0); //get socket
   printf("sockNo = %d\n", sockNo);
   if(sockNo < 0){         //if we get an invalid socket
      perror("We have an error");
      return EXIT_FAILURE; //terminate program
   }
   
   struct sockaddr_in servAddr;   //this is the server's address
   servAddr.sin_family = AF_INET; //set to UDP server
   
   //htonl() converts IP (a long) to big-endian number
   //INADDR_ANY (0) will allow packets arriving to any interface
   //to go to the server
   servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servAddr.sin_port = htons(SERVER_PORT); //set the port
   
   //try to bind the specified address to our socket
   //if failure it returns -1
   printf("Binding socket to port ... ");
   if(bind(sockNo, (struct sockaddr*) &servAddr, sizeof(struct sockaddr)) < 0){
      perror("We have an error");
      close(sockNo);
      return EXIT_FAILURE;
   } else {
      printf("socket binding successful.\n");
   }

   struct sockaddr_in cliAddr; //the address of the client we receive from
   int cliAddrLen = sizeof(cliAddr);
   memset(&cliAddr, 0, cliAddrLen); //set all values to 0

   //begin listening for packets
   char buf[BUF_SIZE]; //buffer for data received
   char requestBuf[SMALL_BUF_SIZE];
   char nameBuf[SMALL_BUF_SIZE];
   
   //this linked list is the register of who is an isn't connected
   CC reg = (CC) malloc(sizeof(struct ConnectedClient)); //acts as head
   strcpy(reg->name, "CLIENT LIST");
   reg->next = NULL;
   
   bool listen = true;
   printf("Beginning to listen on port %d:\n", SERVER_PORT);
   while(listen){
      memset(buf, 0, BUF_SIZE);
      memset(requestBuf, 0, SMALL_BUF_SIZE);
      memset(nameBuf, 0, SMALL_BUF_SIZE);

      recvfrom(sockNo, buf, BUF_SIZE, 0, (struct sockaddr*) &cliAddr, (socklen_t*) &cliAddrLen);
      if(strcmp(buf, "END SERVER") == 0){ //if client says shutdown server
         listen = false;             //we break the loop
      }
      
      printf("\n"
             "\tIncoming packets from IP address %s on port %d\n"
             "\t\tRecieved \"%s\"\n",
             inet_ntoa(cliAddr.sin_addr), cliAddr.sin_port, buf);
      
      //extract request type and name from buf
      strncpy(requestBuf, buf, REQUEST_SIZE);
      strncpy(nameBuf, &buf[REQUEST_SIZE+1], (SMALL_BUF_SIZE - 1));
      
      //if valid request message, send register of clients
      if(strcmp(requestBuf, HELLO_REQUEST) == 0){
         //first we record thie presence of client
         printf("\t\t<<< Adding %s to client list. >>>\n", nameBuf);
         push(reg, nameBuf, cliAddr);

         //send back the list of other users
         int i = 1;
         char* current = get(reg, i); //retrives name, ip, port
         
         printf("\t\tSending back list after waiting %d second(s):\n", WAIT_TO_RESPOND);
         wait(WAIT_TO_RESPOND); //adds theatrical effect

         while(strcmp(current, END_LIST) != 0){ //have to ask for each client until end of list
            printf("\t\t\t%s\n", current);      //print current list entry and send
            sendto(sockNo, current, strlen(current), 0, (struct sockaddr*) &cliAddr, sizeof(cliAddr));
            i++;
            current = get(reg, i);              //get next list entry
         }
         printf("\t\t\t%s\n", current);         //print and send end of list str
         sendto(sockNo, current, strlen(current), 0, (struct sockaddr*) &cliAddr, sizeof(cliAddr));

      } else if(strcmp(requestBuf, BYE_REQUEST) == 0){ //client leaving
         //update logfile to show client left
         printf("\t\t<<< Removing %s from list ... ", nameBuf);
         bool removeCliResult = removeCli(reg, nameBuf);
         
         if(removeCliResult){
            printf("success. >>>\n");
         } else {
            printf("failure. Client not found. >>>\n");
         }
         
      } else {
         printf("\t\tInvalid reuqest.\n");
      }
   }
   
   close(sockNo); //close socket to end all connections
   printf("\n" BAR " Ending UDP Chat Server " BAR "\n\n");
   return EXIT_SUCCESS;
}

void wait(int t){ //wait t seconds
   float start = clock()/CLOCKS_PER_SEC;
   while((start + (float)t) > (clock()/CLOCKS_PER_SEC)){}
   return;
}
   
