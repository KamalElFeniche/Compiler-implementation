%code requires{
    #include<iostream>
    #include<string.h>
    #include <memory>
    #include "ast/ast.hpp"
    #include "ast/utils.hpp"

    struct Helper{
    VSOPList<Field> field;
    VSOPList<Method> method;
    };
}

%union{
    int val;
    char* sval;
    Expr* expr;
    Formal* formal;
    Method* method;
    Field* field;
    Class* _class;
    VSOPList<Field>* fields;
    VSOPList<Formal>* formals;
    VSOPList<Expr>* blocks;
    Helper* help;
};

%code requires{
    #define YYLTYPE yyltype
    typedef struct yyltype{
        int first_column = 1;
        int first_line = 1;
        int last_column = 1;
        int last_line = 1;
        std::string filename;
    } yyltype;
    
}
%locations

%{
    void yyerror(const std::string& text);
    void printResult(const std::string& text);
    int yylex(void);
    int yyparse(void);
    extern std::string file_name;
    extern VSOPList<Class> program;
    void setPosition(Node* node, const YYLTYPE& pos);
    void setFileName(Node* node, std::string& file_name);

%}
%define parse.error verbose
%define parse.lac full
%token END

%token <val> INT_LITERAL
%token <sval> STR_LITERAL
%token <sval> OBJECT_IDENTIFIER
%token <sval>TYPE_IDENTIFIER

%token <sval> AND
%token <sval> BOOL
%token <sval> CLASS
%token <sval> DO
%token <sval> ELSE
%token <sval> EXTENDS
%token <sval> FALSE
%token <sval> IF
%token <sval> IN
%token <sval> INT32
%token <sval> ISNULL
%token <sval> LET
%token <sval> NEW
%token <sval> NOT
%token <sval> SELF
%token <sval> STRING
%token <sval> THEN
%token <sval> TRUE
%token <sval> UNIT
%token <sval> WHILE

%token <sval> LBRACE
%token <sval> RBRACE
%token <sval> LPAR
%token <sval> RPAR
%token <sval> COLON
%token <sval> SEMICOLON
%token <sval> COMMA
%token <sval> PLUS
%token <sval> MINUS
%token <sval> TIMES
%token <sval> DIV
%token <sval> POW
%token <sval> DOT
%token <sval> EQUAL
%token <sval> LOWER
%token <sval> LOWER_EQUAL
%token <sval> ASSIGN

%token START_LEX START_PARSE;

%start start;

%nterm <sval> type
%nterm <sval> extends
%nterm <sval> type-id
%nterm <sval> object-id
%nterm <_class> class
%nterm <help> class-body
%nterm <field> field
%nterm <method> method method-aux
%nterm <formal> formal
%nterm <formals> formals 
%nterm <formals> formals-rec
%nterm <blocks> block
%nterm <blocks> block-rec
%nterm <blocks> args
%nterm <blocks> args-rec
%nterm <expr> expr
%nterm <expr> expr-rec
%nterm <expr> if
%nterm <expr> while
%nterm <expr> let
%nterm <expr> binary
%nterm <expr> unary
%nterm <expr> call
%nterm <expr> literal
%nterm <expr> init

%precedence IF THEN WHILE DO LET IN
%precedence ELSE


%right ASSIGN
%left AND
%right NOT
%nonassoc EQUAL LOWER_EQUAL LOWER
%left PLUS MINUS
%left DIV TIMES
%right ISNULL UNARY_MINUS
%right POW
%left DOT


%%

start:           START_LEX token
                |START_PARSE program

token:          
                |token INT_LITERAL
                    {std::string text ="integer-literal," + std::to_string(yylval.val); printResult(text);}
                |token STR_LITERAL
                    {std::string text ="string-literal," ; printResult(text + String($2).print());}
                |token OBJECT_IDENTIFIER
                    {std::string text ="object-identifier," ; printResult(text + yylval.sval);}
                |token TYPE_IDENTIFIER
                    {std::string text ="type-identifier," ; printResult(text + yylval.sval);}
                |token AND
                    {printResult("and");}
                |token BOOL
                    {printResult("bool");}
                |token CLASS
                    {printResult("class");}
                |token DO
                    {printResult("do");}
                |token ELSE
                    {printResult("else");}
                |token EXTENDS
                    {printResult("extends");}
                |token FALSE
                    {printResult("false");}
                |token IF
                    {printResult("if");}
                |token IN
                    {printResult("in");}
                |token INT32
                    {printResult("int32");}
                |token ISNULL
                    {printResult("isnull");}
                |token LET
                    {printResult("let");}
                |token NEW
                    {printResult("new");}
                |token NOT
                    {printResult("not");}
                |token SELF
                    {printResult("self");}
                |token STRING
                    {printResult("string");}
                |token THEN
                    {printResult("then");}
                |token TRUE
                    {printResult("true");}
                |token UNIT
                    {printResult("unit");}
                |token WHILE
                    {printResult("while");}
                |token LBRACE
                    {printResult("lbrace");}
                |token RBRACE
                    {printResult("rbrace");}
                |token LPAR
                    {printResult("lpar");}
                |token RPAR
                    {printResult("rpar");}
                |token COLON
                    {printResult("colon");}
                |token SEMICOLON
                    {printResult("semicolon");}
                |token COMMA
                    {printResult("comma");}
                |token PLUS
                    {printResult("plus");}
                |token MINUS
                    {printResult("minus");}
                |token TIMES
                    {printResult("times");}
                |token DIV
                    {printResult("div");}
                |token POW
                    {printResult("pow");}
                |token DOT
                    {printResult("dot");}
                |token EQUAL
                    {printResult("equal");}
                |token LOWER
                    {printResult("lower");}
                |token LOWER_EQUAL
                    {printResult("lower-equal");}
                |token ASSIGN
                    {printResult("assign");}

program :   program-rec
                | program-rec program

program-rec:    class
                {program.push($1);};

class: CLASS type-id extends LBRACE class-body
                {$$ = new Class($2, $3, $5->field.reverse(), $5->method.reverse());
                    setPosition($$, @$);
                    setFileName($$, file_name);};

extends:        /* EPSILON */
                {$$ = strdup("Object");}
                | EXTENDS type-id
                {$$ = $2;};

class-body:        RBRACE 
                    {$$ = new Helper();}
                    | field SEMICOLON class-body
                    {$3->field.push($1); $$ = $3;}
                    | method class-body
                    {$2->method.push($1); $$ = $2;}
                    | error END
                    {yyerror("syntax error: missing closing bracket"); };

field:              object-id COLON type init
                    {$$ = new Field($1, $3, $4);
                        setPosition($$, @$);
                        setFileName($$, file_name);};

method:             method-aux block
                    {$1->block = std::make_shared<Block>($2->reverse()); $$ = $1; delete $2;
                        setPosition($$, @$);
                        setFileName($$, file_name);};

method-aux:         object-id formals COLON type
                    {$$ = new Method($1, $4, $2->reverse(), NULL); delete $2;
                        setFileName($$, file_name);};

formal:             object-id COLON type
                    {$$ = new Formal($1, $3);
                        setPosition($$, @$);
                        setFileName($$, file_name);};

formals:            LPAR RPAR
                    {$$ = new VSOPList<Formal>();}
                    | LPAR formals-rec
                    {$$ = $2;};

formals-rec:        formal RPAR
                    {$$ = new VSOPList<Formal>(); $$->push($1);}
                    | formal COMMA formals-rec
                    {$3->push($1); $$ = $3;}
                    | error RPAR
                    {yyerror("syntax error: invalid formal"); }
                    | error COLON formals-rec
                    {yyerror("syntax error: invalid formal"); };

object-id:          OBJECT_IDENTIFIER
                    | TYPE_IDENTIFIER
                    {yyerror("syntax error: expected object-identifier but received a type-identifier"); };

type-id:            TYPE_IDENTIFIER
                    | OBJECT_IDENTIFIER
                    {yyerror("syntax error: expected type-identifier but received an object-identifier"); };

type:               type-id
                    | INT32
                    | BOOL
                    | STRING
                    | UNIT;

block:              LBRACE block-rec
                    {$$ = $2;}
                    | LBRACE RBRACE
                    {yyerror("syntax error: block without a body"); };

block-rec:          expr RBRACE
                    {$$ = new VSOPList<Expr>(); $$->push($1);}
                    | expr SEMICOLON block-rec
                    {$3->push($1); $$ = $3;}
                    | error block block-rec
                    {yyerror("syntax error: unmatched {"); }
                    | error END
                    {yyerror("syntax error: unclosed brackets {}"); };

expr:               expr-rec
                    {$$ = $1;
                        setPosition($$, @$);
                        setFileName($$, file_name);};

expr-rec:           if
                    | while
                    | let
                    | binary
                    | unary
                    | call
                    | literal
                    | NEW type-id
                    { $$ = new New($2);}
                    | object-id ASSIGN expr
                    { $$ = new Assign($1, $3);}
                    | object-id
                    { $$ = new Identifier($1);}
                    | LPAR RPAR
                    {$$ = new Unit();}
                    | LPAR expr RPAR
                    {$$ = $2;}
                    | block
                    {$$ = new Block($1->reverse()); delete $1;}
                    | SELF
                    {$$ = new Self();};

if:                 IF expr THEN expr
                    {$$ = new If($2, $4, NULL);}
                    | IF expr THEN expr ELSE expr
                    {$$ = new If($2, $4, $6);};

while:              WHILE expr DO expr
                    {$$ = new While($2, $4);};

let:                LET object-id COLON type init IN expr
                    {$$ = new Let($2, $4, $5, $7);};

init:               /* EPSILON */
                    {$$ = NULL;}
                    | ASSIGN expr
                    {$$ = $2;};

binary:             expr EQUAL expr
                    {$$ = new BinOp(BinOp::EQUAL, $1, $3);}
                    | expr LOWER expr
                    {$$ = new BinOp(BinOp::LOWER, $1, $3);}
                    | expr LOWER_EQUAL expr
                    {$$ = new BinOp(BinOp::LOWER_EQ, $1, $3);}
                    | expr PLUS expr
                    {$$ = new BinOp(BinOp::PLUS, $1, $3);}
                    | expr MINUS expr
                    {$$ = new BinOp(BinOp::MINUS, $1, $3);}
                    | expr TIMES expr
                    {$$ = new BinOp(BinOp::TIMES, $1, $3);}
                    | expr DIV expr
                    {$$ = new BinOp(BinOp::DIV, $1, $3);}
                    | expr POW expr
                    {$$ = new BinOp(BinOp::POW, $1, $3);}
                    | expr AND expr
                    {$$ = new BinOp(BinOp::AND, $1, $3);};

unary:              NOT expr
                    {$$ = new UnOp(UnOp::NOT, $2);}
                    | ISNULL expr
                    {$$ = new UnOp(UnOp::ISNULL, $2);}
                    | MINUS expr %prec UNARY_MINUS
                    {$$ = new UnOp(UnOp::MINUS, $2);};

literal:            INT_LITERAL
                    {$$ = new Integer($1);}
                    | STR_LITERAL
                    {$$ = new String($1);}
                    | TRUE
                    {$$ = new Boolean(true);}
                    | FALSE
                    {$$ = new Boolean(false);};

call:               expr DOT object-id args
                    {$$ = new Call($1, $3, $4->reverse()); delete $4;}
                    | object-id args
                    {$$ = new Call(new Self(), $1, $2->reverse()); delete $2;};

args:               LPAR RPAR
                    {$$ = new VSOPList<Expr>();}
                    | LPAR args-rec
                    {$$ = $2;};

args-rec:           expr RPAR
                    {$$ = new VSOPList<Expr>(); $$->push($1);}
                    | expr COMMA args-rec
                    {$3->push($1); $$ = $3;}
                    | error END
                    {yyerror("syntax error: unmatched ( in arguments");  };





%%

void yyerror(const std::string& text){
        std::cerr << file_name << ":" << yylloc.first_line << ":";
        std::cerr << yylloc.first_column << ": " << text << std::endl;
        exit(-1);
    }


void printResult(const std::string& text){
    std::cout << yylloc.first_line << "," << yylloc.first_column << "," << text << std::endl;
}

void setPosition(Node* node, const YYLTYPE& pos){
    node->line = pos.first_line;
    node->col = pos.first_column;

}

void setFileName(Node* node, std::string& file_name){
    node->file_name = file_name;
}