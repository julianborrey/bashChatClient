/* 
 * client.c
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
#include <time.h>
#include <sys/types.h>  
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/wait.h>
#include "chatProg.h"
#include <time.h>

#define ENTER_KEY 10
#define RESPONSE_WAIT 3
#define END_CONN "END CONNECTION"
#define ACK_MSG "--- Message Recieved ---\n"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define MEM_KEY 200
#define LEAVING_CHATROOM_MSG "Your friend left. Press ENTER to end the session.\n"
#define CONN_FANFAIR "\n********************************************************\n"
#define CONN_NOTICE "has connected to you!"

//gives a port number based on name
int getPortFromName(char* name);

//function to get UDP socket
int getUDPSocket(void);

//set up client address
void setCliAddr(int cliPort, int UDPsocket, struct sockaddr_in* cliAddr);

//setup server address
void setServAddr(struct sockaddr_in* servAddr);

//function to obtain and display list of UDP client server
void getClientList(CC reg, int UDPsocket, char* recvBuf);

//fundtion to register us with the UDP server
void registerClient(int sock, char* cliName, struct sockaddr* servAddr, char* sendBuf);

//function to remove client from UDP server
void deRegister(int sock, char* cliName, struct sockaddr* servAddr, char* sendBuf);

int main(int argc, char* argv[]){
   printf("\n" BAR " Running Chat Client %s. " BAR "\n\n", argv[1]);
   
   if(argc != 2){
      printf("ERROR: You must enter 1 name (no spaces) on the command line.\n");
      return EXIT_FAILURE;
   }
   
   int cliPort = getPortFromName(argv[1]); //get the client's port based on name

   printf("Setting up client for UDP... \n");
   int UDPsocket = getUDPSocket(); //obtain socket for UDP

   struct sockaddr_in cliAddr;
   setCliAddr(cliPort, UDPsocket, &cliAddr); //set up clients address with port
   
   struct sockaddr_in servAddr; //this is the server's address
   setServAddr(&servAddr);      //set up the server addrss
   
   /************************* Contact  UDP Server **********************/
   
   char sendBuf[BUF_SIZE];
   memset(&sendBuf, 0, BUF_SIZE);
   char recvBuf[BUF_SIZE];
   memset(&recvBuf, 0, BUF_SIZE);
   int addrLen = sizeof(servAddr);
   char currChar;
   
   bool using = true; //when using the program
   while(using){

      //register with UDP server
      registerClient(UDPsocket, argv[1], (struct sockaddr*) &servAddr, sendBuf);
      //now we expect to get a response which is a list of people
         
      //this linked list is the register of who is an isn't connected
      CC reg = (CC) malloc(sizeof(struct ConnectedClient)); //acts as head
      strcpy(reg->name, "CLIENT LIST");
      reg->next = NULL;
      
      //obtain and display list of clients from UDP server
      getClientList(reg, UDPsocket, recvBuf);

      /********************** TRANSITION TO TCP *********************************/
      
      printf("Would you like to wait for a connection or make a connection? (w/m): ");
      char waitOrMake = getc(stdin); 
      
      //get a new socket of TCP type
      int sockTCP = socket(AF_INET, SOCK_STREAM, 0);
      if(sockTCP < 0){ //if < 0 we have an error
         deRegister(UDPsocket, argv[1], (struct sockaddr*) &servAddr, sendBuf);
         perror("We have an error");
         return EXIT_FAILURE;
      }
      
      struct sockaddr_in tcpAddr = cliAddr;
      tcpAddr.sin_port = htons(ntohs(cliAddr.sin_port) + 1); //TCP port will be UDPport++;
      
      int nowSocket; //this will be the socket we use to communicate
      //may come from accepting a connection or starting a connection
      
      //we are now going to make a connection
      char* connectee = NULL; //the name of the person we connect to
      if(waitOrMake == 'm'){ //if the client wants to make a connection
         printf("Pick the numbered client you wish to talk to: ");
         int clientNum = 0;
         scanf("%d", &clientNum);
         
         //get address of other client
         struct sockaddr_in* cliOfInterest = getAddr(reg, clientNum);
         
         connectee = getName(reg, cliOfInterest);
         printf("Making connection to %s\n", connectee);
         //make connection to other client
         printf("Trying connection to socket %d\n", ntohs(cliOfInterest->sin_port));
         if(connect(sockTCP, (struct sockaddr*) cliOfInterest, sizeof(*cliOfInterest)) < 0){
            deRegister(UDPsocket, argv[1], (struct sockaddr*) &servAddr, sendBuf);         
            perror("We have an error");
            return EXIT_FAILURE;
         }
         
         nowSocket = sockTCP;
         printf("Connection made!\n");
         
         //notify conectee that I connected
         sprintf(sendBuf, "%s%s%s %s%s", CONN_FANFAIR, "\t\t", argv[1], 
                 CONN_NOTICE, CONN_FANFAIR);
         send(nowSocket, sendBuf, strlen(sendBuf), 0);
      } else {
         int currCliSocket; //this will be the socket we recieve a connection on  
         struct sockaddr_in cliOfInterest;
         int len = sizeof(cliOfInterest);
         
         //bind the TCP address (different port) to the TCP socket
         bind(sockTCP, (struct sockaddr*) &tcpAddr, sizeof(tcpAddr));
         printf("Now listening on port %d.\n", ntohs(tcpAddr.sin_port));
         listen(sockTCP, 1); //being listening for connections

         //if we get a connection we record the socket we are using and the address
         currCliSocket = accept(sockTCP, (struct sockaddr*) &cliOfInterest, &len);
         
         if(currCliSocket >= 0){
            nowSocket = currCliSocket;
            printf("Connection being made by another client ...\n");
            //name is send over TCP by client
         } else {
            deRegister(UDPsocket, argv[1], (struct sockaddr*) &servAddr, sendBuf);
            perror("Incomming connection failed");
            return EXIT_FAILURE;
         }
      }
      
      //at this point we have a TCP connection with another client
      //we are now going to run two threads simultaneously
      //the child thread will be to send chat
      //the parent thread will be to listen and display
      
      //spawn a child process
      int pid = fork();
      if(pid < 0){ //if failed to fork
         deRegister(UDPsocket, argv[1], (struct sockaddr*) &servAddr, sendBuf);
         perror("Failed to fork!\n");
         return EXIT_FAILURE;
      } else if(pid != 0) { /*********************** PARENT *****************/      
         printf("You are now in a chat room.\n"
                "Type \"" END_CONN "\" to exit the chat room.\n\n");
         
         bool sending = true;   //client still wants to chat
         bool firstTime = true; //start off first time
         while(sending){ //we will be ready to send until END_CONN
            //printf("You [%s]: ", timeStr());
            printf("%s", KCYN); //make home client cyan
            memset(&sendBuf, 0, BUF_SIZE); //clear buffer
            
            if(firstTime){             //if just come from selection
               firstTime = false;      //next time will not be first time
               currChar = getc(stdin); //we do another getc() to clear ENTER_KEY
            }
            
            currChar = getc(stdin);        //get message to send
            
            int i = 0;
            //while we done hit enter and less than max buff (-1 for '\0')
            while(((int)currChar != ENTER_KEY) && (i < (BUF_SIZE - 2))){
               sendBuf[i] = currChar;
               i++;
               currChar = getc(stdin);
            }
            sendBuf[i] = '\0';
            
            //if typed in command to end client, end client
            if(strcmp(sendBuf, END_CONN) == 0){
               sending = false; //end the chat room

               //send message of leaving chatroom
               send(nowSocket, LEAVING_CHATROOM_MSG, sizeof(LEAVING_CHATROOM_MSG), 0); 
               kill(pid, SIGKILL); //kill child process
            } else { //send message to server
               send(nowSocket, sendBuf, BUF_SIZE, 0);
            }
         }
      
      } else { /*********************************** CHILD **************************/
         int nread;
         char bufbuf[120];
         while(true){ //always ready to receive chat --------> until shared memory
            memset(recvBuf, 0, BUF_SIZE); //clear buffer
            nread = read(nowSocket, recvBuf, BUF_SIZE); //receive a message
            recvBuf[nread] = '\0'; //manually terminate string
           
            if(strcmp(recvBuf, ACK_MSG) == 0){ //if we received an ACK, we print ACK
               printf("*** received ***\n\n"); //symbolizes reception of message
            } else if(strcmp(recvBuf, LEAVING_CHATROOM_MSG) == 0){ //if other client left
               printf("*** *** *** You're friend left the chatroom. *** *** ***\n");
               deRegister(UDPsocket, argv[1], (struct sockaddr*) &servAddr, sendBuf);
               printf("Press ENTER to exit...\n");
            } else if(strstr(recvBuf, CONN_NOTICE) != NULL){ //if it is a connection notice, don't ACK
               printf("%s%s%s\n\n", KMAG, recvBuf, KCYN);
            } else { //if normal message
               send(nowSocket, ACK_MSG, sizeof(ACK_MSG), 0); //send ack of recieved and displayed
               printf("%s%s%s\n\n", KMAG, recvBuf, KCYN);
               //weirdly, we needed to do '\n' at the end...
            }
         }
      }
      close(nowSocket); //kill TCP connection
      
      //disconnet before ending
      deRegister(UDPsocket, argv[1], (struct sockaddr*) &servAddr, sendBuf);
   }

   return EXIT_SUCCESS;
}

//function to get a port number based on name
//this is actually a terrible function...but for now its fine
int getPortFromName(char* name){
   printf("Fetching client's port ... ");
   int portNumBase = 1024; //start at 1024 and go up
   int portNumXOR = 0;
   
   //XORs tend to be a way to condense a lot of stuff
   int i = 0;
   while(name[i] != '\0'){
      if((i%2) == 0){ //to shake things up a bit more
         portNumXOR = portNumXOR ^ (int)name[i];
      } else {        //we do a shift sometimes
         portNumXOR = portNumXOR ^ ((int)name[i] << 1);
      }
      i++;
   }
   
   int finalPort = portNumBase + portNumXOR;
   printf("Client's Port = %d\n", finalPort);   
   return finalPort;
}

//function to get UDP socket
int getUDPSocket(void){
   /*
    * AF_INET refers to the usual socket families for internet.
    * SOCK_DGRAM refers to UDP.
    */
   
   printf("Fetching UDP socket ... ");
   int UDPsocket;             //the socket we are using
   UDPsocket = socket(AF_INET, SOCK_DGRAM, 0); //get socket
   printf("UDPsocket = %d\n", UDPsocket);
   if(UDPsocket < 0){         //if we get an invalid socket
      perror("We have an error");
      exit(0); //terminate program
   }
   
   return UDPsocket;
}

//set up client address
void setCliAddr(int cliPort, int UDPsocket, struct sockaddr_in* cliAddr){
   cliAddr->sin_family = AF_INET;
   (cliAddr->sin_addr).s_addr = htonl(INADDR_ANY);
   cliAddr->sin_port = htons(cliPort); //port based on name
   //to allow for multiple clients on the same machine
   
   //try to bind the specified address to our socket
   //if failure it returns -1
   printf("Bind socket to port ... ");
   if(bind(UDPsocket, (struct sockaddr*) cliAddr, sizeof(*cliAddr)) < 0){
      perror("We have an error");
      close(UDPsocket);
      exit(0);
   }
   
   printf("socket binding successful.\n");  
   return;
}

//setup server address
void setServAddr(struct sockaddr_in* servAddr){
   servAddr->sin_family = AF_INET; //set to UDP server

   //htonl() converts IP (a long) to big-endian number
   //INADDR_ANY (0) will allow packets arriving to any interface
   //to go to the server
   (servAddr->sin_addr).s_addr = htonl(INADDR_ANY);
   servAddr->sin_port = htons(SERVER_PORT); //set the port
   return;
}

//function to obtain and display list of UDP client server
void getClientList(CC reg, int UDPsocket, char* recvBuf){
   struct sockaddr_in recvAddr;
   int recvAddrLen = sizeof(recvAddr);
   
   char nameBuf[SMALL_BUF_SIZE];  //to save the name
   char IPbuf[SMALL_BUF_SIZE];
   int cliFromServerPort;
   struct sockaddr_in cliFromServerAddr; //to save the address
   cliFromServerAddr.sin_family = AF_INET; //all clients use this
   
   printf("\n\tList of clients online: \n");
   int i = 1; //to enumerate clients
   
   int nBytes = recvfrom(UDPsocket, recvBuf, BUF_SIZE, 0, (struct sockaddr*) &recvAddr, (socklen_t*) &recvAddrLen);
   recvBuf[nBytes] = '\0'; //terminate string
   while(strcmp(recvBuf, END_LIST) != 0){ //while not at end of list
      printf("\t\t%d. %s\n", i, recvBuf);
      
      memset(nameBuf, 0, SMALL_BUF_SIZE); //clear name buffer
      int spaceI = strcspn(recvBuf, " "); //get index of space
      strncpy(nameBuf, recvBuf, spaceI);  //extract name from string

      //get index of the space after the IP address
      memset(IPbuf, 0, SMALL_BUF_SIZE);
      int fourthspaceI = strcspn(&recvBuf[(spaceI+8)], " ");
      strncpy(IPbuf, &recvBuf[(spaceI+8)], fourthspaceI); //extract IP string
      
      //from here we know where the port number starts, assume 5 chars
      sscanf(&recvBuf[(spaceI+8+fourthspaceI+15)], "%d", &cliFromServerPort);
      
      //build the address of this particular client from the buffers
      cliFromServerAddr.sin_addr.s_addr = inet_addr(IPbuf);
      cliFromServerAddr.sin_port = htons(cliFromServerPort + 1); //+1 for TCP port
      //add this client to the list
      push(reg, nameBuf, cliFromServerAddr);
      i++;
      
      memset(recvBuf, 0, BUF_SIZE); //clear buffer and get next incoming packets
      nBytes = recvfrom(UDPsocket, recvBuf, BUF_SIZE, 0, (struct sockaddr*) &recvAddr, (socklen_t*) &recvAddrLen);
      recvBuf[nBytes] = '\0';
   }
   
   printf("\t\t" END_LIST "\n\n");
   return;
}

//fundtion to register us with the UDP server
void registerClient(int sock, char* cliName, struct sockaddr* servAddr, char* sendBuf){
   printf("Registering with UDP server and getting list of clients online ...\n");
   
   //construct a string for the request to join chats
   sprintf(sendBuf, "%s %s", HELLO_REQUEST, cliName);
   
   //now send the request
   printf("Sending \"%s\"\n", sendBuf);
   sendto(sock, sendBuf, strlen(sendBuf), 0, servAddr, sizeof(*servAddr));
   return;
}

void deRegister(int sock, char* cliName, struct sockaddr* servAddr, char* sendBuf){
   //construct dissconnection request
   memset(sendBuf, 0, BUF_SIZE);
   sprintf(sendBuf, "%s %s", BYE_REQUEST, cliName);
   //of the form "I AM LEAVING! <name>"
   
   sendto(sock, sendBuf, BUF_SIZE, 0, servAddr, sizeof(*servAddr));
   close(sock); //kill UDP connection (socket)
   printf(BAR " Ending client for %s. " BAR "\n\n", cliName);
   return;
}
