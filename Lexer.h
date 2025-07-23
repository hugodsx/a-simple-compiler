#ifndef LEXER_H
#define LEXER_H

/*�ʷ�����*/
#include <string>
#include <vector>
#include <cctype>
#include <iostream>

enum class TokenType{
    INT, RETURN, 
//    MAIN,
    VOID,
    PRINTLIN,  // ���
    IDENT, DIGIT,
    EQUAL,
    PLUS, MINUS, MULTIPLY, DIVIDE, REMAINDER, 
    LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, EQUAL_EQUAL, NOT_EQUAL,
    AND, OR, NOR, NOT,
    AND_AND, OR_OR, // �߼��롢�߼���
    SEMICOLON, LPAREN, RPAREN, // �ֺš������š�������
    LBRACE, RBRACE, // ������
    COMMA, // ����
    END, // �ļ�����
    IF, ELSE,
    WHILE, CONTINUE, BREAK
};
// ��Ҫʶ��ĵ���

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

