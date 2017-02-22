//
// Created by lvv on 20.02.17.
//

#include <fcntl.h>
#include "WebServer.h"
#include "HttpRequest.h"
#include "Utils.h"
#include "HttpResponse.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>


#define MAX_EVENTS 32

void WebServer::PrintFile(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if(fd < 0) {
        std::cerr << "Can't open " << filename << std::endl;
        return;
    }
    char bfr[2000];
    std::cout << filename << std::endl;
    std::cout << "\n------" << std::endl;
    while(true) {
        int rd = read(fd, bfr, 2000);
        write(STDOUT_FILENO, bfr, rd);
        if(rd < 2000)
            break;
    }
    close(fd);
    std::cout << "\n------" << std::endl;
}

bool WebServer::Initialize() {
    if(!InitMasterSocket())
        return false;

    epoll_instance = epoll_create1(0);
    if(epoll_instance == -1) {
        std::cerr << "epoll_create1 returned " << epoll_instance << std::endl;
        return false;
    }

    AddToEpoll(master_socket);
    return true;
}

void WebServer::Run(const std::string& ip_address, int port_n, const std::string& working_dir) {
    ip_addr = ip_address;
    port = port_n;
    work_dir = working_dir;
    if(!work_dir.empty() && work_dir[work_dir.size()-1] == '/')
        work_dir.pop_back();

    if(!Initialize())
        return;
    stopped = false;

    // main server loop
    while(!stopped) {
        epoll_event Events[MAX_EVENTS];
        int ev_count = epoll_wait(epoll_instance, Events, MAX_EVENTS, -1);

        for(int i = 0; i < ev_count; ++i) {
            int fd = Events[i].data.fd;
            if(fd == master_socket) {
                OnNewConnection();
            } else {
                OnReceive(fd);
            }
        }

    }
    Finalize();
}

void WebServer::OnNewConnection()
{
    sockaddr_storage addr;
    char ipstr[INET6_ADDRSTRLEN];
    int port;
    socklen_t len = sizeof(addr);
    int ss = accept(master_socket, (sockaddr*) &addr, &len);

    // deal with both IPv4 and IPv6:
    if (addr.ss_family == AF_INET) {
        sockaddr_in *s = (sockaddr_in *)&addr;
        port = ntohs(s->sin_port);
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
    } else { // AF_INET6
        sockaddr_in6 *s = (sockaddr_in6 *)&addr;
        port = ntohs(s->sin6_port);
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
    }
    std::string client_name = std::string(ipstr) + ":" + std::to_string(port);

    std::cout << "connection " << ss << " from " << client_name << " accepted" << std::endl;
    SetNonblock(ss);
    //clients.insert(ss);
    AddToEpoll(ss);
}

void WebServer::OnReceive(int fd)
{
    /*
    std::vector<char> req;
    auto itFound = partial_requests.find(fd);
    if(itFound != partial_requests.end()) {
        req = std::move(itFound->second);
        partial_requests.erase(itFound);
    }*/
    char buffer[1025];
//    while(true) {
        ssize_t r = recv(fd, buffer, 1024, MSG_NOSIGNAL);
        if (r <= 0 && errno != EAGAIN) {
            OnConnectionClosed(fd);
            //break;
        }
        else if (r > 0) {
      //      req.insert(req.end(), buffer, buffer+ r);
            std::string request(buffer, buffer+ r);
            OnRequest(fd, request);
            //buffer[r] = '\0';
            //std::cout << "recevied <<<" << buffer << ">>>" << std::endl;
        }
 //   }
}

void WebServer::OnConnectionClosed(int fd)
{
    shutdown(fd, SHUT_RDWR);
    close(fd);
    partial_requests.erase(fd);
    std::cout << "connection closed\n" << std::endl;
}

void WebServer::Finalize()
{
    if(epoll_instance != -1)
        close(epoll_instance);
    epoll_instance = -1;

    if(master_socket != -1)
        close(master_socket);
    master_socket = -1;
}

int WebServer::SetNonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1)
        flags = 0;
    return fcntl(fd, F_SETFL,  flags | O_NONBLOCK);
}

bool WebServer::InitMasterSocket()
{
    master_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(master_socket == -1) {
        std::cerr << "error creating socket" << std::endl;
        return false;
    }

    sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    inet_pton(AF_INET, ip_addr.c_str(), &(sa.sin_addr));
// now get it back and print it
//    inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);

   // sa.sin_addr.s_addr = htonl(INADDR_ANY);
    auto bres = bind(master_socket,(sockaddr*)&sa, sizeof(sa));
    if(bres) {
        std::cerr << "bind returned " << bres << std::endl;
        return false;
    }
    SetNonblock(master_socket);

    auto lres = listen(master_socket, SOMAXCONN);
    if(lres) {
        std::cerr << "listen returned " << lres << std::endl;
        return false;
    }
    return true;
}

void WebServer::AddToEpoll(int fd)
{
    epoll_event Event;
    Event.data.fd = fd;
    Event.events = EPOLLIN;
    epoll_ctl(epoll_instance, EPOLL_CTL_ADD, fd, &Event);
}

void WebServer::OnRequest(int fd, const std::string& request)
{
    HttpRequest r(request);

    if(r.GetMethod() == HttpRequest::Method::GET) {
        std::string uri = r.GetUri();
        std::string filename = GetUriFilename(uri);
        std::cout << "Client wants '" << r.GetUri() << "' that is '" + filename + "' " << std::endl;
        OnGetRequest(fd, r.GetVersion(), filename);
    }
    else {
        // TODO close fd?
    }
}

std::string WebServer::GetUriFilename(const std::string &uri)
{
    std::vector<std::string> path;
    ssize_t pos;
    std::string next;
    while(next = Utils::Tokenize(uri, pos, "/\\"), !next.empty()) {
        if(next == "." || next == "~")
            continue;
        if(next == "..") {
            if(!path.empty())
                path.pop_back();
            continue;
        }
        path.push_back(next);
    }
    std::string fullpath = work_dir;
    for(auto &s : path) {
        fullpath += "/";
        fullpath += s;
    }
    return fullpath;
}

void WebServer::OnGetRequest(int fd, const std::string& version, const std::string& filename) {

    static std::string sVersion = "HTTP/1.0";

    HttpResponse resp;

    FILE * f = fopen(filename.c_str(), "rb");

    if (f < 0) {
        resp = HttpResponse(sVersion, 404, "Not found");
    }
    else {
        resp = HttpResponse(sVersion, 200, "OK", "text/html", f);
    }
    if(f)
        fclose(f);
    send(fd, resp.data(), resp.size(), MSG_NOSIGNAL);
}