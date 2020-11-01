#include <iostream>
#include <string>
#include <vector>
#include "cmd.hpp"
#include <sys/wait.h>

using namespace std;
extern pid_t tailCommand;
extern bool tailPipe;

void childHandler(int signo){
  while (waitpid(-1, NULL, WNOHANG) > 0);
}
    
void waitTail(){
    if(tailCommand && !tailPipe)
        waitpid(tailCommand, NULL, 0);
}

void init(){
    ios_base::sync_with_stdio(false);
    cin.tie(0);
    signal(SIGCHLD, childHandler);
    initBuildin();
    setenv("PATH","bin:.",1);
}

int main(){
    init();
    string cmdline;
    vector<string> tokens;
    vector<Cmd*> cmds;

    while(std::cout << "% " << flush && getline(cin, cmdline)){
        tokens = CmdSplit(cmdline);
        cmds = CmdParse(tokens);
        evalCommand(cmds);
        waitTail();
    }
    return 0;
}
