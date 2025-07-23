#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include <vector>
#include <string>
#include <memory>
#include <utility>

class IntegerLiteral;
class Variable;
class BinaryOp;
class Assignment;
class VariableDecl;
class ReturnStmt;
class FunctionCall;
class Block;
class FunctionDecl;
class Program;
class PrintlnIntStmt;
class ExpressionStatement;
class LoopStatement;
class ConditionStatement;
class BreakStmt;
class ContinueStmt;

// ������ģʽ����
class Visitor {
    public:
        virtual ~Visitor() = default;
        virtual void visit(IntegerLiteral&) = 0;
        virtual void visit(Variable&) = 0;
        virtual void visit(BinaryOp&) = 0;
        virtual void visit(Assignment&) = 0;
        virtual void visit(VariableDecl&) = 0;
        virtual void visit(ReturnStmt&) = 0;
        virtual void visit(FunctionCall&) = 0;
        virtual void visit(Block&) = 0;
        virtual void visit(FunctionDecl&) = 0;
        virtual void visit(Program&) = 0;
        virtual void visit(PrintlnIntStmt&) = 0;
	    virtual void visit(ExpressionStatement&) = 0;
        virtual void visit(LoopStatement&) = 0;
        virtual void visit(ConditionStatement&) = 0;
        virtual void visit(BreakStmt&) = 0;
        virtual void visit(ContinueStmt&) = 0;
    };

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(class Visitor& v) = 0;
};

class Expression : public ASTNode {};   // ���ʽ�ڵ�
class Statement : public ASTNode {};    // ���ڵ�

class IntegerLiteral : public Expression {
public:
    int value; // ����ֵ
    IntegerLiteral(int value) : value(value) {}
    void accept(Visitor& v) override {v.visit(*this);};
};  // �����������ڵ�

class Variable : public Expression {
public: 
    std::string name; // ������
    Variable(const std::string& name) : name(name) {}
    void accept(Visitor& v) override {v.visit(*this);};
};  // �����ڵ�

class BinaryOp : public Expression {
public:
    std::unique_ptr<Expression> left; // �������
    std::unique_ptr<Expression> right; // �Ҳ�����
    std::string op; // ����������
    BinaryOp(
        std::unique_ptr<Expression> left, 
        std::unique_ptr<Expression> right, 
        std::string op):
            left(std::move(left)), 
            right(std::move(right)), 
            op(op) {}
    void accept(Visitor& v) override {v.visit(*this);};
}; // ��Ԫ������ڵ�

class Assignment : public Statement {
public:
    std::unique_ptr<Variable> varName; // ������
    std::unique_ptr<Expression> value; // ��ֵ��ֵ
    Assignment(
        std::unique_ptr<Variable> varName, 
        std::unique_ptr<Expression> value) : 
        varName(std::move(varName)), value(std::move(value)) {}
    void accept(Visitor& v) override {v.visit(*this);};
}; // ��ֵ���ڵ�

class VariableDecl : public Statement {
public:
    std::string type;
    std::unique_ptr<Variable> varName;  // ��Ϊʹ��Variable�ڵ�
    std::unique_ptr<Expression> value;  // ��ʼ�����ʽ
    
    VariableDecl(
        const std::string& type,
        std::unique_ptr<Variable> varName,
        std::unique_ptr<Expression> value) 
        : type(type), varName(std::move(varName)), value(std::move(value)) {}
    
    void accept(Visitor& v) override { v.visit(*this); }
};

class ReturnStmt : public Statement {
public:
    std::unique_ptr<Expression> value; // ����ֵ
    ReturnStmt(std::unique_ptr<Expression> value) : value(std::move(value)) {}
    void accept(Visitor& v) override {v.visit(*this);};
};

class FunctionCall : public Expression {
public:
    std::string functionName; // ������
    std::vector<std::unique_ptr<Expression>> args; // ���������б�
    FunctionCall(
        const std::string& functionName, 
        std::vector<std::unique_ptr<Expression>> args) : 
        functionName(functionName), args(std::move(args)) {}
    void accept(Visitor& v) override {v.visit(*this);};
};

class Block : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> statements; // ����б�
    Block() : statements() {}
    
    Block(std::vector<std::unique_ptr<Statement>> statements) : 
        statements(std::move(statements)) {}

    void addStatement(std::unique_ptr<Statement> stmt) {
        statements.push_back(std::move(stmt));
    }
    void accept(Visitor& v) override {v.visit(*this);};
};  // �����ڵ�

class PrintlnIntStmt : public Statement {
public:
    std::unique_ptr<Expression> arg;  // Ҫ��ӡ�ı��ʽ
    
    PrintlnIntStmt(std::unique_ptr<Expression> arg) : arg(std::move(arg)) {}
    
    void accept(Visitor& v) override { v.visit(*this); }
};

class ExpressionStatement : public Statement {
public:
    std::unique_ptr<Expression> expr;
    
    ExpressionStatement(std::unique_ptr<Expression> expr) 
        : expr(std::move(expr)) {}
    
    void accept(Visitor& v) override { v.visit(*this); }
};

class ConditionStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> thenBlock;
    std::unique_ptr<Block> elseBlock;
    
    ConditionStatement(
        std::unique_ptr<Expression> cond,
        std::unique_ptr<Block> thenBlk,
        std::unique_ptr<Block> elseBlk)
        : condition(std::move(cond)),
          thenBlock(std::move(thenBlk)),
          elseBlock(std::move(elseBlk)) {}
    
    void accept(Visitor& v) override { v.visit(*this); }
};

class LoopStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> body;
    
    LoopStatement(
        std::unique_ptr<Expression> cond,
        std::unique_ptr<Block> b)
        : condition(std::move(cond)),
          body(std::move(b)) {}
    
    void accept(Visitor& v) override { v.visit(*this); }
};

class BreakStmt : public Statement {
public:
    void accept(Visitor& v) override { v.visit(*this); }
};

class ContinueStmt : public Statement {
public:
    void accept(Visitor& v) override { v.visit(*this); }
};


class FunctionDecl : public ASTNode {
public:
    std::string returnType;                                  // ����ֵ����
    std::string name;                                        // ������
    std::vector<std::pair<std::string, std::string>> params; // �����б�
    // pair: <type, name>
    std::unique_ptr<Block> body;     

    FunctionDecl(
        const std::string& returnType,
        const std::string& name, 
        std::vector<std::pair<std::string, std::string>> params, 
        std::unique_ptr<Block> body) : 
            returnType(returnType),
            name(name), 
            params(std::move(params)), 
            body(std::move(body)) {}
    void accept(Visitor& v) override {v.visit(*this);};
};


class Program : public ASTNode {
public:
    std::vector<std::unique_ptr<FunctionDecl>> functions;
    
    void accept(Visitor& v) override {
        for (auto& func : functions) {
            func->accept(v);
        }
    }
};




// �﷨��������
class Parser {
public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

    std::unique_ptr<Program> parse(){
        auto program = std::make_unique<Program>();
        
        // ѭ���������к���ֱ���ļ�����
        while (!isAtEnd()) {
            program->functions.push_back(parseFunction());
        }
        
        return program;
    }

private:
    const std::vector<Token>& tokens; // �ʷ����������ɵ�token�б�
    size_t current = 0; // ��ǰtoken����
    const Token& peek() const { return tokens[current]; } // �鿴��ǰtoken
    const Token& previous() const { return tokens[current - 1]; } // �鿴��һ��token
    bool isAtEnd() const { return peek().type == TokenType::END; } // �Ƿ񵽴��ļ�ĩβ
    bool check(TokenType type) const { // ��鵱ǰtoken����
//	std::cout << peek().lexeme << std::endl;
        if(isAtEnd()) return false;
        return peek().type == type;
    }
    const Token& advance() { // ��ǰ�ƶ�һ��token
        if(!isAtEnd()) current++;
        return previous();
    }
    bool match(TokenType type) { // ƥ�䵱ǰtoken����
        if(check(type)) {
            advance();
            return true;
        }
        return false;
    }

    std::unique_ptr<FunctionDecl> parseFunction();                          // ������������
    std::unique_ptr<Block> parseBlock();                                    // ���������
    std::unique_ptr<Statement> parseStatement();                            // �������
    std::unique_ptr<Statement> parseVariableDecl();                         // ������������ 
    std::unique_ptr<Statement> parseReturn();                               // ����������� 
    std::unique_ptr<Statement> parseAssignment();                           // ������ֵ���
    std::unique_ptr<Expression> parseExpression();                          // �������ʽ
    std::unique_ptr<Expression> parseBinaryOp(int minPrec);                 // ������Ԫ�����
    std::unique_ptr<Expression> parsePrimary();                             // �����������ʽ
    std::unique_ptr<Expression> parseFunctionCall(const std::string& name); // ������������
    std::unique_ptr<Statement> parsePrintlnInt();                      // ����println_int�������� 
    std::unique_ptr<Statement> parseIfStatement();
    std::unique_ptr<Statement> parseWhileStatement();
    std::unique_ptr<Statement> parseBreakStatement();
    std::unique_ptr<Statement> parseContinueStatement();


    int getPrecedence(TokenType type);                                      // ��ȡ��������ȼ�
    bool isBinaryOp(TokenType type);
    void consume(TokenType type, const std::string& message);               // ʹ�õ�ǰtoken
    bool checkNext(TokenType type);
};  


class ASTPrinter : public Visitor {
public:
    void visit(IntegerLiteral& node) override {
        std::cout << node.value;
    }
    
    void visit(Variable& node) override {
        std::cout << node.name;
    }
    
    void visit(BinaryOp& node) override {
        std::cout << "(";
        node.left->accept(*this);
        std::cout << " " << node.op << " ";
        node.right->accept(*this);
        std::cout << ")";
    }
    
    void visit(Assignment& node) override {
        node.varName->accept(*this);
        std::cout << " = ";
        node.value->accept(*this);
    }
    
    void visit(VariableDecl& node) {
        std::cout << node.type << " ";
        node.varName->accept(*this);
        if (node.value) {
            std::cout << " = ";
            node.value->accept(*this);
        }
    }
    
    void visit(ReturnStmt& node) override {
        std::cout << "return ";
        if (node.value) node.value->accept(*this);
    }
    
    void visit(FunctionCall& node) override {
        std::cout << node.functionName << "(";
        for (size_t i = 0; i < node.args.size(); ++i) {
            if (i != 0) std::cout << ", ";
            node.args[i]->accept(*this);
        }
        std::cout << ")";
    }
    
    void visit(PrintlnIntStmt& node) override {
        std::cout << "println_int(";
        node.arg->accept(*this);
        std::cout << ")";
    }
    
    void visit(ExpressionStatement& node) override {
        node.expr->accept(*this);
    }
    
    void visit(Block& node) override {
        std::cout << "{\n";
        for (auto& stmt : node.statements) {
            std::cout << "  ";
            stmt->accept(*this);
            std::cout << ";\n";
        }
        std::cout << "}";
    }
    
    void visit(FunctionDecl& node) override {
        std::cout << node.returnType << " " << node.name << "(";
        for (size_t i = 0; i < node.params.size(); ++i) {
            if (i != 0) std::cout << ", ";
            std::cout << node.params[i].first << " " << node.params[i].second;
        }
        std::cout << ") ";
        node.body->accept(*this);
    }

    void visit(ConditionStatement& node) {
    std::cout << "if (";
    node.condition->accept(*this);
    std::cout << ") ";
    node.thenBlock->accept(*this);
    if (node.elseBlock) {
        std::cout << " else ";
        node.elseBlock->accept(*this);
    }
}

    void visit(LoopStatement& node) {
        std::cout << "while (";
        node.condition->accept(*this);
        std::cout << ") ";
        node.body->accept(*this);
    }

    void visit(BreakStmt& node) {
        std::cout << "break";
    }

    void visit(ContinueStmt& node) {
        std::cout << "continue";
    }
    
    void print(ASTNode& node) {
        node.accept(*this);
        std::cout << std::endl;
    }

    void visit(Program& node) {
    for (auto& func : node.functions) {
        func->accept(*this);
        std::cout << "\n\n";
    }
}
};

#endif // PARSER_H


