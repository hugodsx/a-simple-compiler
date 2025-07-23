#include "Parser.h"
#include <cstdio>
	
/**
 * @brief ������������
 * @return ���غ�������AST�ڵ�
 * 
 * �﷨����: int IDENT ( [int IDENT (, int IDENT)*] ) { ... }
 */
std::unique_ptr<FunctionDecl> Parser::parseFunction() {
    // ������������
    std::string returnType;
    if (match(TokenType::INT)) {
        returnType = "int";
    } else if (match(TokenType::VOID)) {
        returnType = "void";
    } else {
        throw std::runtime_error("Expect return type (int/void)");
    }
    
    // ����������
    consume(TokenType::IDENT, "Expect function name");
    std::string funcName = previous().lexeme;
    
    // ���������б�
    consume(TokenType::LPAREN, "Expect '(' after function name");
    
    std::vector<std::pair<std::string, std::string>> params;
    if (!check(TokenType::RPAREN)) {
        do {
            // ��������ֻ����int
            consume(TokenType::INT, "Expect parameter type");
            consume(TokenType::IDENT, "Expect parameter name");
            params.emplace_back("int", previous().lexeme);
        } while (match(TokenType::COMMA));
    }
    
    consume(TokenType::RPAREN, "Expect ')' after parameters");
    consume(TokenType::LBRACE, "Expect '{' before function body");
    
    auto body = parseBlock();
    
    return std::make_unique<FunctionDecl>(
        returnType, 
        funcName, 
        std::move(params), 
        std::move(body));
}

/**
 * @brief ���������
 * @return ���ش����AST�ڵ�
 * 
 * �﷨����: { statement* }
 */
std::unique_ptr<Block> Parser::parseBlock() {
    auto block = std::make_unique<Block>();
    
    // ѭ������������䣬ֱ������'}'
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        block->addStatement(parseStatement());
    }
    
    // ������������
    consume(TokenType::RBRACE, "Expect '}' after block");
    return block;
}

/**
 * @brief �������
 * @return �������AST�ڵ�
 * 
 * ֧�ֵ��������:
 * 1. ��������: int IDENT;
 * 2. return���: return expr;
 * 3. ��ֵ���: IDENT = expr;
 * 4. ���ʽ���: expr;
 */
std::unique_ptr<Statement> Parser::parseStatement() {
	//std::cout << "here" << peek().lexeme << std::endl;
	//printf("here2 %d\n", peek().type);
    if (match(TokenType::INT)) {
        return parseVariableDecl();
    } else if (match(TokenType::RETURN)) {
        return parseReturn();
    } else if (match(TokenType::IF)) {
        return parseIfStatement();
    } else if (match(TokenType::WHILE)) {
        return parseWhileStatement();
    } else if (match(TokenType::BREAK)) {
        return parseBreakStatement();
    } else if (match(TokenType::CONTINUE)) {
        return parseContinueStatement();
    } else if (check(TokenType::IDENT) && checkNext(TokenType::EQUAL)) {
        return parseAssignment();
    } else if (check(TokenType::PRINTLIN)) {
        return parsePrintlnInt();
    } else {

        auto expr = parseExpression();
        consume(TokenType::SEMICOLON, "Expect ';' after expression");
        return std::make_unique<ExpressionStatement>(std::move(expr));
    }
}


std::unique_ptr<Statement> Parser::parseIfStatement() {
    consume(TokenType::LPAREN, "Expect '(' after 'if'");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expect ')' after if condition");
    consume(TokenType::LBRACE, "Expect '{' before if block");
    auto thenBlock = parseBlock();


    std::unique_ptr<Block> elseBlock = nullptr;
    if (match(TokenType::ELSE)) {
        if (check(TokenType::IF)) {
            // ����else if���
	    consume(TokenType::LBRACE, "Expect '{' before if block");
            auto elseIfStmt = parseIfStatement();
            elseBlock = std::make_unique<Block>();
            elseBlock->addStatement(std::move(elseIfStmt));
        } else {
	    consume(TokenType::LBRACE, "Expect '{' before if block");
            elseBlock = parseBlock();
        }
    }
    return std::make_unique<ConditionStatement>(
        std::move(condition), 
        std::move(thenBlock), 
        std::move(elseBlock));
}

std::unique_ptr<Statement> Parser::parseWhileStatement() {
    consume(TokenType::LPAREN, "Expect '(' after 'while'");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expect ')' after while condition");
    consume(TokenType::LBRACE, "Expect '{' before while block");
    auto body = parseBlock();

    
    return std::make_unique<LoopStatement>(
        std::move(condition),
        std::move(body));
}

std::unique_ptr<Statement> Parser::parseBreakStatement() {
    consume(TokenType::SEMICOLON, "Expect ';' after 'break'");
    return std::make_unique<BreakStmt>();
}

std::unique_ptr<Statement> Parser::parseContinueStatement() {
    consume(TokenType::SEMICOLON, "Expect ';' after 'continue'");
    return std::make_unique<ContinueStmt>();
}



/**
 * @brief ������������
 * @return ���ر�������AST�ڵ�
 * 
 * �﷨����: int IDENT;
 */
std::unique_ptr<Statement> Parser::parseVariableDecl() {
    // ʹ��Block����϶����������
    auto block = std::make_unique<Block>();
    
    // ������һ������
    do {
        consume(TokenType::IDENT, "Expect variable name");
        std::string varName = previous().lexeme;
        
        // ����Ƿ��г�ʼ����ֵ
        std::unique_ptr<Expression> initExpr = nullptr;
        if (match(TokenType::EQUAL)) {
            initExpr = parseExpression();
        }
        
        block->addStatement(
            std::make_unique<VariableDecl>("int", 
                std::make_unique<Variable>(varName), 
                std::move(initExpr))
        );
        
    } while (match(TokenType::COMMA)); // �����������ŷָ��ı���
    
    // �����������ֺ�
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration");
    
    // ���ֻ��һ������������ֱ�ӷ�����������Block
    if (block->statements.size() == 1) {
        return std::move(block->statements[0]);
    }
    
    return block;
}

/**
 * @brief ����return���
 * @return ����return���AST�ڵ�
 * 
 * �﷨����: return expr;
 */
std::unique_ptr<Statement> Parser::parseReturn() {
    // ��������ֵ���ʽ
    auto expr = parseExpression();
    // �����������ֺ�
    consume(TokenType::SEMICOLON, "Expect ';' after return");
    return std::make_unique<ReturnStmt>(std::move(expr));
}

/**
 * @brief ������ֵ���
 * @return ���ظ�ֵ���AST�ڵ�
 * 
 * �﷨����: IDENT = expr;
 */
std::unique_ptr<Statement> Parser::parseAssignment() {
    // ��������ʶ��
    auto id = std::make_unique<Variable>(peek().lexeme);
    advance(); // ���ı�ʶ��
    advance(); // ���ĵȺ�
    
    // �����Ҳ���ʽ
    auto expr = parseExpression();
    // �����������ֺ�
    consume(TokenType::SEMICOLON, "Expect ';' after assignment");
    return std::make_unique<Assignment>(std::move(id), std::move(expr));
}

// ����println_int���
std::unique_ptr<Statement> Parser::parsePrintlnInt() {
    
    advance();
    consume(TokenType::LPAREN, "Expect '(' after 'println_int'");
    
    // �����������ʽ
    auto arg = parseExpression();
    
    consume(TokenType::RPAREN, "Expect ')' after println_int argument");
    consume(TokenType::SEMICOLON, "Expect ';' after println_int statement");
    
    return std::make_unique<PrintlnIntStmt>(std::move(arg));
}

/**
 * @brief �������ʽ
 * @return ���ر��ʽAST�ڵ�
 */
std::unique_ptr<Expression> Parser::parseExpression() {
    return parseBinaryOp(0);
}

/**
 * @brief ������Ԫ��������ʽ
 * @param minPrec ��С���ȼ������ڴ�����������ȼ�
 * @return ���ض�Ԫ������ʽAST�ڵ�
 */
std::unique_ptr<Expression> Parser::parseBinaryOp(int minPrec) {
    // ���������ʽ
    auto left = parsePrimary();

    // ѭ��������ܴ��ڵĶ����Ԫ�����
    while (true) {
        auto op = peek();

        int prec = getPrecedence(op.type);
        // �����ǰ��������ȼ�������С���ȼ�����ֹͣ
        if (prec < minPrec || !isBinaryOp(op.type)) break;
        
        advance();
        // �ݹ�����Ҳ���ʽ����������������
        auto right = parseBinaryOp(prec + 1);
        left = std::make_unique<BinaryOp>(std::move(left), std::move(right), op.lexeme);
    }

    return left;
}

/**
 * @brief �����������ʽ
 * @return ���ػ������ʽAST�ڵ�
 * 
 * ֧�ֵĻ������ʽ:
 * 1. ����������
 * 2. ��ʶ��
 * 3. ��������
 * 4. ���ű��ʽ
 */
std::unique_ptr<Expression> Parser::parsePrimary() {

	//std::cout << "here" << peek().lexeme << std::endl;
	//printf("here2 %d\n", peek().type);
    if (match(TokenType::DIGIT)) {

        // ��������������
        return std::make_unique<IntegerLiteral>(std::stoi(previous().lexeme));
    } else if (match(TokenType::IDENT)) {
        // ����Ƿ��Ǻ�������
        if (check(TokenType::LPAREN)) {

            return parseFunctionCall(previous().lexeme);
        }
        // ��ͨ��ʶ��

        return std::make_unique<Variable>(previous().lexeme);
    } else if (match(TokenType::LPAREN)) {
        // ���ű��ʽ

        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expect ')' after expression");
        return expr;
    } else {
        throw std::runtime_error("Expect expression");
    }
}

/**
 * @brief ������������
 * @param name ������
 * @return ���غ�������AST�ڵ�
 * 
 * �﷨����: IDENT ( [expr (, expr)*] )
 */
std::unique_ptr<Expression> Parser::parseFunctionCall(const std::string& name) {
    consume(TokenType::LPAREN, "Expect '(' after function name");
    
    std::vector<std::unique_ptr<Expression>> args;
    // ���������б�
    if (!check(TokenType::RPAREN)) {
        do {
            args.push_back(parseExpression());
        } while (match(TokenType::COMMA)); // ֧�ֶ���������ö��ŷָ�
    }
    
    consume(TokenType::RPAREN, "Expect ')' after arguments");
    return std::make_unique<FunctionCall>(name, std::move(args));
}

/**
 * @brief ��ȡ��������ȼ�
 * @param type �����Token����
 * @return ������������ȼ���ֵ
 */
int Parser::getPrecedence(TokenType type) {
    switch (type) {
        case TokenType::MULTIPLY: case TokenType::DIVIDE: case TokenType::REMAINDER:
            return 10; // ������ȼ�
        case TokenType::PLUS: case TokenType::MINUS:
            return 9;
        case TokenType::LESS: case TokenType::LESS_EQUAL: 
        case TokenType::GREATER: case TokenType::GREATER_EQUAL:
            return 8;
        case TokenType::EQUAL_EQUAL: case TokenType::NOT_EQUAL:
            return 7;
        case TokenType::AND:
            return 6;
        case TokenType::NOR: 
            return 5;
        case TokenType::OR:
            return 4;
        case TokenType::AND_AND: 
            return 3; // �߼���
        case TokenType::OR_OR:
            return 2; // �߼���
        default:
            return 0; // �������
    }
}

/**
 * @brief ����Ƿ�Ϊ��Ԫ�����
 * @param type Token����
 * @return ����Ƕ�Ԫ���������true�����򷵻�false
 */
bool Parser::isBinaryOp(TokenType type) {
    return type == TokenType::PLUS || type == TokenType::MINUS ||
            type == TokenType::MULTIPLY || type == TokenType::DIVIDE ||
            type == TokenType::REMAINDER || type == TokenType::LESS ||
            type == TokenType::LESS_EQUAL || type == TokenType::GREATER ||
            type == TokenType::GREATER_EQUAL || type == TokenType::EQUAL_EQUAL ||
            type == TokenType::NOT_EQUAL || type == TokenType::AND ||
            type == TokenType::OR || type == TokenType::NOR ||
            type == TokenType::AND_AND || type == TokenType::OR_OR;
}

/**
 * @brief ����ָ�����͵�Token
 * @param type ������Token����
 * @param msg ������Ͳ�ƥ��ʱ�Ĵ�����Ϣ
 * @throws std::runtime_error �����ǰToken��������������
 */
void Parser::consume(TokenType type, const std::string& msg) {
    if (check(type)) {
        advance();
        return;
    }
    throw std::runtime_error(msg);
}

/**
 * @brief �����һ��Token������
 * @param type Ҫ����Token����
 * @return �����һ��Token��ָ�����ͷ���true�����򷵻�false
 */
bool Parser::checkNext(TokenType type) {
    if (current + 1 >= tokens.size()) return false;
    return tokens[current + 1].type == type;
}


