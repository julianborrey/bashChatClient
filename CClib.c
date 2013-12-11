/* 
 * CClib.c
 * by Julian Borrey
 * 01/12/13
 * Library for the ConnectedClient linked list structures.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include "chatProg.h"

//push client into linked list
void push(CC reg, char* name, struct sockaddr_in cliAddr){
   CC current = reg;
   while(current->next != NULL){ //iterate to the last
      current = current->next;
   }
   
   //make new node
   CC newCli = (CC) malloc(sizeof(struct ConnectedClient));
   newCli->next = NULL; //its the end of the list
   strcpy((char*) &(newCli->name), name); //set name
   memcpy((void *) &(newCli->addr), (void *) &cliAddr, sizeof(cliAddr)); 
   current->next = newCli; //connect to old tail
   return;
}

//remove the client from the register by name
//true/false for success/failure
bool removeCli(CC reg, char* name){
   CC current = reg;
   CC prev = NULL;

   //iterate until finding name or till end
   while((current != NULL) && (strcmp(name, current->name) != 0)){
      prev = current;
      current = current->next;
   }
   
   if(strcmp(name, (const char*) &current->name) == 0){ //if found the client
      prev->next = current->next; //cut out this node
      free(current); //give memory back to OS
      return true;
   }
   
   return false;
}

//give client at position i (index from 0)
char* get(CC reg, int i){
   CC current = reg;
   int j = 0;
   while((current != NULL) && (j < i)){
      current = current->next;
      j++;
   }
   
   if(current != NULL){
      int sizeRetStr = strlen(current->name) + ADDR_STR_LEN + 1;
      //+1 for EOS char

      char* clientInfo = (char*) malloc(sizeof(char) * sizeRetStr);
      sprintf(clientInfo, "%s has IP %s and is on port %d.", current->name, 
              inet_ntoa((current->addr).sin_addr), ntohs((current->addr).sin_port));
      return clientInfo;
   }
   
   return END_LIST;
}

//get address of i'th client
struct sockaddr_in* getAddr(CC reg, int i){
   CC current = reg;
   int j = 0;
   while((current != NULL) && (j < i)){
      current = current->next;
      j++;
   }

   if(current != NULL){
      return &(current->addr);
   }
   
   return NULL;
}

//get the name given an address - based on port number
char* getName(CC reg, struct sockaddr_in* addrCheck){
   CC current = reg;
   
   //while not at end and not correct address port
   while((current != NULL) && (((current->addr).sin_port) != (addrCheck->sin_port))){
      current = current->next; //iterate
   }
   
   //if we got address, send back name
   if(current != NULL){
      return current->name;
   }
   
   //else some random connected to the client
   char* random = (char*) malloc(sizeof(char) * SIZE_OF_UNKNOWN);
   strcpy(random, "UNKNOWN CLIENT");
   return random;
}

//free's all list memory
void clearList(CC reg){
   CC current = reg;
   CC currNext = NULL;
   
   while(current != NULL){ //iterate through list
      currNext = current->next; //save the next pointer
      free(current);            //free current struct
      current = currNext;       //go to next
   }
   return;
}
