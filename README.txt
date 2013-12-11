Chat Client - by Julian Borrey
==========================================================
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

To Build and Test:
==================
* Compile the server with the following bash line:
> gcc -o serv server.c CClib.c

* Compile the client with the following bash line:
> gcc -o cli client.c CClib.c

* Ensure that the following files are all in the same 
  directory:
      * server.c
      * client.c
      * chatProg.h
      * CClib.c

* To run the program first run the server in a shell.
  (user > ./serv)
  
* Then in another shell run a client. To do this us the line:
  > ./cli <name>
  You can put your name as the first argument. (NO SPACES!)

* Then run a second client, follow prompts and you will be 
  able to connect and chat.

Custom Client Spec:
===================
To build a custom client all you need to do is follow the 
protocol of the UDP server. To do this include the header
file "chatProg.h". This will allow you to access the 
commends you need to send to the server.

The server stores a list of all clients online and will 
update the list when a request for the list is made. This 
will indicate that a new client is online and their IP, 
port and name is recorded. When the client disconnects 
another message is sent to the UDP server and the server 
will remove the client from the list.

To register with the client and receive the list of 
online clients you need to send a string which reads
"<HELLO_REQUEST> <client's name>." (see the header file 
for the actual HELLO_REQUEST)

To deregister from the server the same string is made 
but the request used is BYE_REQUEST.

The list on the server is implemented as a linked list.
The strings returned by the server are then used to 
reconstruct another linked list on the client's side. 
This allows them to access any client online. Given the 
needs of your custom client you may or may not 
construct such a list.

One the list is obtained the client can connect to another 
client via the displayed information. NOTE: The port displayed
or given by the server is the UDP port of that client. To 
make a TCP use the port given by the computation of (UDPport+1).
This is the port that clients should listen on by convention 
outlined in this application.

Other Information:
==================
For full mechanics of the service, please see the diagram.

Basic Client Operation:
=======================
To operate the client you just need to follow the prompts
shown on the terminal. You are able to connect to a client 
shown in the list by selecting their number on the list.
You can end the chat session by typing "END CONNECTION" and 
that will remove you from the server's list of online clients.
If You end the session you will be shown a new list of online 
clients. Given that this is direclty on the bash, it will 
be very eash to just get back into the client if it the other 
user disconnects and casues the TCP connection fail.
(upKey + ENTER)

Bonus:
======
There are a few bonus points:
* The application is all written in C.
* It uses different colored text for the user and the person 
  they are talking to.
* The server's list is implemented as a linked list and so 
  the users are shown in "oldest first" order even when people 
  leave the list. If 2 position leaves, 3, 4, 5, ... move up 
  one position.
* If client is waiting for connection - they are notified of 
  who connected to them.
* Only 1 TCP connection made per chat so no time is lost 
  with remaking a connection. It is like a HTTP keepalive 
  for ever.
* It is bash based. (GUI's are annoying)
* Super commented code.
* Learnt multithreaded programming from scratch for this assingment :).
