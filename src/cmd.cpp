#include "cmd.hpp"
#include "pipe.hpp"
#include <sstream>
#include <map>
#include <unistd.h>
#include <algorithm>

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

bool isBuildin(Cmd* cmd){
    if(cmd->argv[0] == "setenv"){
        if(cmd->argv.size() < 3)
            std::cerr << "Need more argument" << std::endl;
        else
            buildin_setenv(cmd->argv[1],cmd->argv[2]);
        return true;
    }else if (cmd->argv[0] == "printenv"){
        if(cmd->argv.size() < 2)
            std::cerr << "Need more argument" << std::endl;
        else
            buildin_printenv(cmd->argv[1]);
        return true;
    }else if (cmd->argv[0] == "exit"){
        buildin_exit();
        return true;
    }
    return false;
}

PipeManager pipeManager;

bool execute(Cmd* cmd){
    std::array<int,2> pair;
    //std::cout << "cmd " << cmd->argv[0] << std::endl;
    if(cmd->flow.size()){
        if(cmd->flow[0] == '|'){
            int offset = stoi(cmd->flow.substr(1));
            //std::cout <<"X";
            pair = pipeManager.getPipe(offset);
            if( pair == std::array<int,2>({0,1}) || pair[1] == 1){
                pipeManager.insertPipe(new Pipe(offset+pipeManager.counter));
                //std::cout <<"X2";
                pair = pipeManager.getPipe(offset);
            }
        }
    }else{
        pair = pipeManager.getPipe(0);
    }
    std::cout << pair[0] << " " << pair[1] << std::endl;
    pid_t pid = fork();
    if(pid == -1)
    {
        std::cerr << "Fail to fork" << std::endl;
        return false;   
    }
    if(pid != 0){
        pipeManager.prune();      
    }else{      
        dup2(pair[0],0);
        dup2(pair[1],1);
        for(auto &ele : pair){
            if(ele > 2)
                close(ele);
        }
        std::vector<char*> args;
        for(auto &arg: cmd->argv){
            args.push_back(const_cast<char*>(arg.c_str()));
        }
        args.push_back(nullptr);
        if(execvp(args[0], args.data()) == -1){
            std::cerr << "Command not found" << std::endl;
            exit(0);
        }
    }
    pipeManager.counter += 1;
    return true;
}

bool executeCommand(std::vector<Cmd*> cmds){
    std::vector<std::string>::iterator it;
    for(auto &cmd : cmds){
        if( !isBuildin(cmd)){
            if(!execute(cmd)){
                break;
            }
        }
    }
    return true;
}

void buildin_setenv(std::string name, std::string value){
    setenv(name.c_str(),value.c_str(),1);
}

void  buildin_printenv(std::string name){
    std::cout << getenv(name.c_str()) << std::endl;
}

void buildin_exit(){
    exit(0);
}