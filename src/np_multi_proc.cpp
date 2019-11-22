#include <iostream>
#include <string>
#include <vector>
#include "cmd.hpp"
#include "pipe.hpp"
#include "sock.hpp"
#include "shmdata.hpp"
#include <sys/wait.h>
#include <cstdlib>
#include <sys/shm.h>
#include <cstring>
#include <signal.h>
#define USERMAX 30

using namespace std;
extern pid_t tailCommand;
extern bool tailPipe;
extern PipeManager* pipeManager;
extern void broadcast(std::string);
pid_t c_pid;
int shmid;
struct shared_st *shared;
extern userInfo* currentUser;

void childHandler(int signo){
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void intHandler(int signo){
    shmdt(shared);
    shmctl(shmid, IPC_RMID, 0);
    exit(0);
}

void clientHandler(int signo){
    cout << shared->broadcastMessage << flush;
}

void registerUser(int pid, sockaddr_in* client){
    for(int i = 0 ; i < USERMAX ; i++){
        if(shared->userTable[i].pid == 0){
            shared->userTable[i].sin_port = client->sin_port;
            strcpy(shared->userTable[i].sin_addr, inet_ntoa(client->sin_addr));
            shared->userTable[i].uid = i+1;
            shared->userTable[i].pid = pid;
            break;
        }
    }   
}

void logout(){
    std::string name = ( currentUser->user_name[0] == 0 ? "(no name)": currentUser->user_name);
    broadcast("*** User '"+ name +"' left. ***\n");
    bzero(currentUser, sizeof(struct userInfo));
}

void waitTail(){
    if(tailCommand && !tailPipe)
        waitpid(tailCommand, NULL, 0);
}

void init(){
    ios_base::sync_with_stdio(false);
    cin.tie(0);
    signal(SIGCHLD, childHandler);
    signal(SIGINT, intHandler);
    signal(SIGUSR1, clientHandler);
    shmid = shmget((key_t)5566, sizeof(struct shared_st), 0666|IPC_CREAT);
    if(shmid == -1){
        cerr << "shmid failed" << endl;
        exit(0);
    }
    shared = (struct shared_st*)shmat(shmid,0 ,0);
    if( shared == (void *)-1){
        shmctl(shmid, IPC_RMID, 0);
        cerr << "shmat failed" << endl;
        exit(0);
    }
    bzero(shared,sizeof(struct shared_st));
    initBuildin();
}

void fini(){
    shmdt(shared);
    shmctl(shmid, IPC_RMID, 0);
}

void serverBanner(){
    cout << "****************************************\n** Welcome to the information server. **\n****************************************" << endl;
    for(int i = 0 ; ; i = (i+1)%USERMAX ){
        if( shared->userTable[i].pid == c_pid ){
            currentUser = &shared->userTable[i];
            break;
        } 
    }
    string name =  currentUser->user_name[0] == '\00' ? "(no name)" : currentUser->user_name;
    string message = "*** User '" + name + "' entered from " + currentUser->sin_addr + ":" + std::to_string(ntohs(currentUser->sin_port)) + ". ***\n";
    broadcast(message);
}

void spawnShell(){
    clearenv();
    ios_base::sync_with_stdio(false);
    cin.tie(0);
    setenv("PATH","bin:.",1);
    pipeManager = new PipeManager(); 
    string cmdline;
    vector<string> tokens;
    vector<Cmd*> cmds;
    //cin.clear();
    c_pid = getpid();
    serverBanner();
    while(std::cout << "% " << flush && getline(cin, cmdline)){
        tokens = CmdSplit(cmdline);
        cmds = CmdParse(tokens, cmdline);
        if( evalCommand(cmds) == -1){
            delete pipeManager;
            return ;
        }
        waitTail();
    }
    delete pipeManager;
}

int main(int argc, char** argv){
    init();
    int a = dup(1);
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
        pid_t pid;
        while((pid = fork()) == -1)
        {   
            usleep(5000);
        }
        if(pid != 0){
            registerUser(pid, &clientInfo);
            close(infd);
        }else{
            dup2(infd, 0);
            dup2(infd, 1);
            dup2(infd, 2);
            close(infd);
            close(sockfd);
            spawnShell();
            logout();
            shmdt(shared);
            exit(0);
        }
    }
    //wait all user to exit
    fini();
    return 0;
}