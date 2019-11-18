#include <vector>
#include "pipe.hpp"
#include <string>

#ifndef USER_H
#define USER_H

struct user{
    user(){
        pipeManager = new PipeManager();
        env.clear();
    }
    int user_id;
    int sockfd;
    PipeManager* pipeManager;
    std::vector<std::string> env;
    void applyEnv();
    void saveEnv();
};

class UserManager{
public:
    UserManager(){
        users.clear();
        count = 0;
    }
    std::vector<user> users;
    user* getUser(int);
    int addUser(int);
    //should be remove
    int count;
};

#endif