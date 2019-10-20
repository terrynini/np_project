#include <iostream>
#include <string>
#include <vector>
#include "cmd.hpp"

using namespace std;

int main(){
    ios_base::sync_with_stdio(false);
    cin.tie(0);
    string cmdline;
    vector<string> tokens;
    vector<Cmd*> cmds;
    while(true){
        getline(cin, cmdline);
        cout << "in" << endl;
        tokens = CmdSplit(cmdline);
        cmds = CmdParse(tokens);
        executeCommand(cmds);
    }
    return 0;
}
