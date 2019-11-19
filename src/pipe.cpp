#include "pipe.hpp"
#include "user.hpp"
#include <vector>
#include <array>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <string>

extern UserManager userManager;
std::array<std::array<std::array<int,2>,30>,30> userPipe = {-1};
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
    int id ;
    if(cmd->userp_in != ""){
        id = std::stoi(cmd->userp_in.substr(1));
        if(!userManager.getUser(id)){
            pair[0] = -1;
        }else{
            if(userPipe[id][userManager.currentUser->user_id][0] == -1){
                std::cout << "*** Error: the pipe #" << id << "->#" << userManager.currentUser->user_id << " does not exist yet. ***" << std::endl;
                pair[0] = -2;
                return;
            }else{
                 pair[0] = userPipe[id][userManager.currentUser->user_id][0];
                std::string message = "*** " + ( userManager.currentUser->username=="" ? "(no name)": userManager.currentUser->username)+ " (#" + std::to_string(userManager.currentUser->user_id) +
                                    ") just received from " + (userManager.getUser(id)->username=="" ? "(no name)":userManager.getUser(id)->username) + " (#" + std::to_string(id) + ") by '" + cmd->cmdStr + "' ***\n";
                userManager.broadcast(message);   
            }
        }
    }

    if(cmd->userp_out != ""){
        id = std::stoi(cmd->userp_out.substr(1));
        if(!userManager.getUser(id)){
            pair[1] = -1;
        }else{
            if(userPipe[userManager.currentUser->user_id][id][1] == -1){
                std::array<int,2> temp;
                pipe(temp.data());
                userPipe[userManager.currentUser->user_id][id][1] = temp[1];
                userPipe[userManager.currentUser->user_id][id][0] = temp[0];
                pair[1] = userPipe[userManager.currentUser->user_id][id][1];
                std::string message = "*** " + ( userManager.currentUser->username=="" ? "(no name)": userManager.currentUser->username) + " (#" + std::to_string(userManager.currentUser->user_id) +
                                    ") just piped '" + cmd->cmdStr + "' to " + (userManager.getUser(id)->username=="" ? "(no name)":userManager.getUser(id)->username) + " (#" + std::to_string(id) + ") ***\n";
                userManager.broadcast(message);
            }else{
                std::cout << "*** Error: the pipe #" << userManager.currentUser->user_id << "->#" << id << " already exists. ***" << std::endl;
                pair[1] = -2;
                return;
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
    if(cmd->userp_in != ""){
        id = std::stoi(cmd->userp_in.substr(1));
        if(userPipe[id][userManager.currentUser->user_id][0] != -1){
            close(userPipe[id][userManager.currentUser->user_id][0]);
            userPipe[id][userManager.currentUser->user_id][0] = -1;
        }
        if(userPipe[id][userManager.currentUser->user_id][1] != -1){
            close(userPipe[id][userManager.currentUser->user_id][1]);
            userPipe[id][userManager.currentUser->user_id][1] = -1;
        }
    }

    if(all && cmd->userp_out != ""){
        id = std::stoi(cmd->userp_out.substr(1));
        if(userPipe[userManager.currentUser->user_id][id][1] != -1){
            close(userPipe[userManager.currentUser->user_id][id][1]);
            userPipe[userManager.currentUser->user_id][id][1]  = -1;
        }
    }
}