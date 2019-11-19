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
extern std::array<std::array<std::array<int,2>,30>,30> userPipe;

int input ;
int output;
int error ;

inline void talk2sock(int sockfd){
    dup2(sockfd, 0);
    dup2(sockfd, 1);
    dup2(sockfd, 2);
}

inline void talk2local(){
    dup2(input, 0);
    dup2(output, 1);
    dup2(error, 2);
}

void serverBanner(){
    cout << "****************************************\n** Welcome to the information server. **\n****************************************" << endl;
}

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
    for(int i = 0 ; i < 30 ; i++)
        for(int j = 0 ; j < 30 ; j++)
            userPipe[i][j].fill(-1);
    input = dup(0);
    output = dup(1);
    error = dup(2);
    clearenv();
}

bool spawnShell(){
    string cmdline;
    vector<string> tokens;
    vector<Cmd*> cmds;
    cin.clear();
    if(getline(cin, cmdline)){
        tokens = CmdSplit(cmdline);
        cmds = CmdParse(tokens);
        //
        /* for(auto &cmd : cmds){
            std::cout << "-- ";
            for(auto &arg : cmd->argv)
                cout << arg << " ";
            cout << "flow:" << cmd->flow << "userin:" << cmd->userp_in << "userout:" << cmd->userp_out << " --\n";
        } */
        //
        if( evalCommand(cmds) == -1){
            return false;
        }
        waitTail();
        std::cout << "% " << flush ;
    }else{
        return false;
    }
    return true;
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
        if( select(maxfd, &readfds, NULL, NULL, &tv) > 0)
        {
            for(int trav_fd = 0 ; trav_fd < FD_SETSIZE; trav_fd++){
                if(FD_ISSET(trav_fd, &readfds)){
                    if(trav_fd == sockfd){
                        client_fd = accept(sockfd, (struct sockaddr *)&clientInfo, &addrlen);
                        if(client_fd > 0){
                            talk2sock(client_fd);
                            serverBanner();
                            if(userManager.addUser(client_fd, &clientInfo) < 0){
                                talk2local();
                                close(client_fd);
                                continue;    
                            }
                            std::cout << "% " << flush;
                            talk2local();
                            FD_SET(client_fd, &activatefds);
                            maxfd = (client_fd >= maxfd ) ? client_fd + 1 : maxfd;
                        }
                    }else{
                        talk2sock(trav_fd);
                        userManager.switchUser(trav_fd);
                        userManager.currentUser->applyEnv();
                        pipeManager = userManager.currentUser->pipeManager;
                        if(!spawnShell()){
                            userManager.deleteUser(trav_fd);
                            close(trav_fd);
                            FD_CLR(trav_fd, &activatefds);
                        }else{
                            userManager.currentUser->saveEnv();
                        }
                        pipeManager = nullptr;
                        userManager.currentUser = nullptr;
                        talk2local();
                    }
                }
            }
        }
    }

    close(sockfd);
    return 0;
}
