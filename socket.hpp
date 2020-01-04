#include <iostream>
#include <sstream>

#include <string.h> /* for bzero() */
#include <errno.h>  /* for errno */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace std;

void error(string message) {
    cerr << "[Error] " << message << endl;
    exit(1);
}

/*
    port : service associated with the desired port,
           or 0 to assign port automatically
    qlen : maximum length of the server request queue
*/
int server_sock(int port) {
    struct sockaddr_in sin; // an Internet endpoint address
    int sock;               // socket descriptor and socket type
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    // sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    /* Map service name to port number */
    if (port == 0) {
        sin.sin_port = htons(INADDR_ANY);
    } else if ((sin.sin_port = htons((u_short)port)) == 0) {
        error("can't get \"" + to_string(port) + "\" service entry");
    }
    /* Allocate a socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("can't create socket: " );
    /* Set socket option SO_REUSEADDR = true */
    int opt_val = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)) < 0)
        error("can't socket option: " );
    /* Bind the socket */
    if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        error("can't bind to " + to_string(port) +" port: " );
    if (listen(sock, 30) < 0)
        error("can't listen on " +  to_string(port) +" port: " );
    return sock;
}

/*
    ip   : ip to which connection is desired
    port : service associated with the desired port
*/
int client_sock(char *ip, int port) {
    struct hostent *phe;    /* pointer to host information entry */
    struct sockaddr_in sin; /* an Internet endpoint address */
    int sock;               /* socket descriptor */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    /* Map service name to port number */
    if ((sin.sin_port = htons((u_short)port)) == 0)
        error("can't get \"" + to_string(port) + "\" service entry");
    /* Set IP address with dotted decimal */
    if ((sin.sin_addr.s_addr = inet_addr(ip)) == INADDR_NONE)
        error("can't get \"" + string(ip) +"\" host entry\n");
    /* Allocate a socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("can't create socket: " );
    /* Connect the socket */
    if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        error("can't connect to " + string(ip) + ":"+ to_string(port) + ": " );
    return sock;
}