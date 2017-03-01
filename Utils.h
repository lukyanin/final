//
// Created by lvv on 22.02.17.
//

#ifndef FINAL_UTILS_H
#define FINAL_UTILS_H
#include <string>


namespace Utils {
    std::string GetLine(const std::string& source, ssize_t& pos);   // "\r\n" or "\r" or "\n" or "\n\r"
    std::string Tokenize(const std::string& source, ssize_t& pos, const char *delimiters);   // "\r\n" or "\r" or "\n" or "\n\r"
    std::string Strip(const std::string& source, const char *white = " \t");
    std::string GetFileExtension(const std::string& filename);
}


#endif //FINAL_UTILS_H
