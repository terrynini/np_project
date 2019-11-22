#include "cmd.hpp"
#include "pipe.hpp"
#include "user.hpp"
#include "shmdata.hpp"
#include <sstream>
#include <map>
#include <unistd.h>
#include <algorithm>
#include <iterator>
#include <functional>
#include <cstring>
#include <signal.h>
#include <string>
#define USERMAX 30

extern PipeManager* pipeManager;
extern UserManager* userManager;
extern struct shared_st *shared;
extern pid_t tailCommand;
extern std::array<std::array<std::array<int,2>,USERMAX+1>,USERMAX+1> userPipe;
extern bool tailPipe;
userInfo* currentUser;

std::map<std::string,std::function<bool (std::vector<std::string>,std::string)>> Buildin;

std::vector<std::string> CmdSplit(std::string cmdline){ 
    std::istringstream css(cmdline);
    std::vector<std::string> tokens((std::istream_iterator<std::string>(css)),
                                    std::istream_iterator<std::string>());
    return tokens;
}

void broadcast(std::string message){
    strcpy(shared->broadcastMessage, message.c_str());
    for(int i = 0; i < USERMAX+1 ; i++){
        if(shared->userTable[i].pid > 0){
            kill(shared->userTable[i].pid, SIGUSR1);
        }
    }
}

std::vector<Cmd*> CmdParse(std::vector<std::string> tokens, std::string cmdline){
    std::vector<Cmd*> cmds;
    cmds.push_back(new Cmd());
    Cmd* work = cmds.back();
    bool redirect = false;
    
    for (auto &token : tokens){
        if (redirect){
            work->flow += token;
            work->cmdStr += " " + token;
            redirect = false;
        }
        else if(token[0] == '|' || token[0] == '!'){
            work->flow = token;
            work->cmdStr += " " + token;
            if(token.size() == 1)
                work->flow += "1";
            cmds.push_back(new Cmd());
            work = cmds.back();
        }
        else if( token[0] == '>' || token[0] == '<'){
            if( token.size() == 1){
                redirect = true;
                work->flow = token;//">";
                work->cmdStr += " " + token;
            }else if(userManager){
                if( token[0] == '>'){
                    work->userp_out = token;        
                    work->cmdStr  += " " + token;
                }else{
                    work->userp_in = token;
                    work->cmdStr += " " + token;
                }
            }
        }else{
            work->cmdStr += " " + token;
            work->argv.push_back(token);
        }
    }
    if(work->argv.size() == 0)
        cmds.pop_back();
    //buildin exception
    auto func = Buildin.find(tokens[0]);
    if( func != Buildin.end()){
        for( int i = 1 ; i < cmds.size() ; i++)
            cmds[0]->cmdStr += cmds[i]->cmdStr;
        for(int i = 1 ; i < cmds.size() ; i++)
            cmds.pop_back();
    }
    for(int i = 0 ; i < cmds.size() ; i++){
        if(cmds[i]->userp_in != "" || cmds[i]->userp_out != "")
            cmds[i]->cmdStr = cmdline;
        else
            cmds[i]->cmdStr = cmds[i]->cmdStr.substr(1);
        
    }
    return cmds;
}

void execute(Cmd* cmd){
    std::array<int,2> pair;
    pipeManager->getIO(cmd, pair);
    if(pair[0] < 0){
        if(pair[0] == -1)
            std::cout << "*** Error: user #" + cmd->userp_in.substr(1) + " does not exist yet. ***" << std::endl;
        return;
    }else if(pair[1] < 0){
        if(pair[1] == -1)
            std::cout << "*** Error: user #" + cmd->userp_out.substr(1) + " does not exist yet. ***" << std::endl;
        return; 
    }

    pid_t pid;
    while((pid = fork()) == -1)
    {
        usleep(5000);
    }
    
    if(pid != 0){
        pipeManager->prune();
        clearUserpipe(cmd, false);
        if( cmd->flow.size() && (cmd->flow[0] == '|' || cmd->flow[0] == '!'))
            tailPipe = true;
        else
        {
            tailPipe = false;
        }
        tailCommand = pid;
    }else{      
        dup2(pair[0],0);
        dup2(pair[1],1);
        if(cmd->flow[0] == '!'){
            dup2(pair[1],2);  
        }
        clearUserpipe(cmd, true);
        pipeManager->prune();
        if(cmd->Exec() == -1){
            std::cerr << "Unknown command: [" << cmd->argv[0] << "]." << std::endl;
            exit(0);
        }
    }
}

int evalCommand(std::vector<Cmd*> cmds){
    tailCommand = 0;
    tailPipe = false;
    int status;
    for(auto &cmd : cmds){
        status = runBuildin(cmd) ;
        if( status == 0){
            execute(cmd);
        }else if (status != 1){
            return status;
        }
        pipeManager->counter += 1;
    }
    return 1;
}

int Cmd::Exec(){
    std::vector<char*> args;
    for(auto &arg: this->argv){
        args.push_back(const_cast<char*>(arg.c_str()));
    }
    args.push_back(nullptr);
    return execvp(args[0], args.data());
}

void server2Buildin(){
    Buildin["exit"] = [&](std::vector<std::string> argv, std::string cmdstr) -> bool{
        userManager->broadcast("*** User '"+ ( userManager->currentUser->username=="" ? "(no name)": userManager->currentUser->username) +"' left. ***\n"); 
        return false;
    };   
    Buildin["who"] = [&](std::vector<std::string> argv, std::string cmdstr) -> bool{
        std::cout << "<ID>\t<nickname>\t<IP:port>\t<indicate me>" << std::endl;
        for(auto &user : userManager->users){
            std::cout << user.user_id << "\t";
            if( user.username == "")
                std::cout << "(no name)\t";
            else
                std::cout << user.username << "\t";
            std::cout << user.IP << ":" << user.port << "\t";
            if( user.user_id == userManager->currentUser->user_id )
                std::cout << "<-me";
            std::cout << std::endl;
        }
        return true;
    };   
    Buildin["yell"] = [&](std::vector<std::string> argv, std::string cmdstr) -> bool{
        if(argv.size() < 2)
            std::cerr << "Need more argument" << std::endl;
        else{
            std::string message = "";
            message += "*** " + ( userManager->currentUser->username=="" ? "(no name)": userManager->currentUser->username) + " yelled ***: ";
            message += cmdstr.substr(argv[0].size());
            userManager->broadcast( message + "\n");
        }
        return true;
    };   
    Buildin["tell"] = [&](std::vector<std::string> argv, std::string cmdstr) -> bool{
        if(argv.size() < 3)
            std::cerr << "Need more argument" << std::endl;
        else{
            int uid = stoi(argv[1]);
            user* dest = userManager->getUser(uid);
            if(dest == 0 ){
                std::cout << "*** Error: user #" + argv[1] +" does not exist yet. ***" << std::endl;
            }else{
                std::string message = "";
                message += "*** " + ( userManager->currentUser->username=="" ? "(no name)": userManager->currentUser->username) + " told you ***: ";
                message += cmdstr.substr((argv[0]+" "+argv[1]).size()+1);
                /* message += argv[2];
                for(int i = 3 ; i < argv.size() ; i++ )
                    message += " " + argv[i]; */
                dprintf(dest->sockfd, "%s" , (message+"\n").c_str());
            }
        }
        return true;
    };   
    Buildin["name"] = [&](std::vector<std::string> argv, std::string cmdstr) -> bool{
        if(argv.size() < 2)
            std::cerr << "Need more argument" << std::endl;
        else{
            for(auto &user : userManager->users){
                if(user.username == cmdstr.substr(argv[0].size()+1)){
                    std::cout << "*** User '" + user.username +"' already exists. ***" << std::endl;
                    return true;
                }
            }
            userManager->currentUser->username = cmdstr.substr(argv[0].size()+1);
            userManager->broadcast("*** User from " + userManager->currentUser->IP + ":" + std::to_string(userManager->currentUser->port) + " is named '"+ cmdstr.substr(argv[0].size()+1) +"'. ***\n");
        }
        return true;
    };
}

void server3Buildin(){
    Buildin["exit"] = [&](std::vector<std::string> argv, std::string cmdstr) -> bool{
        return false;
    };   
    Buildin["who"] = [&](std::vector<std::string> argv, std::string cmdstr) -> bool{
        std::cout << "<ID>\t<nickname>\t<IP:port>\t<indicate me>" << std::endl;
        for(int i  = 0 ; i < USERMAX+1 ; i++){
            if( shared->userTable[i].uid == 0)
                continue;
            std::cout << shared->userTable[i].uid << "\t";
            if( !strcmp(shared->userTable[i].user_name, "") )
                std::cout << "(no name)\t";
            else
                std::cout << shared->userTable[i].user_name << "\t";
            std::cout << shared->userTable[i].sin_addr << ":" << ntohs(shared->userTable[i].sin_port) << "\t";
            if( shared->userTable[i].uid == currentUser->uid )
                std::cout << "<-me";
            std::cout << std::endl;
        }
        return true;
    };   

    Buildin["yell"] = [&](std::vector<std::string> argv, std::string cmdstr) -> bool{
        if(argv.size() < 2)
            std::cerr << "Need more argument" << std::endl;
        else{
            std::string message = "";
            std::string name = (currentUser->user_name == "" ? "(no name)": currentUser->user_name) ;
            message += "*** " + name + " yelled ***: ";
            message += cmdstr.substr(argv[0].size());
            broadcast( message + "\n");
        }
        return true;
    };   
    Buildin["tell"] = [&](std::vector<std::string> argv, std::string cmdstr) -> bool{
        if(argv.size() < 3)
            std::cerr << "Need more argument" << std::endl;
        else{
            int uid = stoi(argv[1]);
            userInfo* dest = &shared->userTable[uid];
            if(dest->pid == 0 ){
                std::cout << "*** Error: user #" + argv[1] +" does not exist yet. ***" << std::endl;
            }else{
                std::string message = "";
                std::string name = currentUser->user_name == "" ? "(no name)" : currentUser->user_name;
                message += "*** " + name + " told you ***: ";
                message += cmdstr.substr((argv[0]+" "+argv[1]).size()+1) + "\n";
                strcpy(shared->broadcastMessage, message.c_str());
                kill(dest->pid, SIGUSR1);
            }
        }
        return true;
    };   
    Buildin["name"] = [&](std::vector<std::string> argv, std::string cmdstr) -> bool{
        if(argv.size() < 2)
            std::cerr << "Need more argument" << std::endl;
        else{
            for(int i = 0 ;i < USERMAX+1 ;i++){
                if( !strcmp(shared->userTable[i].user_name ,cmdstr.substr(argv[0].size()+1).c_str())){
                    std::cout << "*** User '" << shared->userTable[i].user_name << "' already exists. ***" << std::endl;
                    return true;
                }
            }
            strcpy(currentUser->user_name, cmdstr.substr(argv[0].size()+1).c_str());
            std::string message = "*** User from ";
            message += currentUser->sin_addr;
            message += ":" + std::to_string(ntohs(currentUser->sin_port)) + " is named '"+ cmdstr.substr(argv[0].size()+1) +"'. ***\n";
            broadcast(message);
        }
        return true;
    };
}

void initBuildin(){
    Buildin["setenv"] = [](std::vector<std::string> argv, std::string cmdstr) -> bool{
        if(argv.size() < 3)
            std::cerr << "Need more arguments" << std::endl;
        else
            setenv(argv[1].c_str(),argv[2].c_str(),1);
        return true;
    };
    Buildin["printenv"] = [](std::vector<std::string> argv, std::string cmdstr) -> bool{
        if(argv.size() < 2)
            std::cerr << "Need more argument" << std::endl;
        else{
            char* s = getenv(argv[1].c_str());
            if(s)
                std::cout << s << std::endl;
        }
        return true;
    };

    if(userManager){
        server2Buildin();
    }else if (shared){
        server3Buildin();
    }else{
        //server1
        Buildin["exit"] = [&](std::vector<std::string> argv, std::string cmdstr) -> bool{
        return false;
        };   
    }
}

int runBuildin(Cmd* cmd){
    auto func = Buildin.find(cmd->argv[0]);
    if( func != Buildin.end()){
        if((func->second)(cmd->argv, cmd->cmdStr))
            return 1;
        else
            return -1;
    }
    return 0;
}

