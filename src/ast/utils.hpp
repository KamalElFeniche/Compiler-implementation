#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdlib>
#include <string>

// Converts an escape sequence to char
char escape_to_char(const char* c);
// Converts an hexadecimal sequence to char
char hex_to_char(std::string s);
// Converts a char to an hexadecimal representation
std::string char_to_hex(char c);


#endif