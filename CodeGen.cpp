#include "CodeGen.h"

CodeGen::CodeGen(std::unique_ptr<Program> ast) : ast_(std::move(ast)) {
    // 初始化寄存器状态
    for (const auto& reg : registers_) {
        reg_used_[reg] = false;
    }
}

void CodeGen::generateCode() {
    // 生成汇编前导代码
    emit(".intel_syntax noprefix");
    emit(".global main");
    emit(".extern printf");

    emit(".data");
    emit("format_str: .asciz \"%d\\n\""); // printf格式字符串

    emit(".text");
    emit("");
    
    // 生成主函数代码
    genFunction(*ast_);
}

void CodeGen::genFunction(const Program& program) {

    // 生成函数体代码
    for (const auto& func : program.functions) {
        // std::cout  << "Generating function: " << func->name << std::endl;
        if (auto decl = dynamic_cast<const FunctionDecl*>(func.get())) {
            genFunctionDecl(*decl);
        } else {
            throw std::runtime_error("Unknown function type");
        }
    }
    
}

void CodeGen::genBlock(const Block& block) {
    for (const auto& stmt : block.statements) {
        genStatement(*stmt);
    }
}

void CodeGen::genStatement(const Statement& stmt) {
    if (auto decl = dynamic_cast<const VariableDecl*>(&stmt)) {
        // 变量声明
        genVariableDecl(*decl);
    } else if (auto assign = dynamic_cast<const Assignment*>(&stmt)) {
        // 赋值语句
        genAssignment(*assign);
    } else if (auto ret = dynamic_cast<const ReturnStmt*>(&stmt)) {
        // 返回语句
        genReturn(*ret);
    } else if (auto print = dynamic_cast<const PrintlnIntStmt*>(&stmt)) {
        // println_int语句
        genPrintlnInt(*print);
    } else if (auto block = dynamic_cast<const Block*>(&stmt)) {
        // 代码块
        genBlock(*block);
    } else if (auto cond = dynamic_cast<const ConditionStatement*>(&stmt)) {
        genCondition(*cond);
    } else if (auto loop = dynamic_cast<const LoopStatement*>(&stmt)) {
        genLoop(*loop);
    } else if (auto brk = dynamic_cast<const BreakStmt*>(&stmt)) {
        genBreak(*brk);
    } else if (auto cont = dynamic_cast<const ContinueStmt*>(&stmt)) {
        genContinue(*cont);
    } else{
        // 其他语句类型/错误处理
        throw std::runtime_error("Unknown statement type");
    }
}


void CodeGen::genPrintlnInt(const PrintlnIntStmt& print) {
    // 生成参数表达式的代码（可以是变量、字面量或函数调用）
    genExpression(*print.arg);  // 结果会在eax中
    
    // 将结果压栈准备printf调用
    emit("  push eax");
    emit("  push offset format_str");
    emit("  call printf");
    emit("  add esp, 8");
}

void CodeGen::genVariableDecl(const VariableDecl& decl) {
    // 添加到局部变量列表
    local_vars_[current_function_name_].push_back(decl.varName->name);
    
    if (decl.value) {
        // 初始化赋值
        genAssignment(Assignment(
            std::make_unique<Variable>(decl.varName->name),
            std::move(const_cast<std::unique_ptr<Expression>&>(decl.value))
        ));
    }
}

void CodeGen::genAssignment(const Assignment& assign) {
    genExpression(*assign.value);

    int index = findIndex(assign.varName->name);

    // 获取变量在栈中的位置
    int offset = (index + 1) * 4;
    emit("  mov DWORD PTR [ebp-" + std::to_string(offset) + "], eax");
}

void CodeGen::genCondition(const ConditionStatement& cond) {
    std::string elseLabel = newLabel();
    std::string endLabel = newLabel();
    genExpression(*cond.condition);
    emit("  cmp eax, 0");
    emit("  je " + elseLabel); // 如果条件为假，跳转到else部分

    genBlock(*cond.thenBlock); // 生成then部分代码
    emit("  jmp " + endLabel); // 跳过else部分
    
    emit(elseLabel + ":");
    if(cond.elseBlock){
        genBlock(*cond.elseBlock); // 生成else部分代码
    }

    emit(endLabel + ":");
}

void CodeGen::genLoop(const LoopStatement& loop) {
    std::string startLabel = newLabel();
    std::string endLabel = newLabel();
    
    loop_labels_[current_function_name_] = {startLabel, endLabel};

    emit(startLabel + ":");

    genExpression(*loop.condition);
    emit("  cmp eax, 0");
    emit("  je " + endLabel); // 如果条件为假，跳转到循环结束

    genBlock(*loop.body); // 生成循环体代码

    emit("  jmp " + startLabel); // 跳回循环开始
    emit(endLabel + ":");

    loop_labels_.erase(current_function_name_); // 清除当前函数的循环标签
}

void CodeGen::genBreak(const BreakStmt& breakStmt) {
    auto labels = loop_labels_[current_function_name_];
    if(!labels.second.empty()){
       emit("  jmp " + labels.second);
    } else {
        throw std::runtime_error("Break statement not inside a loop");
    }

}

void CodeGen::genContinue(const ContinueStmt& continueStmt) {
    auto labels = loop_labels_[current_function_name_];
    if(!labels.first.empty()){
       emit("  jmp " + labels.first);
    } else {
        throw std::runtime_error("Continue statement not inside a loop");
    }
}


void CodeGen::genReturn(const ReturnStmt& ret) {
    if (ret.value) {
        genExpression(*ret.value); // 返回值在eax中
    } else {
        emit("  mov eax, 0"); // 默认返回0
    }
    emit("  leave");
    emit("  ret");
}

void CodeGen::genExpression(const Expression& expr) {
    if (auto lit = dynamic_cast<const IntegerLiteral*>(&expr)) {
        genIntegerLiteral(*lit);
    } else if (auto var = dynamic_cast<const Variable*>(&expr)) {
        genVariable(*var);
    } else if (auto op = dynamic_cast<const BinaryOp*>(&expr)) {
        genBinaryOp(*op);
    } else if (auto call = dynamic_cast<const FunctionCall*>(&expr)) {
        genFunctionCall(*call);  // 添加函数调用支持
    }
    // 可以添加其他表达式类型的处理
}

void CodeGen::genIntegerLiteral(const IntegerLiteral& lit) {
    emit("  mov eax, " + std::to_string(lit.value));
}

void CodeGen::genFunctionCall(const FunctionCall& call) {
    emit("  push ecx");
    emit("  push edx");
    
    // 压入参数(从右到左)
    for (int i = call.args.size() - 1; i >= 0; --i) {
        genExpression(*call.args[i]);
        emit("  push eax");
    }
    
    emit("  call " + call.functionName);
    
    // 清理参数栈
    if (!call.args.empty()) {
        emit("  add esp, " + std::to_string(call.args.size() * 4));
    }
    
    // 恢复调用者保存的寄存器
    emit("  pop edx");
    emit("  pop ecx");
}

void CodeGen::genFunctionDecl(const FunctionDecl& func) {
    emit(func.name + ":");
    emit("  push ebp");
    emit("  mov ebp, esp");
    
    // 为局部变量分配空间 (每个变量4字节)
    int local_var_size = 16; // 基础空间
    emit("  sub esp, " + std::to_string(local_var_size));
    
    // 保存参数信息
    param_counts_[func.name] = func.params.size();
    for (const auto& param : func.params) {
        func_params_[func.name].push_back(param.second);
    }
    
    current_function_name_ = func.name;
    genBlock(*func.body);
    
}


void CodeGen::genVariable(const Variable& var) {
    // 先检查是否是参数
    auto& params = func_params_[current_function_name_];
    auto param_it = std::find(params.begin(), params.end(), var.name);
    
    if (param_it != params.end()) {
        // 参数访问 (正偏移)
        int param_index = std::distance(params.begin(), param_it);
        int offset = 8 + param_index * 4;  // ebp+8第一个参数
        emit("  mov eax, DWORD PTR [ebp+" + std::to_string(offset) + "]");
    } else {
        // 局部变量访问 (负偏移)
        auto& locals = local_vars_[current_function_name_];
        auto local_it = std::find(locals.begin(), locals.end(), var.name);
        
        if (local_it == locals.end()) {
            locals.push_back(var.name);
            local_it = locals.end() - 1;
        }
        
        int local_index = std::distance(locals.begin(), local_it);
        int offset = (local_index + 1) * 4;  // ebp-4第一个局部变量
        emit("  mov eax, DWORD PTR [ebp-" + std::to_string(offset) + "]");
    }
}


void CodeGen::genBinaryOp(const BinaryOp& op) {
    genExpression(*op.left);
    emit("  push eax"); // 保存左操作数
    genExpression(*op.right);
    emit("  mov ebx, eax");
    emit("  pop eax"); // 恢复左操作数
    
    if (op.op == "+") {
        emit("  add eax, ebx");
    } else if (op.op == "-") {
        emit("  sub eax, ebx");
    } else if (op.op == "*") {
        emit("  imul eax, ebx");
    } else if (op.op == "/") {
        // emit("  xchg eax, ebx");
        emit("  cdq");
        emit("  idiv ebx");
    } else if (op.op == "%") {
        emit("  cdq");
        emit("  idiv ebx");
        emit("  mov eax, edx"); // 余数在edx中
    } 
    else if (op.op == "==") {
        emit("  cmp eax, ebx");
        emit("  sete al");
        emit("  movzx eax, al");
    } else if (op.op == "!=") {
        emit("  cmp eax, ebx");
        emit("  setne al");
        emit("  movzx eax, al");
    } else if (op.op == "<") {
        emit("  cmp eax, ebx");
        emit("  setl al");
        emit("  movzx eax, al");
    } else if (op.op == "<=") {
        emit("  cmp eax, ebx");
        emit("  setle al");
        emit("  movzx eax, al");
    } else if (op.op == ">") {
        emit("  cmp eax, ebx");
        emit("  setg al");
        emit("  movzx eax, al");
    } else if (op.op == ">=") {
        emit("  cmp eax, ebx");
        emit("  setge al");
        emit("  movzx eax, al");
    } else if(op.op == "|"){
        emit("  or eax, ebx");
    } else if(op.op == "&"){
        emit("  and eax, ebx");
    } else if(op.op == "^"){
        emit("  xor eax, ebx");
    } else if(op.op == "&&"){
        emit("  and eax, ebx");
    } else if(op.op == "||"){
        emit("  or eax, ebx");
    } else {
        throw std::runtime_error("Unknown binary operator: " + op.op);
    }
    
    // 可以添加其他运算符的处理
}


std::string CodeGen::getRegister() {
    for (const auto& reg : registers_) {
        if (!reg_used_[reg]) {
            reg_used_[reg] = true;
            return reg;
        }
    }
    throw std::runtime_error("No available registers");
}

void CodeGen::freeRegister(const std::string& reg) {
    reg_used_[reg] = false;
}

void CodeGen::emit(const std::string& code) {
    std::cout << code << std::endl;
}

std::string CodeGen::newLabel() {
    return "label_" + std::to_string(label_count_++);
}

// std::string CodeGen::newLabel() {
//     return "label_" + std::to_string(label_count_++);
// }


