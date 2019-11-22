#include "pipe.hpp"
#include "user.hpp"
#include <vector>
#include "shmdata.hpp"
#include <array>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <sys/stat.h> 
#include <signal.h>

#define USERMAX 30
extern UserManager* userManager;
extern shared_st* shared;
extern void broadcast(std::string);
extern userInfo* currentUser;
std::array<std::array<std::array<int,2>,31>,31> userPipe = {-1};
PipeManager* pipeManager;
pid_t tailCommand;
bool  tailPipe;
                   
std::array<int,2> PipeManager::getPipe(int offset){
    std::array<int,2> result = {0,1};
    for(auto& pipe: this->pipes){
        if(offset != 0 && pipe->count == this->counter+offset){
            result[1] = pipe->pfd[1];
            break;
        }
    }
    for(auto& pipe: this->pipes){
        if( pipe->count == this->counter){
            result[0] = pipe->pfd[0];
            break;
        }
    }
    return result;
}

void PipeManager::getIO(Cmd* cmd,std::array<int,2> &pair){
    int fileRedirect = 0;
    if(cmd->flow.size()){
        if(cmd->flow[0] == '|' || cmd->flow[0] == '!'){
            int offset = stoi(cmd->flow.substr(1));
            pair = this->getPipe(offset);
            if( pair == std::array<int,2>({0,1}) || pair[1] == 1){
                this->insertPipe(new Pipe(offset+this->counter));
                pair = this->getPipe(offset);
            }
        }else{
            fileRedirect = open(cmd->flow.substr(1).c_str(), O_RDWR|O_CREAT|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        }
    }else{
        pair = this->getPipe(0);
    }
    if(fileRedirect > 0){
        pair = this->getPipe(0);
        pair[1] = fileRedirect;
    }
    //server2
    if(userManager){
        int id ;
        if(cmd->userp_in != ""){
            id = std::stoi(cmd->userp_in.substr(1));
            if(!userManager->getUser(id)){
                pair[0] = -1;
            }else{
                if(userPipe[id][userManager->currentUser->user_id][0] == -1){
                    std::cout << "*** Error: the pipe #" << id << "->#" << userManager->currentUser->user_id << " does not exist yet. ***" << std::endl;
                    pair[0] = -2;
                    return;
                }else{
                    pair[0] = userPipe[id][userManager->currentUser->user_id][0];
                    std::string message = "*** " + ( userManager->currentUser->username=="" ? "(no name)": userManager->currentUser->username)+ " (#" + std::to_string(userManager->currentUser->user_id) +
                                        ") just received from " + (userManager->getUser(id)->username=="" ? "(no name)":userManager->getUser(id)->username) + " (#" + std::to_string(id) + ") by '" + cmd->cmdStr + "' ***\n";
                    userManager->broadcast(message);   
                }
            }
        }
        if(cmd->userp_out != ""){
            id = std::stoi(cmd->userp_out.substr(1));
            if(!userManager->getUser(id)){
                pair[1] = -1;
            }else{
                if(userPipe[userManager->currentUser->user_id][id][1] == -1){
                    std::array<int,2> temp;
                    pipe(temp.data());
                    userPipe[userManager->currentUser->user_id][id][1] = temp[1];
                    userPipe[userManager->currentUser->user_id][id][0] = temp[0];
                    pair[1] = userPipe[userManager->currentUser->user_id][id][1];
                    std::string message = "*** " + ( userManager->currentUser->username=="" ? "(no name)": userManager->currentUser->username) + " (#" + std::to_string(userManager->currentUser->user_id) +
                                        ") just piped '" + cmd->cmdStr + "' to " + (userManager->getUser(id)->username=="" ? "(no name)":userManager->getUser(id)->username) + " (#" + std::to_string(id) + ") ***\n";
                    userManager->broadcast(message);
                }else{
                    std::cout << "*** Error: the pipe #" << userManager->currentUser->user_id << "->#" << id << " already exists. ***" << std::endl;
                    pair[1] = -2;
                    return;
                }
            }
        }
    }
    //server3
    if(shared){
        int id ;
        if(cmd->userp_in != ""){
            id = std::stoi(cmd->userp_in.substr(1));
            if(!shared->userTable[id].pid){
                pair[0] = -1;
            }else{
                 if(shared->userPipe[id][currentUser->uid][0] == -1){
                    std::cout << "*** Error: the pipe #" << id << "->#" << currentUser->uid << " does not exist yet. ***" << std::endl;
                    pair[0] = -2;
                    return;
                }else{
                    pair[0] = shared->userPipe[id][currentUser->uid][0];
                    std::string myname = currentUser->user_name[0] == 0 ? "(no name)": currentUser->user_name;
                    std::string tarname = shared->userTable[id].user_name[0] == 0 ? "(no name)":shared->userTable[id].user_name; 
                    std::string message = "*** " + myname  + " (#" + std::to_string(currentUser->uid) +
                                        ") just received from " + tarname + " (#" + std::to_string(id) + ") by '" + cmd->cmdStr + "' ***\n";
                    broadcast(message);   
                }
            }
        }
        if(cmd->userp_out != ""){
            id = std::stoi(cmd->userp_out.substr(1));
            if(!shared->userTable[id].pid){
                pair[1] = -1;
            }else{
                if(shared->userPipe[currentUser->uid][id][1] == -1){
                    //openFIFO(temp);
                    std::string fifo_path = "./user_pipe/" + std::to_string(currentUser->uid) + "_" + std::to_string(id);
                    shared->userPipeInfo = currentUser->uid;
                    if((mkfifo(fifo_path.c_str(), 0666) <0) && (errno != EEXIST))
                        std::cout << "Create FIFO fail" << std::endl;
                    /*while( mkfifo(fifo_path.c_str(), 0666) < 0 ) {
                        kill(shared->userTable[id].pid, SIGUSR2);
                        usleep(10000);
                        std::cout << " try.." << std::endl;
                    } */
                    kill(shared->userTable[id].pid,SIGUSR2);
                    shared->userPipe[currentUser->uid][id][1] = open(fifo_path.c_str(), O_WRONLY);
                    //shared->userPipe[currentUser->uid][id][0] =  temp[0];
                    pair[1] = shared->userPipe[currentUser->uid][id][1];
                    std::string myname = currentUser->user_name[0] == 0 ? "(no name)": currentUser->user_name;
                    std::string tarname = shared->userTable[id].user_name[0] == 0 ? "(no name)":shared->userTable[id].user_name; 
                    std::string message = "*** " + myname + " (#" + std::to_string(currentUser->uid) +
                                        ") just piped '" + cmd->cmdStr + "' to " + tarname + " (#" + std::to_string(id) + ") ***\n";
                    broadcast(message);
                }else{
                    std::cout << "*** Error: the pipe #" << currentUser->uid << "->#" << id << " already exists. ***" << std::endl;
                    pair[1] = -2;
                    return;
                }
            }
        }
    }
}

void PipeManager::insertPipe(Pipe* in){
    pipes.push_back(in);
}

void PipeManager::prune(){
    for (auto pipe = this->pipes.begin(); pipe!= this->pipes.end(); ){
        if( (*pipe)->count == this->counter){  
            for(auto &ele : (*pipe)->pfd){
                if(ele > 2)
                    close(ele);
            }          
            pipe = this->pipes.erase( pipe );
        }else{
            pipe++;
        }
    }
}

void clearUserpipe(Cmd* cmd, bool all){
    int id;
    if(userManager){
        if(cmd->userp_in != ""){
            id = std::stoi(cmd->userp_in.substr(1));
            if(userPipe[id][userManager->currentUser->user_id][0] != -1){
                close(userPipe[id][userManager->currentUser->user_id][0]);
                userPipe[id][userManager->currentUser->user_id][0] = -1;
            }
            if(userPipe[id][userManager->currentUser->user_id][1] != -1){
                close(userPipe[id][userManager->currentUser->user_id][1]);
                userPipe[id][userManager->currentUser->user_id][1] = -1;
            }
        }

        if(all && cmd->userp_out != ""){
            id = std::stoi(cmd->userp_out.substr(1));
            if(userPipe[userManager->currentUser->user_id][id][1] != -1){
                close(userPipe[userManager->currentUser->user_id][id][1]);
                userPipe[userManager->currentUser->user_id][id][1]  = -1;
            }
        }
    }else if(shared){
        if(cmd->userp_in != ""){
            id = std::stoi(cmd->userp_in.substr(1));
            if(shared->userPipe[id][currentUser->uid][0] != -1){
                close(shared->userPipe[id][currentUser->uid][0]);
                shared->userPipe[id][currentUser->uid][0] = -1;
            }
        }

        if(cmd->userp_out != ""){
            id = std::stoi(cmd->userp_out.substr(1));
            if(shared->userPipe[currentUser->uid][id][1] != -1){
                close(shared->userPipe[currentUser->uid][id][1]);
                shared->userPipe[currentUser->uid][id][1]  = -1;
            }
        }
    }
}