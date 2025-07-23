#ifndef CODEGEN_H
#define CODEGEN_H

#include "Parser.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>

class CodeGen {
public:
    CodeGen(std::unique_ptr<Program> ast);
    void generateCode();

private:
    std::unique_ptr<Program> ast_;
    std::unordered_map<std::string, int> var_map_; // 变量到栈偏移的映射
    int stack_offset_ = 0; // 当前栈偏移量
    int label_count_ = 0;  // 标签计数器
    
    // 寄存器管理
    const std::vector<std::string> registers_ = {"eax", "ebx", "ecx", "edx", "esi", "edi"};
    std::unordered_map<std::string, bool> reg_used_;

    std::unordered_map<std::string, std::vector<std::string>> func_params_;  // 函数参数映射
    std::unordered_map<std::string, std::vector<std::string>> local_vars_;   // 局部变量映射
    std::unordered_map<std::string, int> param_counts_;                      // 函数参数计数

    std::unordered_map<std::string, std::pair<std::string, std::string>> loop_labels_;

    std::unordered_map<std::string, std::vector<std::string>> funct_vars_; // 函数调用列表
    // functionName, vars[]
    int current_function_stack_size_ = 0; // 当前函数栈大小
    std::string current_function_name_;

    std::string getRegister();
    void freeRegister(const std::string& reg);
    
    // AST节点代码生成方法
    void genFunction(const Program& program);
    void genBlock(const Block& block);
    void genStatement(const Statement& stmt);
    void genExpression(const Expression& expr);
    void genVariableDecl(const VariableDecl& decl);
    void genAssignment(const Assignment& assign);
    void genReturn(const ReturnStmt& ret);
    void genPrintlnInt(const PrintlnIntStmt& print);
    void genBinaryOp(const BinaryOp& op);
    void genVariable(const Variable& var);
    void genIntegerLiteral(const IntegerLiteral& lit);
    void genFunctionCall(const FunctionCall& call);
    void genFunctionDecl(const FunctionDecl& func);
    void genCondition(const ConditionStatement& cond);
    void genLoop(const LoopStatement& loop);
    void genBreak(const BreakStmt& breakStmt);
    void genContinue(const ContinueStmt& continueStmt);
    
    // 工具方法
    void emit(const std::string& code);
    int findIndex(const std::string& varName) {    
        auto& vars = funct_vars_[current_function_name_]; 
        auto it = std::find(vars.begin(), vars.end(), varName);
        if (it == vars.end()) {
            vars.push_back(varName);
            return vars.size() - 1;
        }
        return std::distance(vars.begin(), it);
    }

    std::string newLabel();
};

#endif // CODEGEN_H

