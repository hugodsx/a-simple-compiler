#ifndef LEXER_H
#define LEXER_H

/*词法分析*/
#include <string>
#include <vector>
#include <cctype>
#include <iostream>

enum class TokenType{
    INT, RETURN, 
//    MAIN,
    VOID,
    PRINTLIN,  // 输出
    IDENT, DIGIT,
    EQUAL,
    PLUS, MINUS, MULTIPLY, DIVIDE, REMAINDER, 
    LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, EQUAL_EQUAL, NOT_EQUAL,
    AND, OR, NOR, NOT,
    AND_AND, OR_OR, // 逻辑与、逻辑或
    SEMICOLON, LPAREN, RPAREN, // 分号、左括号、右括号
    LBRACE, RBRACE, // 花括号
    COMMA, // 逗号
    END, // 文件结束
    IF, ELSE,
    WHILE, CONTINUE, BREAK
};
// 需要识别的单词

struct Token{
    TokenType type;
    std::string lexeme;
};


class Lexer{
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    const std::string source;
    size_t pos = 0;
};

#endif // LEXER_H

