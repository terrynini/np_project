#include "cmd.hpp"
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
            cmds.push_back(new Cmd());
            work = cmds.back();
        }
        else if( token[0] == '>'){
            redirect = true;
            work->flow = "> ";
        }else{
            work->argv.push_back(token);
        }
    }
    if(work->argv.size() == 0)
        cmds.pop_back();
    return cmds;
}

//static std::vector<std::string> buildin = {std::string("setenv"), std::string("printenv"), std::string("exit")};
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

bool executeCommand(std::vector<Cmd*> cmds){
    pid_t pid;
    std::vector<std::string>::iterator it;
    for(int i = 0 ; i < cmds.size() ; i++){
        if( !isBuildin(cmds[i])){
            pid =fork();
            if(pid == -1)
            {
                std::cerr << "Fail to fork" << std::endl;
                break;
            }
            if(pid != 0){

            }else{
                char* args[] = {"ls", NULL};
                execv("/bin/ls", args);
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