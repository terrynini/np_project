#include "pipe.hpp"
#include <vector>
#include <array>

std::array<int,2> PipeManager::getPipe(int offset){
    std::array<int,2> result = {0,1};

    //std::cout << "counter "  << this->counter+offset << std::endl;
    for(auto& pipe: this->pipes){
        //std::cout << "\tpipe " << pipe->count << " " << pipe->pfd[0] << ' ' << pipe->pfd[1] << std::endl;
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
    // for (auto pipe = this->pipes.begin(); pipe!= this->pipes.end(); ){
    //     //std::cout << "\tpipe " << (*pipe)->count << " " << (*pipe)->pfd[0] << ' ' << (*pipe)->pfd[1] << std::endl;
    //     if( (*pipe)->count == this->counter){
    //         result[0] = (*pipe)->pfd[0];
    //         //std::cout << "\terase " << (*pipe)->pfd[0] <<  " " << (*pipe)->pfd[1] << std::endl;
    //         //pipe = this->pipes.erase( pipe );
    //         pipe++;
    //     }else{
    //         pipe++;
    //     }
    // }
    return result;
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