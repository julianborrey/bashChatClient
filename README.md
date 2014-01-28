# BashChat #

Currently a very buggy chat client that only works on your local host.
More of a proof of concept for now. One day I will hopefully get to 
improving it. As of now, it is entirely useless...

You could use it to understand TCP and UDP programming if you want.
(I comment a LOT.)

### Basic Architecture: ###
   * Client connects to server by UDP
   * Server gives information about clients online.
   * Client uses TCP to connect to another client (P2P).

See the README.txt file for more information.

### Goals: ###
   * Make sure it gets around firewalls.
   * Enable chat to any client
   * Remove bugs.
   * Enable multiple clients per chat room.
