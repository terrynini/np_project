#include <iostream>
#include <string>
#include <vector>
#include "cmd.hpp"
#include "pipe.hpp"
#include "sock.hpp"
#include "user.hpp"
#include <sys/wait.h>
#include <cstdlib>

using namespace std;
extern pid_t tailCommand;
extern bool tailPipe;
extern PipeManager* pipeManager;
extern UserManager* userManager;
struct shared_st *shared;
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
    userManager = 0;
    shared = 0;
}

void spawnShell(){
    clearenv();
    setenv("PATH","bin:.",1);
    pipeManager = new PipeManager(); 
    string cmdline;
    vector<string> tokens;
    vector<Cmd*> cmds;
    cin.clear();
    while(std::cout << "% " << flush && getline(cin, cmdline)){
        tokens = CmdSplit(cmdline);
        cmds = CmdParse(tokens, cmdline);
        if( evalCommand(cmds) == -1){
            delete pipeManager;
            waitTail();
            return ;
        }
        waitTail();
    }
    delete pipeManager;
}

int main(int argc, char** argv){
    init();
    int input = dup(0);
    int output = dup(1);
    int error = dup(2);
    int port, len = 1;
    if (argc > 1)
        port = atoi(argv[1]);
    else
        port = 5566;
    int sockfd = tcpBind(port, len);
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
        dup2(input, 0);
        dup2(output, 1);
        dup2(error, 2);
    }
    return 0;
}
