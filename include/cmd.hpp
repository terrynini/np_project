#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <unistd.h>

extern char ** environ;

class Cmd{
public:
    std::vector<std::string> argv;
    std::string flow;
};

std::vector<std::string> CmdSplit(std::string);
std::vector<Cmd*> CmdParse(std::vector<std::string>);
bool executeCommand(std::vector<Cmd*>);
void buildin_setenv(std::string, std::string);
void  buildin_printenv(std::string);
void buildin_exit();