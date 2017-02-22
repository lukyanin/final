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
                 const char *content_type = nullptr, FILE * f = nullptr);
    const char * data() {return buffer.empty()? nullptr : &buffer[0];}
    size_t size() {return buffer.size();}
protected:
    void AddLine(const std::string &line);
    std::vector<char> buffer;
};


#endif //FINAL_HTTPRESPONSE_H
