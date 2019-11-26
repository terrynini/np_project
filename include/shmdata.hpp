#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef _SHMDATA_H
#define _SHMDATA_H

//*** User ’(no name)’ entered from 140.113.215.62:1201. *** 
struct userInfo{
    int pid;
    int uid;
    char user_name[21];
    char sin_addr[16];
    in_port_t sin_port;
};

 struct shared_st
{
    int usercount;
    struct userInfo userTable[50];
    char   broadcastMessage[400];
    int    userPipe[31][31][2];
    int    userPipeInfo;
    int    shutdown;
    int    server_pid;
};

#endif