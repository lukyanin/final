//
// Created by lvv on 22.02.17.
//

#include "HttpRequest.h"
#include "Utils.h"
#include <iostream>

using namespace std;
//GET / HTTP/1.1
//Host: 192.168.0.12:8800
//User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:51.0) Gecko/20100101 Firefox/51.0
//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
//Accept-Language: en-US,en;q=0.5
//Accept-Encoding: gzip, deflate
//Connection: keep-alive
//        Upgrade-Insecure-Requests: 1

HttpRequest::HttpRequest(const std::string& req)
{
    ssize_t pos = 0, lpos = 0;
    string line = Utils::GetLine(req, pos);
    cout << "request " << line << endl;

    method = Utils::Tokenize(line, lpos, " ");
    uri = Utils::Tokenize(line, lpos, " ");
    version = Utils::Tokenize(line, lpos, " ");
    if(method == "PUT")
        e_method = Method ::PUT;
    else if(method == "GET")
        e_method = Method ::GET;
    else if(method == "POST")
        e_method = Method ::POST;
    else if(method == "HEAD")
        e_method = Method ::HEAD;

    while(line = Utils::GetLine(req, pos), !line.empty()) {
        auto dp = line.find(':');
    	//cout << "line='" << line << "', dp=" << dp << endl;
        if(dp == string::npos)
            continue;
        headers[line.substr(0, dp)] = Utils::Strip(line.substr(dp + 1));
    }
    // cout << "Request prepared" << endl;
}

std::string HttpRequest::GetHeader(const std::string& key) const {
    auto itFound = headers.find(key);
    return itFound == headers.end()? std::string() : itFound->second;
}
