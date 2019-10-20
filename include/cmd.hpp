#include <string>
#include <iostream>
#include <vector>
#include <array>

class Cmd{
public:
    Cmd(){
        this->funcp = NULL;
    }
    Cmd(bool (*funcp)()){
        this->funcp = funcp;
    }
    ~Cmd(){ argv.clear();}
    bool execute(){
        if(this->funcp){
            return funcp();
        }
        return false;
    }
    std::vector<std::string> argv;
    std::string flow;
private:
    bool (*funcp)();
};

std::vector<std::string> CmdSplit(std::string);
std::vector<Cmd*> CmdParse(std::vector<std::string>);
bool setupBuildin();
bool executeCommand(std::vector<Cmd*>);
bool buildin_setenv();
bool buildin_printenv();
bool buildin_exit();
bool buildin_runfile(std::string);