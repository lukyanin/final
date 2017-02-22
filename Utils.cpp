//
// Created by lvv on 22.02.17.
//
#include <string.h>
#include "Utils.h"

namespace Utils {

    std::string GetLine(const std::string &source, ssize_t &pos) {
        for (ssize_t end = pos; end < source.size(); end++) {
            char ch = source[end];
            if (ch == '\r' || ch == '\n') {
                std::string line = source.substr(pos, end - pos);
                pos = end + 1;
                if (pos < source.size() && (source[pos] ^ ch) == ('\r' ^ '\n'))
                    ++pos;
                return line;
            }
        }
        std::string line = source.substr(pos);
        pos = source.size();
        return line;
    }

    std::string Tokenize(const std::string& source, ssize_t& pos, const char *delimiters) {
        while(pos < source.size() && strchr(delimiters, source[pos]))
            ++pos;
        ssize_t start = pos;
        while(pos < source.size() && !strchr(delimiters, source[pos]))
            ++pos;
        return source.substr(start, pos - start);
    }

    std::string Strip(const std::string& source, const char *white) {
        ssize_t start = 0, stop = source.size();
        while(start < source.size() && strchr(white, source[start]))
            ++start;
        while(stop > start && strchr(white, source[stop - 1]))
            --stop;
        return source.substr(start, stop - start);
    }
} // namespace Utils