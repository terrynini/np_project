#include "cmd.hpp"
#include <sstream>
#include <map>

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

bool executeCommand(std::vector<Cmd*> cmds){
    return true;
}

bool setupBuildin(){
    return true;
}

bool buildin_setenv(){
    return printf("setenv\n");
}

bool buildin_printenv(){
    return printf("print env\n");
}

bool buildin_exit(){
    //do nothing about threads?
    //exit(0);
    return printf("exit\n");
}

bool buildin_runfile(std::string file){
    return printf("run file %s", file.c_str());
}