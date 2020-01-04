#include <fstream>
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <string.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUF_SIZE 60000
#define IS_SOCKS4A(x) (!(x & 0x00ffffff)) && (x & 0xff000000)
using namespace std;

class Socks4 {
public:
    u_char sp_vn;
    u_char sp_cd;
    u_int sp_dst_port;
    u_int sp_dst_ip;
    char* sp_user_id;
    u_char* request;
    int client_fd;
    char sp_dst_ip_cstr[INET_ADDRSTRLEN];
    sockaddr_in client_addr;
    u_char reply[8];
    Socks4(int client_fd, sockaddr_in client_addr);
    ~Socks4();
    void transport(int from_fd, int to_fd);
    void Read();
    void Parse();
    void FireWall();
    void Info();
    void Reject();
    void ConnectMode();
    void BindMode();
    void Run();
};

class Server {
public:
    int sock;
    Server(int port);
    ~Server();
    static int genSock(int port);
    static int genClient(char *ip, int port );
    void Run();
};

int main(int argc, char const* argv[])
{
    if (argc < 2) {
        throw "Usage: ./socks_server [port]";
    }
    signal(SIGCLD, SIG_IGN);
    Server server(atoi(argv[1]));
    try  {
        server.Run();
    } catch (const char* msg) {
        cout << "[ ERROR ] " << msg << endl;
    }
    return 0;
}

void Socks4::Run()
{
    Read();
    Parse();
    FireWall();
    Info();
    Reject();
    if (sp_cd == 1) {
        ConnectMode();
    } else if (sp_cd == 2) {
        BindMode();
    } else {
        throw "Bad sock4 request, CD = " + std::to_string(sp_cd);
    }
}

Socks4::Socks4(int client_fd, sockaddr_in client_addr)
{
    this->client_fd = client_fd;
    this->client_addr = client_addr;
    memset(this->reply, 0, 8);
}

Socks4::~Socks4()
{
    free(this->request);
}

void Socks4::Read()
{
    u_char request[BUF_SIZE];
    int read_count = read(client_fd, request, BUF_SIZE);
    if (read_count == -1)
        throw "Client connection fail";
    if (read_count == 0) {
        cout << "Client disconnect" << endl;
        exit(0);
    }
    this->request = (u_char*)malloc(read_count);
    memcpy(this->request, request, read_count);
}

void Socks4::Parse()
{

    sp_vn = request[0];
    sp_cd = request[1];
    sp_dst_port = request[2] << 8 | request[3];
    sp_dst_ip = request[7] << 24 | request[6] << 16 | request[5] << 8 | request[4];
    sp_user_id = (char*)request + 8;
    if (sp_vn != 4) {
        throw "Bad sock4 request, VN = " + sp_vn;
    }

    if (IS_SOCKS4A(sp_dst_ip)) {
        size_t user_id_len = strlen(sp_user_id);
        char* sp_domain_name = sp_user_id + user_id_len + 1;
        /* resolve ip address by domain_name */
        struct hostent* phe;
        if (phe = gethostbyname(sp_domain_name)) {
            struct sockaddr_in sin;
            bcopy(phe->h_addr, (char*)&sin.sin_addr, phe->h_length);
            inet_ntop(AF_INET, &sin.sin_addr, sp_dst_ip_cstr, INET_ADDRSTRLEN);
        } else {
            throw "Get ip by domain_name failed, domain_name = " + string(sp_domain_name);
        }
    } else {
        inet_ntop(AF_INET, &sp_dst_ip, sp_dst_ip_cstr, INET_ADDRSTRLEN);
    }
}

void Socks4::FireWall()
{
    ifstream fin("socks.conf");
    if (fin) {
        string rule, mode, addr_str, addr_part[4];
        reply[1] = 91; /* default with rejected */
        while (fin >> rule >> mode >> addr_str) {
            for (int i = 0; i < 4; i++) {
                auto pos = addr_str.find_first_of('.');
                if (pos != string::npos) {
                    addr_part[i] = addr_str.substr(0, pos);
                    addr_str.erase(0, pos + 1);
                } else {
                    addr_part[i] = addr_str;
                }
            }
            if (((mode == "c" && sp_cd == 1) || (mode == "b" && sp_cd == 2)) && (addr_part[0] == "*" || ((u_char)stoul(addr_part[0]) == request[4])) && (addr_part[1] == "*" || ((u_char)stoul(addr_part[1]) == request[5])) && (addr_part[2] == "*" || ((u_char)stoul(addr_part[2]) == request[6])) && (addr_part[3] == "*" || ((u_char)stoul(addr_part[3]) == request[7]))) {
                reply[1] = 90;
                break;
            }
        }
    } else {
        cout << "[Warning] Can not open \"socks.conf\", all accept" << endl;
        reply[1] = 90;
    }
}

void Socks4::Info()
{
    char s_ip_cstr[INET_ADDRSTRLEN];
    int s_port = ntohs(client_addr.sin_port);
    inet_ntop(AF_INET, &client_addr.sin_addr, s_ip_cstr, INET_ADDRSTRLEN);
    cout << "<S_IP>:    " << s_ip_cstr << endl
         << "<S_PORT>:  " << s_port << endl
         << "<D_IP>:    " << sp_dst_ip_cstr << endl
         << "<D_PORT>:  " << sp_dst_port << endl
         << "<Command>: " << ((sp_cd == 1) ? "CONNECT" : "BIND") << endl
         << "<Reply>:   " << ((reply[1] == 90) ? "Accept" : "Reject") << endl
         << endl;
}

void Socks4::Reject()
{
    /* reply reject */
    if (reply[1] == 91) {
        for (int i = 2; i < 8; i++) {
            reply[i] = request[i];
        }
        write(client_fd, reply, 8);
        exit(0);
    }
}

void Socks4::ConnectMode()
{
    fd_set read_fdset, act_fdset;
    FD_ZERO(&act_fdset);
    /* reply accept */
    for (int i = 2; i < 8; i++) {
        reply[i] = request[i];
    }
    write(client_fd, reply, 8);
    int target_fd = Server::genClient(sp_dst_ip_cstr, sp_dst_port);
    /* transport data between socks client & target server */
    int maxfd_num = max(client_fd, target_fd) + 1;
    FD_SET(target_fd, &act_fdset);
    FD_SET(client_fd, &act_fdset);
    while (true) {
        memcpy(&read_fdset, &act_fdset, sizeof(read_fdset));
        if (select(maxfd_num, &read_fdset, NULL, NULL, NULL) > 0) {
            if (FD_ISSET(client_fd, &read_fdset)) {
                transport(client_fd, target_fd);
            }
            if (FD_ISSET(target_fd, &read_fdset)) {
                transport(target_fd, client_fd);
            }
        }
    }
}

void Socks4::BindMode()
{
    fd_set read_fdset, act_fdset;
    FD_ZERO(&act_fdset);
    int bind_fd = Server::genSock(0);
    struct sockaddr_in bind_addr;
    u_int bind_addr_len = sizeof(bind_addr);
    if (getsockname(bind_fd, (struct sockaddr*)&bind_addr, &bind_addr_len) == -1) {
        throw "Can't get sockaddr_in of bind_fd";
    }
    /* reply accept */
    reply[2] = (u_char)(ntohs(bind_addr.sin_port) / 256);
    reply[3] = (u_char)(ntohs(bind_addr.sin_port) % 256);
    for (int i = 4; i < 8; ++i) {
        reply[i] = 0; /* 0.0.0.0 means the same ip to socks server */
    }
    write(client_fd, reply, 8);
    /* connected from ftp server */
    struct sockaddr_in ftp_addr;
    u_int ftp_addr_len = sizeof(ftp_addr);
    int ftp_fd = accept(bind_fd, (struct sockaddr*)&ftp_addr, &ftp_addr_len);
    if (ftp_fd < 0) {
        throw "Accept error : ftp_fd";
    }
    /* check the ip from the ftp server */
    if (sp_dst_ip != ftp_addr.sin_addr.s_addr) {
        reply[1] = 91;
        write(client_fd, reply, 8); /* reply reject */
        char ftp_ip_cstr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ftp_addr.sin_addr, ftp_ip_cstr, INET_ADDRSTRLEN);
        throw "Bad incoming connection, ip = " + string(ftp_ip_cstr);
    }
    /* reply again after being connected from ftp server */
    write(client_fd, reply, 8);
    /* transport data between socks client & ftp server */
    int maxfd_num = max(client_fd, ftp_fd) + 1;
    FD_SET(ftp_fd, &act_fdset);
    FD_SET(client_fd, &act_fdset);
    while (true) {
        memcpy(&read_fdset, &act_fdset, sizeof(read_fdset));
        if (select(maxfd_num, &read_fdset, NULL, NULL, NULL) > 0) {
            if (FD_ISSET(client_fd, &read_fdset)) {
                transport(client_fd, ftp_fd);
            }
            if (FD_ISSET(ftp_fd, &read_fdset)) {
                transport(ftp_fd, client_fd);
            }
        }
    }
}

/* exit(0) if read eof from from_fd */
void Socks4::transport(int from_fd, int to_fd)
{
    char buffer[BUF_SIZE] = { 0 };
    int read_count = read(from_fd, buffer, BUF_SIZE);
    if (read_count == 0) { /* read() eof */
        exit(0);
    } else if (read_count == -1) {
        throw "Read error";
    } else {
        int write_count = write(to_fd, buffer, read_count);
        if (write_count != read_count) {
            cout << "[Warning] from_fd -> to_fd:  write_count != read_count" << endl;
        }
    }
}

Server::Server(int port)
{
    this->sock = genSock(port);
}

int Server::genSock(int port)
{
    struct sockaddr_in sin;
    int sock;
    memset((char*)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    // sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    sin.sin_port = htons(port); // use port 0 and bind will use the next available high port.
    /* Allocate a socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw "can't create socket: ";
    /* Set socket option SO_REUSEADDR = true */
    int opt_val = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)) < 0)
        throw "can't socket option: ";
    /* Bind the socket */
    if (bind(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0)
        throw "can't bind to " + to_string(port) + " port: ";
    if (listen(sock, 30) < 0)
        throw "can't listen on " + to_string(port) + " port: ";
    return sock;
}
Server::~Server()
{
    close(this->sock);
}

void Server::Run()
{
    struct sockaddr_in client_sin;
    int client_sock;
    int socklen;
    while (true) {
        socklen = sizeof(client_sin);
        client_sock = accept(sock, (struct sockaddr*)&client_sin, (socklen_t*)&socklen);
        if (client_sock < 0) {
            throw "accept() failed";
        }
        int pid;
        while ((pid = fork()) < 0) {
            usleep(100);
        }
        if (pid) {
            close(client_sock);
        } else {
            Socks4 service(client_sock, client_sin);
            service.Run();
            break;
        }
    }
}

int Server::genClient(char *ip, int port) {
    struct hostent *phe;    /* pointer to host information entry */
    struct sockaddr_in sin; /* an Internet endpoint address */
    int sock;               /* socket descriptor */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    /* Map service name to port number */
    if ((sin.sin_port = htons((u_short)port)) == 0)
        throw "can't get \"" + to_string(port) + "\" service entry";
    /* Set IP address with dotted decimal */
    if ((sin.sin_addr.s_addr = inet_addr(ip)) == INADDR_NONE)
        throw "can't get \"" + string(ip) +"\" host entry\n";
    /* Allocate a socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw "can't create socket: " ;
    /* Connect the socket */
    if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        throw "can't connect to " + string(ip) + ":"+ to_string(port) + ": " ;
    return sock;
}