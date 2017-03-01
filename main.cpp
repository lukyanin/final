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
    string ip = "0.0.0.0";
    int port = 80;
    string dir = get_cwd();
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
    //cout << "ip: " << ip << endl;
    //cout << "port: " << port << endl;
    //cout << "dir: " << dir << endl;

//    chdir(dir.c_str());
    WebServer server;
//    server.PrintFile("/home/lvv/c++course/final/index.htm");
 //   server.PrintFile("index.htm");
    server.Run(ip, port, dir);
    return 0;
}
