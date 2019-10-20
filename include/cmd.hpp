#include <string>
#include <iostream>
#include <vector>

class Cmd{
    bool (*funcp)();
public:
    Cmd(){
        this->funcp = NULL;
    }
    Cmd(bool (*funcp)()){
        this->funcp = funcp;
    }
    ~Cmd(){}
    bool execute(){
        if(this->funcp){
            return funcp();
        }
        return false;
    }
};

std::vector<std::string> CmdSplit(std::string);
bool setupBuildin();
bool executeCommand(std::vector<std::string>);
bool buildin_setenv();
bool buildin_printenv();
bool buildin_exit();