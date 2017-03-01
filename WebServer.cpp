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
#include <sys/stat.h>
#include <dirent.h>
#include <sstream>
#include <algorithm>
#include <thread>


#define MAX_EVENTS 32

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

    int work_threads_count = std::thread::hardware_concurrency();
    if(!work_threads_count)
        work_threads_count = 1;
    std::vector<std::thread> thread_pool;
    for(int i = 0; i < work_threads_count; i++)
        thread_pool.emplace_back(&WebServer::WoringThread, this);

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

    for(int i = 0; i < work_threads_count; i++) {
        std::unique_lock<std::mutex> lock(tx_queue);
        condition_queue.notify_one();
    }

    for(auto &th : thread_pool)
        th.join();
    Finalize();
}

void WebServer::WoringThread()
{
    while(!stopped) {
        Request rq;
        rq.fd = 0;
        {
            std::unique_lock<std::mutex> lock(tx_queue);
            condition_queue.wait(lock, [this] { return stopped || !req_queue.empty(); });
            if (!req_queue.empty()) {
                rq = req_queue.front();
                req_queue.pop();
            }
        }
        if (rq.fd)
            OnRequest(rq.fd, rq.raw_request);
    }
}

void WebServer::PushRequest(int fd, std::string&& request) {
    std::unique_lock<std::mutex> lock(tx_queue);
    req_queue.push({fd, std::move(request) });
    condition_queue.notify_one();
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

    //std::cout << "connection " << ss << " from " << client_name << " accepted" << std::endl;
    SetNonblock(ss);
    //clients.insert(ss);
    AddToEpoll(ss);
}

static bool IsFullRequest(const std::string &req) {
    // reuest is full if it has "\r\n\r\n"
    return req.find("\r\n\r\n") != std::string::npos;
}

void WebServer::OnReceive(int fd) {
    std::string req;
    auto itFound = partial_requests.find(fd);
    if(itFound != partial_requests.end()) {
        req = std::move(itFound->second);
        partial_requests.erase(itFound);
    }

    char buffer[1025];
//    while(true) {
        ssize_t r = recv(fd, buffer, 1024, MSG_NOSIGNAL);
        if (r <= 0 && errno != EAGAIN) {
            OnConnectionClosed(fd);
            //break;
        }
        else if (r > 0) {
            req.insert(req.end(), buffer, buffer+ r);
            if(IsFullRequest(req))
                PushRequest(fd, std::move(req));
            else
                partial_requests[fd] = req;
            //std::cout << "recevied <<<" << buffer << ">>>" << std::endl;
        }
 //   }
}

void WebServer::OnConnectionClosed(int fd)
{
    shutdown(fd, SHUT_RDWR);
    close(fd);
    partial_requests.erase(fd);
    //std::cout << "connection " << fd << " closed" << std::endl;
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

void WebServer::AddToEpoll(int fd) {
    epoll_event Event;
    Event.data.fd = fd;
    Event.events = EPOLLIN;
    epoll_ctl(epoll_instance, EPOLL_CTL_ADD, fd, &Event);
}

void WebServer::RemoveFromEpoll(int fd) {
    epoll_ctl(epoll_instance, EPOLL_CTL_DEL, fd, nullptr);
}

void WebServer::OnRequest(int fd, const std::string& request)
{
    HttpRequest r(request);

    if(r.GetMethod() == HttpRequest::Method::GET) {
        std::string uri = r.GetUri();
        //std::cout << "Client wants '" << r.GetUri() << "'" << std::endl;
        std::string filename = GetUriFilename(uri);
        //std::cout << "That is      '" + filename + "' " << std::endl;
        OnGetRequest(fd, r.GetVersion(), filename);
    }
    else {
        // TODO close fd?
    }
}

std::string WebServer::GetUriFilename(const std::string &uri)
{
    std::vector<std::string> path;
    ssize_t pos = 0;
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

std::string GetDirListHtml(DIR* dir, const std::string &dirname, const std::string &fulldirname) {
    std::stringstream text;
    text << "<html><head><title>List of /";
    text << dirname << "</title></head><body><H1>List of /" << dirname << "</H1>";

    text << "<ul>";
    while (dirent * ent = readdir(dir)) {
        if (ent->d_name[0] == '.')
            continue;
        std::string file_name = ent->d_name;
        std::string full_file_name = fulldirname + "/" + file_name;

        struct stat st;
        if (stat(full_file_name.c_str(), &st) == -1)
            continue;

        if (st.st_mode & S_IFDIR)  // is directory
            file_name += "/";

        text << "<li><a href=\"" << file_name.c_str() << "\">" << file_name.c_str() << "</a></li>";
    }
    text << "</ul></body></html>";
    return text.str();
}

const char * GetContentType(const std::string &filename) {
    std::string ext = Utils::GetFileExtension(filename);
    if (!ext.empty())
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == "txt")
        return "text/plain";
    if (ext == "htm" || ext == "html")
        return "text/html";
    if (ext == "ico")
        return "image/x-icon";
    if (ext == "jpg" || ext == "jpeg")
        return "image/jpeg";
    if (ext == "png")
        return "image/jpeg";
    return nullptr;
}

static std::string notFound = "<html><head><title>Not found</title></head><body><H1>:(</H1></body></html>";

void WebServer::OnGetRequest(int fd, const std::string& version, const std::string& filename) {

    static std::string sVersion = "HTTP/1.0";

    HttpResponse resp;

    FILE * f = nullptr;
    struct stat st;
    if (stat(filename.c_str(), &st) == -1) {
        resp = HttpResponse(sVersion, 404, "Not found", nullptr, nullptr, &notFound);
    }
    else if (st.st_mode & S_IFDIR) {
        DIR* dir = opendir(filename.c_str());
        std::string dirlist = GetDirListHtml(dir, filename.substr(work_dir.size()), filename);
        resp = HttpResponse(sVersion, 200, "OK", "text/html", nullptr, &dirlist);
        closedir(dir);
    }
    else {
        f = fopen(filename.c_str(), "rb");
        resp = HttpResponse(sVersion, 200, "OK", GetContentType(filename), f);
    }
    resp.Send(fd);
    if(f)
        fclose(f);

//    shutdown(fd, SHUT_RDWR);
    RemoveFromEpoll(fd);
    OnConnectionClosed(fd);
}
