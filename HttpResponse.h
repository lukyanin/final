//
// Created by lvv on 22.02.17.
//

#ifndef FINAL_HTTPRESPONSE_H
#define FINAL_HTTPRESPONSE_H

#include <vector>
#include <string>

class HttpResponse {
public:
    HttpResponse() {}
    HttpResponse(const std::string &version, int code, const char *reason,
                 const char *content_type = nullptr, FILE * f = nullptr, const std::string * text = nullptr);
    void Send(int fd);
protected:
    void AddLine(const std::string &line);
    std::vector<char> buffer;
    FILE * file = 0;
    size_t content_size = 0;
};


#endif //FINAL_HTTPRESPONSE_H
