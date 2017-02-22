//
// Created by lvv on 22.02.17.
//

#ifndef FINAL_HTTPREQUEST_H
#define FINAL_HTTPREQUEST_H
#include <string>
#include <map>

class HttpRequest {
public:
    HttpRequest(const std::string& req);

    enum class Method {
        UNKNOWN,
        PUT,
        GET,
        POST,
        HEAD
    };
    Method GetMethod() const {return e_method;}
    std::string GetUri() const {return uri;}
    std::string GetVersion() const {return version;}
    std::string GetHeader(const std::string& key) const;
protected:
    Method e_method = Method::UNKNOWN;
    std::string method;
    std::string uri;
    std::string version;
    std::map<std::string, std::string> headers;
};


#endif //FINAL_HTTPREQUEST_H
