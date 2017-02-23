//
// Created by lvv on 22.02.17.
//

#include "HttpResponse.h"
#include <string>

/*
mime types
text/plain.txt
image/x-icon .ico
image/jpeg.jpeg, .jpg
image/png.png
text/html.html
*/

HttpResponse::HttpResponse(const std::string &version, int code, const char *reason,
                           const char *content_type, FILE * f) {

    static std::string sContentType = "Content-Type: ";
    static std::string sContentLength = "Content-Length: ";

    AddLine(version + " " + std::to_string(code) + " " + reason);
    if (content_type)
        AddLine(sContentType + content_type);

    size_t content_size = 0;
    if (f) {
        fseek(f, 0, SEEK_END);
        content_size = ftell(f);
        fseek(f, 0, SEEK_SET);
        AddLine(sContentLength + std::to_string(content_size));
    }
    AddLine(std::string());

    if(f) {
        size_t hdr_size = buffer.size();
        buffer.resize(hdr_size + content_size);
        fread(&buffer[hdr_size], 1, content_size, f);
    }

    // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
    /*
HTTP/1.0 200 OK
Date: Fri, 08 Aug 2003 08:12:31 GMT
Server: Apache/1.3.27 (Unix)
MIME-version: 1.0
Last-Modified: Fri, 01 Aug 2003 12:45:26 GMT
Content-Type: text/html
Content-Length: 2345
** a blank line *
<HTML> ...
     */
    /*
       response-header = Accept-Ranges           ; Section 14.5
                       | Age                     ; Section 14.6
                       | ETag                    ; Section 14.19
                       | Location                ; Section 14.30
                       | Proxy-Authenticate      ; Section 14.33

                       | Retry-After             ; Section 14.37
                       | Server                  ; Section 14.38
                       | Vary                    ; Section 14.44
                       | WWW-Authenticate        ; Section 14.47
     */
}

void HttpResponse::AddLine(const std::string &line) {
    buffer.insert(buffer.end(), line.begin(), line.end());
    const char *eol = "\r\n";
    buffer.insert(buffer.end(), eol, eol + 2);
}
