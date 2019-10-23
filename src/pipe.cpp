#include "pipe.hpp"
#include <vector>
#include <array>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>

PipeManager pipeManager;
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