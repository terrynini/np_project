#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <unistd.h>
#include "cmd.hpp"

#ifndef PIPE_H
#define PIPE_H

struct Pipe{
    Pipe(int count = 0){
        this->count = count;
        pipe(pfd);
    }
    int count;
    int pfd[2];  
};

class PipeManager{   
public:
    PipeManager(){ counter = 0;}
    std::array<int,2> getPipe(int);
    void getIO(Cmd*,std::array<int,2>&);
    void insertPipe(Pipe*);
    void prune();
    int counter;
    std::vector<Pipe*> pipes;
};
#endif