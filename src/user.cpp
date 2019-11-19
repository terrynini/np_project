#include "user.hpp"

extern char ** environ;
UserManager userManager;

user* UserManager::getUser(int user_id){
    for(auto &user : this->users){
        if(user.user_id == user_id){
            return &user;
        }
    }
    return 0;
}

void UserManager::switchUser(int sockfd){
    for(auto &user : this->users){
        if(user.sockfd == sockfd){
            this->currentUser = &user;
            break;
        }
    }
}
void UserManager::broadcast(std::string message){
    for(auto &user : this->users){
        dprintf(user.sockfd, "%s", message.c_str());
    }
}

void UserManager::deleteUser(int sockfd){
    for(auto iter=this->users.begin(); iter != this->users.end() ; iter++){
        if(iter->sockfd == sockfd){
            this->users.erase(iter);
            return;
        }
    }
}

int UserManager::addUser(int sockfd, sockaddr_in * client_info = nullptr ){
    user newUser;
    newUser.sockfd = sockfd;
    newUser.env.push_back("PATH=bin:.");
    newUser.IP = inet_ntoa(client_info->sin_addr);
    newUser.port = ntohs(client_info->sin_port);
    
    int count = 1;
    auto iter = this->users.begin();
    while(true){
        if(count > 30)
            return -1;
        if( iter == this->users.end() || (iter)->user_id != count ){
            newUser.user_id = count;
            break;
        }
        count ++;
        iter ++ ;
    }    
    this->users.push_back(newUser);
    this->broadcast("*** User '(no name)' entered from " + newUser.IP + ":" + std::to_string(newUser.port) + ". ***\n");
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

