#include "Parser.h"
#include <cstdio>
	
/**
 * @brief 解析函数声明
 * @return 返回函数声明AST节点
 * 
 * 语法规则: int IDENT ( [int IDENT (, int IDENT)*] ) { ... }
 */
std::unique_ptr<FunctionDecl> Parser::parseFunction() {
    // 解析返回类型
    std::string returnType;
    if (match(TokenType::INT)) {
        returnType = "int";
    } else if (match(TokenType::VOID)) {
        returnType = "void";
    } else {
        throw std::runtime_error("Expect return type (int/void)");
    }
    
    // 解析函数名
    consume(TokenType::IDENT, "Expect function name");
    std::string funcName = previous().lexeme;
    
    // 解析参数列表
    consume(TokenType::LPAREN, "Expect '(' after function name");
    
    std::vector<std::pair<std::string, std::string>> params;
    if (!check(TokenType::RPAREN)) {
        do {
            // 参数类型只能是int
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
 * @brief 解析代码块
 * @return 返回代码块AST节点
 * 
 * 语法规则: { statement* }
 */
std::unique_ptr<Block> Parser::parseBlock() {
    auto block = std::make_unique<Block>();
    
    // 循环解析所有语句，直到遇到'}'
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        block->addStatement(parseStatement());
    }
    
    // 解析代码块结束
    consume(TokenType::RBRACE, "Expect '}' after block");
    return block;
}

/**
 * @brief 解析语句
 * @return 返回语句AST节点
 * 
 * 支持的语句类型:
 * 1. 变量声明: int IDENT;
 * 2. return语句: return expr;
 * 3. 赋值语句: IDENT = expr;
 * 4. 表达式语句: expr;
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
            // 处理else if情况
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
 * @brief 解析变量声明
 * @return 返回变量声明AST节点
 * 
 * 语法规则: int IDENT;
 */
std::unique_ptr<Statement> Parser::parseVariableDecl() {
    // 使用Block来组合多个变量声明
    auto block = std::make_unique<Block>();
    
    // 解析第一个变量
    do {
        consume(TokenType::IDENT, "Expect variable name");
        std::string varName = previous().lexeme;
        
        // 检查是否有初始化赋值
        std::unique_ptr<Expression> initExpr = nullptr;
        if (match(TokenType::EQUAL)) {
            initExpr = parseExpression();
        }
        
        block->addStatement(
            std::make_unique<VariableDecl>("int", 
                std::make_unique<Variable>(varName), 
                std::move(initExpr))
        );
        
    } while (match(TokenType::COMMA)); // 继续解析逗号分隔的变量
    
    // 解析语句结束分号
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration");
    
    // 如果只有一个变量声明，直接返回它而不是Block
    if (block->statements.size() == 1) {
        return std::move(block->statements[0]);
    }
    
    return block;
}

/**
 * @brief 解析return语句
 * @return 返回return语句AST节点
 * 
 * 语法规则: return expr;
 */
std::unique_ptr<Statement> Parser::parseReturn() {
    // 解析返回值表达式
    auto expr = parseExpression();
    // 解析语句结束分号
    consume(TokenType::SEMICOLON, "Expect ';' after return");
    return std::make_unique<ReturnStmt>(std::move(expr));
}

/**
 * @brief 解析赋值语句
 * @return 返回赋值语句AST节点
 * 
 * 语法规则: IDENT = expr;
 */
std::unique_ptr<Statement> Parser::parseAssignment() {
    // 解析左侧标识符
    auto id = std::make_unique<Variable>(peek().lexeme);
    advance(); // 消耗标识符
    advance(); // 消耗等号
    
    // 解析右侧表达式
    auto expr = parseExpression();
    // 解析语句结束分号
    consume(TokenType::SEMICOLON, "Expect ';' after assignment");
    return std::make_unique<Assignment>(std::move(id), std::move(expr));
}

// 解析println_int语句
std::unique_ptr<Statement> Parser::parsePrintlnInt() {
    
    advance();
    consume(TokenType::LPAREN, "Expect '(' after 'println_int'");
    
    // 解析参数表达式
    auto arg = parseExpression();
    
    consume(TokenType::RPAREN, "Expect ')' after println_int argument");
    consume(TokenType::SEMICOLON, "Expect ';' after println_int statement");
    
    return std::make_unique<PrintlnIntStmt>(std::move(arg));
}

/**
 * @brief 解析表达式
 * @return 返回表达式AST节点
 */
std::unique_ptr<Expression> Parser::parseExpression() {
    return parseBinaryOp(0);
}

/**
 * @brief 解析二元运算符表达式
 * @param minPrec 最小优先级，用于处理运算符优先级
 * @return 返回二元运算表达式AST节点
 */
std::unique_ptr<Expression> Parser::parseBinaryOp(int minPrec) {
    // 解析左侧表达式
    auto left = parsePrimary();

    // 循环处理可能存在的多个二元运算符
    while (true) {
        auto op = peek();

        int prec = getPrecedence(op.type);
        // 如果当前运算符优先级低于最小优先级，则停止
        if (prec < minPrec || !isBinaryOp(op.type)) break;
        
        advance();
        // 递归解析右侧表达式，处理运算符结合性
        auto right = parseBinaryOp(prec + 1);
        left = std::make_unique<BinaryOp>(std::move(left), std::move(right), op.lexeme);
    }

    return left;
}

/**
 * @brief 解析基本表达式
 * @return 返回基本表达式AST节点
 * 
 * 支持的基本表达式:
 * 1. 整数字面量
 * 2. 标识符
 * 3. 函数调用
 * 4. 括号表达式
 */
std::unique_ptr<Expression> Parser::parsePrimary() {

	//std::cout << "here" << peek().lexeme << std::endl;
	//printf("here2 %d\n", peek().type);
    if (match(TokenType::DIGIT)) {

        // 解析整数字面量
        return std::make_unique<IntegerLiteral>(std::stoi(previous().lexeme));
    } else if (match(TokenType::IDENT)) {
        // 检查是否是函数调用
        if (check(TokenType::LPAREN)) {

            return parseFunctionCall(previous().lexeme);
        }
        // 普通标识符

        return std::make_unique<Variable>(previous().lexeme);
    } else if (match(TokenType::LPAREN)) {
        // 括号表达式

        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expect ')' after expression");
        return expr;
    } else {
        throw std::runtime_error("Expect expression");
    }
}

/**
 * @brief 解析函数调用
 * @param name 函数名
 * @return 返回函数调用AST节点
 * 
 * 语法规则: IDENT ( [expr (, expr)*] )
 */
std::unique_ptr<Expression> Parser::parseFunctionCall(const std::string& name) {
    consume(TokenType::LPAREN, "Expect '(' after function name");
    
    std::vector<std::unique_ptr<Expression>> args;
    // 解析参数列表
    if (!check(TokenType::RPAREN)) {
        do {
            args.push_back(parseExpression());
        } while (match(TokenType::COMMA)); // 支持多个参数，用逗号分隔
    }
    
    consume(TokenType::RPAREN, "Expect ')' after arguments");
    return std::make_unique<FunctionCall>(name, std::move(args));
}

/**
 * @brief 获取运算符优先级
 * @param type 运算符Token类型
 * @return 返回运算符优先级数值
 */
int Parser::getPrecedence(TokenType type) {
    switch (type) {
        case TokenType::MULTIPLY: case TokenType::DIVIDE: case TokenType::REMAINDER:
            return 10; // 最高优先级
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
            return 3; // 逻辑与
        case TokenType::OR_OR:
            return 2; // 逻辑或
        default:
            return 0; // 非运算符
    }
}

/**
 * @brief 检查是否为二元运算符
 * @param type Token类型
 * @return 如果是二元运算符返回true，否则返回false
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
 * @brief 消耗指定类型的Token
 * @param type 期望的Token类型
 * @param msg 如果类型不匹配时的错误信息
 * @throws std::runtime_error 如果当前Token不是期望的类型
 */
void Parser::consume(TokenType type, const std::string& msg) {
    if (check(type)) {
        advance();
        return;
    }
    throw std::runtime_error(msg);
}

/**
 * @brief 检查下一个Token的类型
 * @param type 要检查的Token类型
 * @return 如果下一个Token是指定类型返回true，否则返回false
 */
bool Parser::checkNext(TokenType type) {
    if (current + 1 >= tokens.size()) return false;
    return tokens[current + 1].type == type;
}


