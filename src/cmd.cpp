#include "cmd.hpp"
#include "pipe.hpp"
#include "user.hpp"
#include <sstream>
#include <map>
#include <unistd.h>
#include <algorithm>
#include <iterator>
#include <functional>

extern PipeManager* pipeManager;
extern UserManager userManager;
extern pid_t tailCommand;
extern bool tailPipe;

std::map<std::string,std::function<bool (std::vector<std::string>)>> Buildin;

std::vector<std::string> CmdSplit(std::string cmdline){ 
    std::istringstream css(cmdline);
    std::vector<std::string> tokens((std::istream_iterator<std::string>(css)),
                                    std::istream_iterator<std::string>());
    return tokens;
}

std::vector<Cmd*> CmdParse(std::vector<std::string> tokens){
    std::vector<Cmd*> cmds;
    cmds.push_back(new Cmd());
    Cmd* work = cmds.back();
    bool redirect = false;

    for (auto &token : tokens){
        if (redirect){
            work->flow += token;
            redirect = false;
        }
        else if(token[0] == '|' || token[0] == '!'){
            work->flow = token;
            if(token.size() == 1)
                work->flow += "1";
            cmds.push_back(new Cmd());
            work = cmds.back();
        }
        else if( token[0] == '>'){
            redirect = true;
            work->flow = ">";
        }else{
            work->argv.push_back(token);
        }
    }

    if(work->argv.size() == 0)
        cmds.pop_back();

    return cmds;
}

void execute(Cmd* cmd){
    std::array<int,2> pair;
    pipeManager->getIO(cmd, pair);

    pid_t pid;
    while((pid = fork()) == -1)
    {
        usleep(5000);
    }
    
    if(pid != 0){
        pipeManager->prune();
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

void initBuildin(){
    Buildin["setenv"] = [](std::vector<std::string> argv) -> bool{
        if(argv.size() < 3)
            std::cerr << "Need more arguments" << std::endl;
        else
            setenv(argv[1].c_str(),argv[2].c_str(),1);
        return true;
    };
    Buildin["printenv"] = [](std::vector<std::string> argv) -> bool{
        if(argv.size() < 2)
            std::cerr << "Need more argument" << std::endl;
        else{
            char* s = getenv(argv[1].c_str());
            if(s)
                std::cout << s << std::endl;
        }
        return true;
    };
    Buildin["exit"] = [](std::vector<std::string> argv) -> bool{
        return false;
    };   
    Buildin["who"] = [&](std::vector<std::string> argv) -> bool{
        std::cout << "<ID>\t<nickname>\t<IP:port>\t<indicate me>" << std::endl;
        for(auto &user : userManager.users){
            std::cout << user.user_id << "\t";
            if( user.username == "")
                std::cout << "(no name)\t";
            else
                std::cout << user.username << "\t";
            std::cout << user.IP << ":" << user.port << "\t";
            if( user.user_id == userManager.currentUser->user_id )
                std::cout << "<-me";
            std::cout << std::endl;
        }
        return true;
    };   
    Buildin["yell"] = [&](std::vector<std::string> argv) -> bool{
        if(argv.size() < 2)
            std::cerr << "Need more argument" << std::endl;
        else{
            if(userManager.currentUser->username == "" )
                userManager.broadcast("*** (no name) yelled ***: ");
            else
                userManager.broadcast("*** " + userManager.currentUser->username + " yelled ***: ");
            userManager.broadcast(argv[1]);
            for(int i = 2 ; i < argv.size() ; i++)
                userManager.broadcast( " " + argv[i]);
            userManager.broadcast("\n");
        }
        return true;
    };   
    Buildin["tell"] = [&](std::vector<std::string> argv) -> bool{
        if(argv.size() < 3)
            std::cerr << "Need more argument" << std::endl;
        else{
            int uid = stoi(argv[1]);
            for(auto &user : userManager.users){
                if(user.user_id == uid){
                    dprintf(user.sockfd, "*** %s told you ***: ", userManager.currentUser->username==""? "(no name)": userManager.currentUser->username.c_str());
                    dprintf(user.sockfd, "%s", argv[2].c_str());
                    for(int i = 3 ; i < argv.size() ; i++ )
                        dprintf(user.sockfd, " %s", argv[i].c_str());
                    dprintf(user.sockfd, "\n");
                    break;
                }
            }
        }
        return true;
    };   
    Buildin["name"] = [&](std::vector<std::string> argv) -> bool{
        if(argv.size() < 2)
            std::cerr << "Need more argument" << std::endl;
        else{
            userManager.currentUser->username = argv[1];
            userManager.broadcast("*** User from " + userManager.currentUser->IP + ":" + std::to_string(userManager.currentUser->port) + " is named '"+ argv[1] +"'. ***\n");
        }
        return true;
    };
}

int runBuildin(Cmd* cmd){
    auto func = Buildin.find(cmd->argv[0]);
    if( func != Buildin.end()){
        if((func->second)(cmd->argv))
            return 1;
        else
            return -1;
    }
    return 0;
}