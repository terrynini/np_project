#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <unistd.h>


struct Pipe{
    Pipe(){
        this->count = 0;
        pipe(pfd);
    }
    Pipe(int count){
        this->count = count;
        pipe(pfd);
    }
    int count;
    int pfd[2];  
};

struct p_pair{
    int pfd[2];
};

class PipeManager{   
public:
    PipeManager(){ counter = 0;}
    std::array<int,2> getPipe(int);
    void insertPipe(Pipe*);
    void prune();
    int counter;
    std::vector<Pipe*> pipes;
};
