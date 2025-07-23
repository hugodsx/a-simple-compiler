#include "Lexer.h"

// ��ʼ�����������ַ�����ʽ����Դ����
Lexer::Lexer(const std::string& source): source(source){}

// �ʷ���������������һ��Token�б�
std::vector<Token> Lexer::tokenize(){
    std::vector<Token> tokens;
    while(pos < source.size()){
        char current = source[pos];
        if(isspace(current)){ 
            // �����հ��ַ�
            pos++;
        }

        // ����̶��ؼ���

        else if(current == 'i' && source.substr(pos, 3) == "int"){
            // ����int
            tokens.push_back({TokenType::INT, "int"});
            pos += 3;
        }
        else if(current == 'p' && source.substr(pos, 11) == "println_int"){
            // ����println_int
            tokens.push_back({TokenType::PRINTLIN, "println_int"});
            pos += 11;
        }
        else if(current == 'r' && source.substr(pos, 6) == "return"){
            // ����return
            tokens.push_back({TokenType::RETURN, "return"});
            pos += 6;
        }
        // else if(current == 'm' && source.substr(pos, 4) == "main"){
        //     // ����main
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

        // ���������

        else if(isdigit(current)){
            // ��������
            size_t end = pos + 1;
            while(end < source.size() && isdigit(source[end])){
                end++;
            }
            tokens.push_back({TokenType::DIGIT, source.substr(pos, end - pos)});
            pos = end;
        }
        else if(isalpha(current) || current == '_'){
            // �����ʶ��
            size_t end = pos + 1;
            while(end < source.size() && (isalnum(source[end]) || source[end] == '_')){
                end++;
            }
            tokens.push_back({TokenType::IDENT, source.substr(pos, end - pos)});
            pos = end;
        }


        // ���������

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

        // ����
        
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
    tokens.push_back({TokenType::END, ""}); // ����ļ��������
    return tokens;
}

/*
�������ɵ�token����
int a = 10;
printf("%d", a);
[
  {INT, "int"}, {IDENT, "a"}, {EQUAL, "="}, {NUMBER, "10"}, {SEMICOLON, ";"},
  {PRINTF, "printf"}, {LPAREN, "("}, {STRING, "\"%d\""}, {COMMA, ","}, 
  {IDENT, "a"}, {RPAREN, ")"}, {SEMICOLON, ";"}
]
*/



