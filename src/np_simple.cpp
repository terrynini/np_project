#include <iostream>
#include <string>
#include <vector>
#include "cmd.hpp"
#include "pipe.hpp"
#include "sock.hpp"
#include <sys/wait.h>
#include <cstdlib>

using namespace std;
extern pid_t tailCommand;
extern bool tailPipe;
extern PipeManager* pipeManager;

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
    clearenv();
    setenv("PATH","bin:.",1);
}
int input, output ,error;
void spawnShell(){
    pipeManager = new PipeManager(); 
    string cmdline;
    vector<string> tokens;
    vector<Cmd*> cmds;
    cin.clear();
    while(std::cout << "% " << flush && getline(cin, cmdline)){
        dprintf(output,"-- %s --\n", cmdline.c_str());
        tokens = CmdSplit(cmdline);
        cmds = CmdParse(tokens);
        dprintf(output,"eval\n");
        evalCommand(cmds);
        dprintf(output,"wailt\n");
        waitTail();
        dprintf(output,"leave\n");
    }
    delete pipeManager;
}

int main(int argc, char** argv){
    init();
            input = dup(0);
        output = dup(1);
        error = dup(2);
    int sockfd = tcpBind();
    sockaddr_in clientInfo;
    socklen_t addrlen = sizeof(clientInfo);
    int infd = 0;
    while(1){
        infd = accept(sockfd, (struct sockaddr *)&clientInfo, &addrlen);
        dup2(infd, 0);
        dup2(infd, 1);
        dup2(infd, 2);
        close(infd);
        spawnShell();
    }

    return 0;
}
