//
// Created by lvv on 20.02.17.
//

#ifndef FINAL_WEBSERVER_H
#define FINAL_WEBSERVER_H

#include <string>
#include <vector>
#include <map>

class WebServer {
public:
    WebServer() {
    }

    ~WebServer() {
        Finalize();
    }
    void Run(const std::string& ip_address, int port_n, const std::string& working_dir);
    // debug
    void PrintFile(const char *filename);

protected:
    bool Initialize();            // creates master socket and epoll instance
    bool InitMasterSocket();
    static int SetNonblock(int fd);
    void Finalize();              // destroys all
    void AddToEpoll(int fd);
    void OnNewConnection();       // from master_socket, we must accept and handle new client
    void OnReceive(int fd);       // received or disconnected
    void OnConnectionClosed(int fd);
    void OnRequest(int fd, const std::string& request);
    void OnGetRequest(int fd, const std::string& version, const std::string& filename);
    std::string GetUriFilename(const std::string &uri);
    std::string ip_addr;   // own address
    int         port = 0;      // own port
    std::string work_dir;  // own work dir

/*    // clients list
    struct Client {
        int fd;
        std::string raw_request;
    };
    std::vector<Client> clients;*/
    std::map<int, std::vector<char>> partial_requests;

    int master_socket = -1;   // listening socket
    int epoll_instance = -1;
    bool stopped = false;
};


#endif //FINAL_WEBSERVER_H
