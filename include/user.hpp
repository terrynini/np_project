#include <vector>
#include "pipe.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef USER_H
#define USER_H

struct user{
    user(){
        pipeManager = new PipeManager();
        env.clear();
        username = "";
    }
    int user_id;
    int sockfd;
    int port;
    std::string username;
    std::string IP;
    std::vector<std::string> env;
    PipeManager* pipeManager;
    void applyEnv();
    void saveEnv();
};

class UserManager{
public:
    UserManager(){
        users.clear();
        currentUser = nullptr;
    }
    std::vector<user> users;
    user* currentUser;
    user* getUser(int);
    void switchUser(int);
    int addUser(int, sockaddr_in*);
    void deleteUser(int);
    void broadcast(std::string);
};

#endif