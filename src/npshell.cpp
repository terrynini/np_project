#include <iostream>
#include <string>
#include <vector>
#include "cmd.hpp"

using namespace std;

vector<string> cmds;

int main(){
    string cmdline;
    ios_base::sync_with_stdio(false);
    cin.tie(0);
    while(true){
        getline(cin, cmdline);
        cmds = CmdSplit(cmdline);
        executeCommand(cmds);
    }
    return 0;
}
