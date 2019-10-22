#include "cmd.hpp"
#include "pipe.hpp"
#include <sstream>
#include <map>
#include <unistd.h>
#include <algorithm>
#include <iterator>
#include <functional>

extern PipeManager pipeManager;
extern pid_t tailCommand;
std::map<std::string,std::function<void (std::vector<std::string>)>> Buildin;

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
    pipeManager.getIO(cmd, pair);

    pid_t pid;
    while((pid = fork()) == -1)
    {
        usleep(5000);
    }
    
    if(pid != 0){
        pipeManager.prune();   
        tailCommand = pid;
    }else{      
        dup2(pair[0],0);
        dup2(pair[1],1);
        if(cmd->flow[0] == '!'){
            dup2(pair[1],2);  
        }
        pipeManager.prune();
        if(cmd->Exec() == -1){
            std::cerr << "Unknown command: [" << cmd->argv[0] << "]." << std::endl;
            exit(0);
        }
    }
}

void evalCommand(std::vector<Cmd*> cmds){
    tailCommand = 0;
    std::vector<std::string>::iterator it;

    for(auto &cmd : cmds){
        if( !runBuildin(cmd)){
            execute(cmd);
        }
        pipeManager.counter += 1;
    }
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
    Buildin["setenv"] = [](std::vector<std::string> argv){
        if(argv.size() < 3)
            std::cerr << "Need more arguments" << std::endl;
        else
            setenv(argv[1].c_str(),argv[2].c_str(),1);
    };
    Buildin["printenv"] = [](std::vector<std::string> argv){
        if(argv.size() < 2)
            std::cerr << "Need more argument" << std::endl;
        else
            std::cout << getenv(argv[1].c_str()) << std::endl;
    };
    Buildin["exit"] = [](std::vector<std::string> argv){
        exit(0);
    };   
}

bool runBuildin(Cmd* cmd){
    auto func = Buildin.find(cmd->argv[0]);
    if( func != Buildin.end()){
        (func->second)(cmd->argv);
        return true;
    }
    return false;
}