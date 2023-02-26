#include "utils.hpp"

using namespace std;

char escape_to_char(const char* c){

    if (c[1] == 'x')
        return hex_to_char(c);

    if (c[1] == 'b')
        return '\b';
    
    if (c[1] == 't')
        return '\t';

    if (c[1] == 'n')
        return '\n';

    if (c[1] == 'r')
        return '\r';

    return c[1];
}

char hex_to_char(string s){

    s[0] = '0';
    return strtol(&s[0], NULL, 16);
}

string char_to_hex(char c){

    string final = "\\x00";

    char d = c / 16;

    if (d < 10)
        final[2] = d + '0';
    else
        final[2] = d + 'a' - 10;

    d = c % 16;

    if (d < 10)
        final[3] = d + '0';
    else
        final[3] = d + 'a' - 10;

    return final;
}