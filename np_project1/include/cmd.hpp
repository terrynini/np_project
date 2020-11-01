#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <unistd.h>
#include <functional>
#include <map>

#ifndef CMD_H
#define CMD_H
extern char ** environ;

class Cmd{
public:
    std::vector<std::string> argv;
    std::string flow;
    std::string inflow;
    int Exec();
};

void initBuildin();
bool runBuildin(Cmd*);
void execute(Cmd*);
void evalCommand(std::vector<Cmd*>);
std::vector<std::string> CmdSplit(std::string);
std::vector<Cmd*> CmdParse(std::vector<std::string>);

#endif