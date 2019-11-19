#include "sock.hpp"
#include <cstring>
#include <iostream>

using std::cerr;
using std::endl;

int tcpBind(int port, int len){
    int sockfd;
    sockaddr_in serverInfo;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serverInfo, 0, sizeof(sockaddr_in));
    serverInfo.sin_family = AF_INET; 
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");//htonl(INADDR_ANY); 
    serverInfo.sin_port = htons(port);
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    if(bind(sockfd, (struct sockaddr *)&serverInfo, sizeof(serverInfo))< 0){
        cerr << "bind error" << endl;
        exit(0);
    }
    if(listen(sockfd, len)){
        cerr << "listen error" << endl;
        exit(0);
    }
    return sockfd;
}