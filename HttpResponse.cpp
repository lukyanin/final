//
// Created by lvv on 22.02.17.
//

#include "HttpResponse.h"
#include <string>
#include <sys/socket.h>
#include <unistd.h>

HttpResponse::HttpResponse(const std::string &version, int code, const char *reason,
                           const char *content_type, FILE * f, const std::string * text) {

    static std::string sContentType = "Content-Type: ";
    static std::string sContentLength = "Content-Length: ";

    AddLine(version + " " + std::to_string(code) + " " + reason);
    if (content_type)
        AddLine(sContentType + content_type);

    content_size = 0;
    if (f) {
        fseek(f, 0, SEEK_END);
        content_size = ftell(f);
        fseek(f, 0, SEEK_SET);
    }
    else if(text)
        content_size = text->size();
    if(content_size)
        AddLine(sContentLength + std::to_string(content_size));

    AddLine(std::string());
    file = f;
    if(text)
        AddLine(*text);

}


static bool SendNonBlocking(int fd, const char *bfr, size_t len) {
    size_t to_send = len;
    while(to_send) {
        ssize_t sent = send(fd, bfr, to_send, MSG_NOSIGNAL);
        if(sent < 0) {
            if(errno == EAGAIN) {
                usleep(50000);
                continue;
            }
            return false;
        }
        to_send -= sent;
        bfr += sent;
    }
    return true;
}

void HttpResponse::Send(int fd)
{
    if(!SendNonBlocking(fd, &buffer[0], buffer.size()))
        return;
    std::vector<char> bfr;
    bfr.resize(1024*128);
    if(file) {
        size_t rest = content_size;
        while(rest) {
            size_t now = rest < bfr.size() ? rest : bfr.size();
            now = fread(&bfr[0], 1, now, file);
            if(!SendNonBlocking(fd, &bfr[0], now))
                return;
            rest -= now;
        }
    }
}

void HttpResponse::AddLine(const std::string &line) {
    buffer.insert(buffer.end(), line.begin(), line.end());
    const char *eol = "\r\n";
    buffer.insert(buffer.end(), eol, eol + 2);
}
