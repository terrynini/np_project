#include "catch.hpp"
#define private public
#include "cmd.hpp"

TEST_CASE("Test the cmd split"){
    REQUIRE(CmdSplit("abcd efg") == std::vector<std::string>({"abcd","efg"}));
    REQUIRE(CmdSplit("printenv PATH") == std::vector<std::string>({"printenv", "PATH"}));
    REQUIRE(CmdSplit("setenv PATH bin") == std::vector<std::string>({"setenv", "PATH", "bin"}));
    REQUIRE(CmdSplit("printenv PATH") == std::vector<std::string>({"printenv", "PATH"}));
    REQUIRE(CmdSplit("ls") == std::vector<std::string>({"ls"}));
    REQUIRE(CmdSplit("ls bin") == std::vector<std::string>({"ls", "bin"}));
    REQUIRE(CmdSplit("cat test.html > test1.txt") == std::vector<std::string>({"cat", "test.html", ">", "test1.txt"}));
    REQUIRE(CmdSplit("cat test1.txt") == std::vector<std::string>({"cat", "test1.txt"}));
    REQUIRE(CmdSplit("removetag test.html") == std::vector<std::string>({"removetag", "test.html"}));
    REQUIRE(CmdSplit("removetag test.html > test2.txt") == std::vector<std::string>({"removetag", "test.html", ">", "test2.txt"}));
    REQUIRE(CmdSplit("cat test2.txt") == std::vector<std::string>({"cat", "test2.txt"}));
    REQUIRE(CmdSplit("removetag0 test.html") == std::vector<std::string>({"removetag0", "test.html"}));
    REQUIRE(CmdSplit("removetag0 test.html > test2.txt") == std::vector<std::string>({"removetag0", "test.html", ">", "test2.txt"}));
    REQUIRE(CmdSplit("cat test2.txt") == std::vector<std::string>({"cat", "test2.txt"}));
    REQUIRE(CmdSplit("removetag test.html | number") == std::vector<std::string>({"removetag", "test.html", "|", "number"}));
    REQUIRE(CmdSplit("removetag test.html |2 ls ") == std::vector<std::string>({"removetag", "test.html", "|2", "ls"}));
    REQUIRE(CmdSplit("number ") == std::vector<std::string>({"number"}));
    REQUIRE(CmdSplit("removetag test.html |2 removetag test.html |1") == std::vector<std::string>({"removetag", "test.html", "|2", "removetag", "test.html", "|1"}));
    REQUIRE(CmdSplit("number |1 number") == std::vector<std::string>({"number", "|1", "number"}));
    REQUIRE(CmdSplit("removetag test.html | number |1 number") == std::vector<std::string>({"removetag", "test.html", "|", "number", "|1", "number"}));
    REQUIRE(CmdSplit("ls |2 ls") == std::vector<std::string>({"ls", "|2", "ls"}));
    REQUIRE(CmdSplit("number > test3.txt") == std::vector<std::string>({"number", ">", "test3.txt"}));
    REQUIRE(CmdSplit("cat test3.txt") == std::vector<std::string>({"cat", "test3.txt"}));
    REQUIRE(CmdSplit("removetag test.html |1 ") == std::vector<std::string>({"removetag", "test.html", "|1"}));
    REQUIRE(CmdSplit(" number ") == std::vector<std::string>({"number"}));
    REQUIRE(CmdSplit("removetag test.html |2 ") == std::vector<std::string>({"removetag", "test.html", "|2"}));
    REQUIRE(CmdSplit("removetag test.html |1") == std::vector<std::string>({"removetag", "test.html", "|1"}));
}

TEST_CASE("Test cmd parse"){
    std::vector<Cmd*> cmds;
    cmds = CmdParse(std::vector<std::string>({"removetag", "test.html", "|1"}));
    REQUIRE(cmds[0]->argv == std::vector<std::string>({ "removetag","test.html"}));
    REQUIRE(cmds[0]->flow == "|1");
    cmds = CmdParse(std::vector<std::string>({"removetag", "test.html", "|2", "remove7ag", "test.html", "|1"}));
    REQUIRE(cmds[0]->argv == std::vector<std::string>({"removetag","test.html"}));
    REQUIRE(cmds[0]->flow == "|2");
    REQUIRE(cmds[1]->argv == std::vector<std::string>({"remove7ag","test.html"}));
    REQUIRE(cmds[1]->flow == "|1");
    cmds = CmdParse(std::vector<std::string>({"removetag", "test.html", "|", "number"}));
    REQUIRE(cmds[0]->argv == std::vector<std::string>({"removetag", "test.html"}));
    REQUIRE(cmds[0]->flow == "|1");
    REQUIRE(cmds[1]->argv == std::vector<std::string>({"number"}));
    cmds = CmdParse(std::vector<std::string>({"removetag0", "test.html", ">", "test2.txt"}));
    REQUIRE(cmds[0]->argv == std::vector<std::string>({"removetag0", "test.html"}));
    REQUIRE(cmds[0]->flow == ">test2.txt");
}