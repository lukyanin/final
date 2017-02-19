#include <iostream>
#include <unistd.h>

using namespace std;

void display_usage() {
    cout << "usage: final -h <ip> -p <port> -d <directory>";
    exit(0);
}

int main(int argc, char * const argv[]) {

    // /home/box/final/final -h <ip> -p <port> -d <directory>
    string ip = "127.0.0.1";
    int port = 80;
    string dir;
    int opt;
    static const char *optString = "h:p:d:";
    while( opt = getopt( argc, argv, optString ), opt != -1 ) {
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
/*            case '?':
                display_usage();
                break;
            default:
                cerr << "Unknown option" << endl;
                break;*/
        }
    }
    cout << "pre-final" << endl;
    cout << "ip: " << ip << endl;
    cout << "port: " << port << endl;
    cout << "dir: " << dir << endl;
    return 0;
}
