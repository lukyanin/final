//
// Created by lvv on 22.02.17.
//
#include <string.h>
#include "Utils.h"

/*
string (1)
size_t find_first_not_of (const string& str, size_t pos = 0) const noexcept;
c-string (2)
size_t find_first_not_of (const char* s, size_t pos = 0) const;
buffer (3)
size_t find_first_not_of (const char* s, size_t pos, size_t n) const;
character (4)
size_t find_first_not_of (char c, size_t pos = 0) const noexcept;
string (1)
size_t find_first_of (const string& str, size_t pos = 0) const noexcept;
c-string (2)
size_t find_first_of (const char* s, size_t pos = 0) const;
buffer (3)
size_t find_first_of (const char* s, size_t pos, size_t n) const;
character (4)
size_t find_first_of (char c, size_t pos = 0) const noexcept;
*/
namespace Utils {

    std::string GetLine(const std::string &source, ssize_t &pos) {
	// ins
	size_t eol = source.find_first_of("\r\n", pos);
	std::string line;
	if(eol == std::string::npos) {
	    line = source.substr(pos);
	    pos = source.size();
	} else {
	    line = source.substr(pos, eol - pos);
	    pos = eol + 1;
	    // if the next symbol is pair symbol
	    if(pos < source.size() &&
		(source[pos] ^ source[eol]) == ('\r' ^ '\n'))
		++pos;
	}
	return line;
	// end
/*
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
*/
    }

    std::string Tokenize(const std::string& source, ssize_t& pos, const char *delimiters) {
	size_t start = source.find_first_not_of(delimiters, pos);
	if(start != std::string::npos) {
	    pos = source.find_first_of(delimiters, start);
	    if(pos == std::string::npos)
		pos = source.size();
	    return source.substr(start, pos - start);
	}
	pos = source.size();
	return std::string();
/*
        while(pos < source.size() && strchr(delimiters, source[pos]))
            ++pos;
        ssize_t start = pos;
        while(pos < source.size() && !strchr(delimiters, source[pos]))
            ++pos;
        return source.substr(start, pos - start);
*/
    }

    std::string Strip(const std::string& source, const char *white) {
	size_t start = source.find_first_not_of(white);
	size_t stop = source.find_last_not_of(white);
	if(start == std::string::npos || stop == std::string::npos)
	    return std::string();
	return source.substr(start, stop - start + 1);
/*
        ssize_t start = 0, stop = source.size();
        while(start < source.size() && strchr(white, source[start]))
            ++start;
        while(stop > start && strchr(white, source[stop - 1]))
            --stop;
        return source.substr(start, stop - start);
*/
    }
} // namespace Utils
