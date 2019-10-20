#include "cmd.hpp"
#include <sstream>
#include <map>

std::map<std::string,std::string> env();

std::vector<std::string> CmdSplit(std::string cmdline){ 
    std::istringstream css(cmdline);
    std::vector<std::string> splits((std::istream_iterator<std::string>(css)),
                                    std::istream_iterator<std::string>());
    return splits;
}

bool executeCommand(std::vector<std::string> cmds){
    std::map<std::string, bool(*)()>BuildIn;
    //init; here should be refactor later
    setupBuildin();
    BuildIn["setenv"] = buildin_setenv; 
    BuildIn["printenv"] = buildin_printenv;
    BuildIn["exit"] = buildin_exit;
    //end init
    for(auto &cmd : cmds){
        if(BuildIn.find(cmd) != BuildIn.end()){
            return BuildIn[cmd]();
        }else{
            //find bin 
        }
    }
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
