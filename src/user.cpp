#include "user.hpp"

extern char ** environ;
UserManager userManager;

user* UserManager::getUser(int sockfd){
    for(auto &user : this->users){
        if(user.sockfd == sockfd){
            return &user;
        }
    }
}

int UserManager::addUser(int sockfd){
    user newUser;
    newUser.sockfd = sockfd;
    newUser.env.push_back("PATH=bin:.");
    newUser.user_id = this->count++;
    this->users.push_back(newUser);
    return newUser.user_id;
}

void user::applyEnv(){
    clearenv();
    for(auto &env : this->env){
        putenv((char*)env.c_str());
    }
}


void user::saveEnv(){
    this->env.clear();
    if(environ){
        int i = 0;
        while(environ[i])
            this->env.push_back(environ[i++]);
    }
}