#include <iostream>
#include <vector>
#include <unistd.h>
#include "WebServer.h"

using namespace std;

string get_cwd() {
    std::vector<char> path;
    for(int s = 100; s < 1000; s *= 2) {
        path.resize(s);
        if(getcwd(&path[0], s))
            return string(&path[0]);
        else if (errno != ERANGE)
            break;
    }
    return string();
}

int main(int argc, char * const argv[]) {

    // /home/box/final/final -h <ip> -p <port> -d <directory>
    string ip;
    int port = -1;
    string dir;
    int opt;
    while(opt = getopt( argc, argv, "h:p:d:"), opt != -1) {
        switch( opt ) {
            case 'h':
                ip = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'd':
                dir = optarg;
                break;
        }
    }
    if(ip.empty() || port == -1 || dir.empty()) {
        cerr << "usage: /home/box/final/final -h <ip> -p <port> -d <directory>" << endl;
        exit(1);
    }
    //cout << "ip: " << ip << endl;
    //cout << "port: " << port << endl;
    //cout << "dir: " << dir << endl;

    // make daemon
    pid_t pid = fork();
    if(pid == -1) {
        cerr << "failed to fork" << endl;
        exit(1);
    }
    if(pid != 0) {
        exit(0); // ok, it is parent, we have done daemon
    }

    chdir("/");
    setsid(); // create new session
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    WebServer server;
//    server.PrintFile("/home/lvv/c++course/final/index.htm");
 //   server.PrintFile("index.htm");
    server.Run(ip, port, dir);
    return 0;
}
