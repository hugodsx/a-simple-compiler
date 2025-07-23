#include "Lexer.h"

// 初始化函数，以字符串形式读入源代码
Lexer::Lexer(const std::string& source): source(source){}

// 词法分析函数，返回一个Token列表
std::vector<Token> Lexer::tokenize(){
    std::vector<Token> tokens;
    while(pos < source.size()){
        char current = source[pos];
        if(isspace(current)){ 
            // 跳过空白字符
            pos++;
        }

        // 处理固定关键字

        else if(current == 'i' && source.substr(pos, 3) == "int"){
            // 处理int
            tokens.push_back({TokenType::INT, "int"});
            pos += 3;
        }
        else if(current == 'p' && source.substr(pos, 11) == "println_int"){
            // 处理println_int
            tokens.push_back({TokenType::PRINTLIN, "println_int"});
            pos += 11;
        }
        else if(current == 'r' && source.substr(pos, 6) == "return"){
            // 处理return
            tokens.push_back({TokenType::RETURN, "return"});
            pos += 6;
        }
        // else if(current == 'm' && source.substr(pos, 4) == "main"){
        //     // 处理main
        //     tokens.push_back({TokenType::MAIN, "main"});
        //     pos += 4;
        // }
        else if(current == 'v' && source.substr(pos, 4) == "void"){
            // void
            tokens.push_back({TokenType::VOID, "void"});
            pos += 4;
        }
                else if(current == 'i' && source.substr(pos, 2) == "if"){
            // if
            tokens.push_back({TokenType::IF, "if"});
            pos += 2;
        }
        else if(current == 'e' && source.substr(pos, 4) == "else"){
            tokens.push_back({TokenType::ELSE, "else"});
            pos += 4;
        }
        else if(current == 'w' && source.substr(pos, 5) == "while"){
            // while
            tokens.push_back({TokenType::WHILE, "while"});
            pos += 5;
        }
        else if(current == 'c' && source.substr(pos, 8) == "continue"){
            // continue
            tokens.push_back({TokenType::CONTINUE, "continue"});
            pos += 8;
        }
        else if(current == 'b' && source.substr(pos, 5) == "break"){
            // break
            tokens.push_back({TokenType::BREAK, "break"});
            pos += 5;
        }

        // 处理变量名

        else if(isdigit(current)){
            // 处理数字
            size_t end = pos + 1;
            while(end < source.size() && isdigit(source[end])){
                end++;
            }
            tokens.push_back({TokenType::DIGIT, source.substr(pos, end - pos)});
            pos = end;
        }
        else if(isalpha(current) || current == '_'){
            // 处理标识符
            size_t end = pos + 1;
            while(end < source.size() && (isalnum(source[end]) || source[end] == '_')){
                end++;
            }
            tokens.push_back({TokenType::IDENT, source.substr(pos, end - pos)});
            pos = end;
        }


        // 处理运算符

        else if(current == '='){
            if(source[pos + 1] == '='){
                tokens.push_back({TokenType::EQUAL_EQUAL, "=="});
                pos += 2;
            }
            else{
                tokens.push_back({TokenType::EQUAL, "="});
                pos++;
            }
        }
        else if(current == '+'){
            tokens.push_back({TokenType::PLUS, "+"});
            pos++;
        }
        else if(current == '-'){
            tokens.push_back({TokenType::MINUS, "-"});
            pos++;
        }
        else if(current == '*'){
            tokens.push_back({TokenType::MULTIPLY, "*"});
            pos++;
        }
        else if(current == '/'){
            tokens.push_back({TokenType::DIVIDE, "/"});
            pos++;
        }
        else if(current == '%'){
            tokens.push_back({TokenType::REMAINDER, "%"});
            pos++;
        }

        else if(current == '<'){
            if(source[pos + 1] == '='){
                tokens.push_back({TokenType::LESS_EQUAL, "<="});
                pos += 2;
            }
            else{
                tokens.push_back({TokenType::LESS, "<"});
                pos++;
            }
        }
        else if(current == '>'){
            if(source[pos + 1] == '='){
                tokens.push_back({TokenType::GREATER_EQUAL, ">="});
                pos += 2;
            }
            else{
                tokens.push_back({TokenType::GREATER, ">"});
                pos++;
            }
        }

        // 与或非
        
        else if(current == '&'){
            if(source[pos + 1] == '&'){
                tokens.push_back({TokenType::AND_AND, "&&"});
                pos += 2;
            }
            else{
                tokens.push_back({TokenType::AND, "&"});
                pos++;
            }
        }
        else if(current == '|'){
            if(source[pos + 1] == '|'){
                tokens.push_back({TokenType::OR_OR, "||"});
                pos += 2;
            }
            else{
                tokens.push_back({TokenType::OR, "|"});
                pos++;
            }
        }
        else if(current == '!'){
            if(source[pos + 1] == '='){
                tokens.push_back({TokenType::NOT_EQUAL, "!="});
                pos += 2;
            }
            else{
                tokens.push_back({TokenType::NOT, "!"});
                pos++;
            }
        }

        else if (current == '^'){
            tokens.push_back({TokenType::NOR, "^"});
            pos++;
        }

        else if(current == '('){
            tokens.push_back({TokenType::LPAREN, "("});
            pos++;
        }
        else if(current == ')'){
            tokens.push_back({TokenType::RPAREN, ")"});
            pos++;
        }
        else if(current == ';'){
            tokens.push_back({TokenType::SEMICOLON, ";"});
            pos++;
        }
        else if(current == '{'){
            tokens.push_back({TokenType::LBRACE, "{"});
            pos++;
        }
        else if(current == '}'){
            tokens.push_back({TokenType::RBRACE, "}"});
            pos++;
        }
        else if(current == ','){
            tokens.push_back({TokenType::COMMA, ","});
            pos++;
        }
        
    }
    tokens.push_back({TokenType::END, ""}); // 添加文件结束标记
    return tokens;
}

/*
期望生成的token流：
int a = 10;
printf("%d", a);
[
  {INT, "int"}, {IDENT, "a"}, {EQUAL, "="}, {NUMBER, "10"}, {SEMICOLON, ";"},
  {PRINTF, "printf"}, {LPAREN, "("}, {STRING, "\"%d\""}, {COMMA, ","}, 
  {IDENT, "a"}, {RPAREN, ")"}, {SEMICOLON, ";"}
]
*/



