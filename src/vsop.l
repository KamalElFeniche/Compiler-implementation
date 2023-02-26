%{  
    #define YY_USER_ACTION update();
    #include <stack>
    #include <iostream>
    #include <unordered_map>
    #include <string>
    #include <sstream>
    #include <iomanip>
    #include "vsop.tab.h"
    #include "ast/utils.hpp"

    extern int mode;

    extern void yyerror(const std::string& text);

    /* Stack used for remembering position of opening
        comment and opening string. */
    std::stack<YYLTYPE> yystack;

    /* Buffer used to store the possible string-literal
     when an opening quote is seen */
    std::string buffer;

    std::unordered_map<std::string, int> keywords = {
        {"and", AND},
        {"bool", BOOL},
        {"class", CLASS},
        {"do", DO},
        {"else", ELSE},
        {"extends", EXTENDS},
        {"false", FALSE},
        {"if", IF},
        {"in", IN},
        {"int32", INT32},
        {"isnull", ISNULL},
        {"let", LET},
        {"new", NEW},
        {"not", NOT},
        {"self", SELF},
        {"string", STRING},
        {"then", THEN},
        {"true", TRUE},
        {"unit", UNIT},
        {"while", WHILE}
    };

    std::unordered_map<std::string, int> operators = {
        {"{", LBRACE},
        {"}", RBRACE},
        {"(", LPAR},
        {")", RPAR},
        {":", COLON},
        {";", SEMICOLON},
        {",", COMMA},
        {"+", PLUS},
        {"-", MINUS},
        {"*", TIMES},
        {"/", DIV},
        {"^", POW},
        {".", DOT},
        {"=", EQUAL},
        {"<", LOWER},
        {"<=", LOWER_EQUAL},
        {"<-", ASSIGN}
    };

    /**
    * Update the position in the file.
    *
    */
    void update(){
        yylloc.first_column = yylloc.last_column;
        yylloc.first_line = yylloc.last_line;

        for (int i = 0; i < yyleng; i++){
            if (yytext[i] == '\n'){
                yylloc.last_line++;
                yylloc.last_column = 1;
            }else{
                yylloc.last_column++;
            }
        }
    }

    /** Checks if a string can be represented as an integer-literal.
      *
      * @param text a string which contains a supposed integer-literal.
      * @return true if it can be represented as an integer-literal, or false if not.
      *
      * Example: checkString('42') -> true
      * Example: checkString('42x') -> false
      * Example: checkString('0x1b') -> 27
      * Example: checkString('0xz') -> false
      */
    bool checkString(std::string text){
        size_t len = text.length();
        
        if(len > 2 && text[0] == '0' && text[1] == 'x'){
            for (size_t i = 2; i < len; i++){
                if (text[i] >= '0' && text[i] <= '9'){
                    continue;
                }
                else if (text[i] >= 'a' && text[i] <= 'f'){
                    continue;
                }
                else if (text[i] >= 'A' && text[i] <= 'F'){
                    continue;
                } else {
                    return false;
                }
            }
            return true;
        }

        for (size_t i = 0; i < len; i++){
            if (text[i] < '0' || text[i] > '9')
                return false;
        }
        
        return true;
    }

%}
%option noyywrap
%x COMMENT STRING


lowercase_letter [a-z]
uppercase_letter [A-Z]
letter {lowercase_letter}|{uppercase_letter}
bin-digit [0-1]
digit [0-9]
hexdigit {digit}|[A-Fa-f]
integer-literal {digit}+|"0x"{hexdigit}+
whitespace " "|\x09|\x0a|\x0c|\x0d  
type-identifier {uppercase_letter}({letter}|{digit}|"_")*
object-identifier {lowercase_letter}({letter}|{digit}|"_")*

regular-char [^\0\n\"\\]
escape-char [btnr\"\\]|(x{hexdigit}{2})|(\n[ \t]*)
escape-sequence \\{escape-char}
string-literal ({regular-char}|{escape-char})*
may-follow-integer {letter}|{digit}|_

operator "{"|"}"|"("|")"|":"|";"|","|"+"|"-"|"*"|"/"|"^"|"."|"="|"<"|"<="|"<-"
inLineComment "//"[^\0\n]*
commentBegin "(*" 
commentEnding "*)"
bad_hex [g-zG-Z]
wronginteger "0x"({hexdigit}*{bad_hex}+{hexdigit}*)*
nonascii [^\x00-\x7F]

%%

%{
    switch(mode){
        
        case START_LEX:
            mode = 0;
            return START_LEX;
        case START_PARSE:
            mode = 0;
            return START_PARSE;
        default:
            break;
    }
%}


<INITIAL>\"             {   buffer = "";
                            BEGIN(STRING); 
                            yystack.push(yylloc);}


<STRING>{regular-char}+ {
                            buffer += yytext;
              
                            }

<STRING>{escape-sequence}   {
                                if (yytext[1] != '\n')
                                    buffer += escape_to_char(yytext);
                            }

<STRING>\"              {        
                            BEGIN(INITIAL);
                            // Change the first_line & first_column to print correctly the position of the string-literal
                            yylloc.first_line = yystack.top().first_line;
                            yylloc.first_column = yystack.top().first_column;
                            yystack.pop();
                            yylval.sval = strdup(buffer.c_str());
                            return STR_LITERAL;}

<STRING><<EOF>> {   yylloc = yystack.top();
                    yyerror("lexical error: String not closed before end-of-file !");
                    }


<INITIAL>"(*" {yystack.push(yylloc);
                BEGIN(COMMENT);}


<COMMENT><<EOF>>    {yylloc = yystack.top();
                    yyerror("lexical error: Comment not closed before end-of-file !"); 
                    }


<COMMENT>"(*"       {yystack.push(yylloc);}


<COMMENT>{commentEnding} {yystack.pop(); 
                          if (yystack.empty()){
                               BEGIN(INITIAL);
                            }
                        }


<COMMENT>[^\0] {}


<INITIAL>{inLineComment} {}


<INITIAL>{operator} {yylval.sval = strdup(yytext);
                        return operators.at(yytext);}


<INITIAL>{whitespace} {}


<INITIAL>{wronginteger} {yyerror("lexical error: Invalid integer-literal"); }


<INITIAL>{integer-literal}{may-follow-integer}* {   std::string text = "";
                                                    int integer;
                                                    if (!checkString(yytext)){
                                                        yyerror("lexical error: Invalid integer-literal");
                                                        
                                                    }
                                                    if (yytext[0] == '0' && yytext[1] == 'x')
                                                        integer = std::stoi(yytext, nullptr, 16);
                                                    else
                                                        integer = std::stoi(yytext);
                                                    yylval.val = integer;
                                                    return INT_LITERAL;
                                                }


<INITIAL>{object-identifier} {  if (keywords.find(yytext) != keywords.end()){
                                    yylval.sval = strdup(yytext);
                                    return keywords.at(yytext);
                                }
                                else{
                                    yylval.sval = strdup(yytext);
                                    return OBJECT_IDENTIFIER;
                                }
                             }


<INITIAL>{type-identifier} {    std::string text = yytext;
                                yylval.sval = strdup(yytext);
                                return TYPE_IDENTIFIER;
                            }


<INITIAL>{nonascii} {yyerror("lexical error: Non VSOP character detected !"); }


<*>.|\n {yyerror("lexical error: Invalid character !"); }

%%