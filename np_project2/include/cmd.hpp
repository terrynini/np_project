#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <unistd.h>
#include <functional>
#include <map>

#ifndef CMD_H
#define CMD_H

class Cmd{
public:
    Cmd(){
        argv.clear();
        flow = "";
        userp_in = "";
        userp_out = "";
        cmdStr = "";
    }
    std::vector<std::string> argv;
    std::string flow;
    std::string userp_in;
    std::string userp_out;
    std::string cmdStr;
    int Exec();
};

void initBuildin();
int runBuildin(Cmd*);
void execute(Cmd*);
int evalCommand(std::vector<Cmd*>);
std::vector<std::string> CmdSplit(std::string);
std::vector<Cmd*> CmdParse(std::vector<std::string>,std::string);

#endif