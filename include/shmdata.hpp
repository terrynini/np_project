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
    in_addr sin_addr;
    in_port_t sin_port;
};

 struct shared_st
{
    struct userInfo userTable[30];
    char   broadcastMessage[100];
};

#endif