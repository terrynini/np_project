#include <iostream>
#include <string>
#include <vector>
#include "cmd.hpp"
#include "pipe.hpp"
#include "sock.hpp"
#include <sys/wait.h>
#include <sys/time.h>
#include <cstdlib>
#include "user.hpp"

using namespace std;

extern pid_t tailCommand;
extern bool tailPipe;
extern PipeManager* pipeManager;
extern UserManager userManager;

int input ;
int output;
int error ;

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
    input = dup(0);
    output = dup(1);
    error = dup(2);
    clearenv();
}

void talkToSocket(){}
void spawnShell(){
    string cmdline;
    vector<string> tokens;
    vector<Cmd*> cmds;
    cin.clear();
    if(getline(cin, cmdline)){
        tokens = CmdSplit(cmdline);
        cmds = CmdParse(tokens);
        if( evalCommand(cmds) == -1){
            //should delete current user
            //delete userManager;
            return ;
        }
        waitTail();
        std::cout << "% " << flush ;
    }
}

int main(int argc, char** argv){
    init();
    int port, len = 30;
    if (argc > 1)
        port = atoi(argv[1]);
    else
        port = 5566;
    int sockfd = tcpBind(port, len);
    sockaddr_in clientInfo;
    socklen_t addrlen = sizeof(clientInfo);
    int client_fd = 0;
    //
    fd_set activatefds, readfds;
    FD_ZERO(&activatefds);
    FD_SET(sockfd, &activatefds);
    int maxfd = sockfd + 1;
    //
    struct timeval tv;
    while(1){
        tv.tv_sec = 2;
        tv.tv_usec = 500000;
        readfds = activatefds;
        
        if( select(maxfd, &readfds, NULL, NULL, &tv) )
        {
            for(int trav_fd = 0 ; trav_fd < FD_SETSIZE; trav_fd++){
                if(FD_ISSET(trav_fd, &readfds)){
                    if(trav_fd == sockfd){
                        client_fd = accept(sockfd, (struct sockaddr *)&clientInfo, &addrlen);
                        if(client_fd > 0){
                            dup2(client_fd, 0);
                            dup2(client_fd, 1);
                            dup2(client_fd, 2);
                            //close(client_fd); should be done at log out
                            cout << "you are user " << userManager.addUser(client_fd) << endl;;
                            std::cout << "% " << flush;
                            //should not spawn shell at first
                            //spawnShell();
                            dup2(input, 0);
                            dup2(output, 1);
                            dup2(error, 2);
                            FD_SET(client_fd, &activatefds);
                            maxfd = (client_fd >= maxfd ) ? client_fd + 1 : maxfd;
                        }
                    }else{
                        dup2(trav_fd, 0);
                        dup2(trav_fd, 1);
                        dup2(trav_fd, 2);
                        user* nowUser = userManager.getUser(trav_fd);
                        //extract env
                        nowUser->applyEnv();
                        pipeManager = nowUser->pipeManager;
                        spawnShell();
                        //save env
                        nowUser->saveEnv();
                        pipeManager = nullptr;
                        dup2(input, 0);
                        dup2(output, 1);
                        dup2(error, 2);
                    }
                }
            }
            /* close(i);
            FD_CLR(i, &activatefds); */
        }
    }

    close(sockfd);
    return 0;
}
