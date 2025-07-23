#include "CodeGen.h"

CodeGen::CodeGen(std::unique_ptr<Program> ast) : ast_(std::move(ast)) {
    // ��ʼ���Ĵ���״̬
    for (const auto& reg : registers_) {
        reg_used_[reg] = false;
    }
}

void CodeGen::generateCode() {
    // ���ɻ��ǰ������
    emit(".intel_syntax noprefix");
    emit(".global main");
    emit(".extern printf");

    emit(".data");
    emit("format_str: .asciz \"%d\\n\""); // printf��ʽ�ַ���

    emit(".text");
    emit("");
    
    // ��������������
    genFunction(*ast_);
}

void CodeGen::genFunction(const Program& program) {

    // ���ɺ��������
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
        // ��������
        genVariableDecl(*decl);
    } else if (auto assign = dynamic_cast<const Assignment*>(&stmt)) {
        // ��ֵ���
        genAssignment(*assign);
    } else if (auto ret = dynamic_cast<const ReturnStmt*>(&stmt)) {
        // �������
        genReturn(*ret);
    } else if (auto print = dynamic_cast<const PrintlnIntStmt*>(&stmt)) {
        // println_int���
        genPrintlnInt(*print);
    } else if (auto block = dynamic_cast<const Block*>(&stmt)) {
        // �����
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
        // �����������/������
        throw std::runtime_error("Unknown statement type");
    }
}


void CodeGen::genPrintlnInt(const PrintlnIntStmt& print) {
    // ���ɲ������ʽ�Ĵ��루�����Ǳ������������������ã�
    genExpression(*print.arg);  // �������eax��
    
    // �����ѹջ׼��printf����
    emit("  push eax");
    emit("  push offset format_str");
    emit("  call printf");
    emit("  add esp, 8");
}

void CodeGen::genVariableDecl(const VariableDecl& decl) {
    // ��ӵ��ֲ������б�
    local_vars_[current_function_name_].push_back(decl.varName->name);
    
    if (decl.value) {
        // ��ʼ����ֵ
        genAssignment(Assignment(
            std::make_unique<Variable>(decl.varName->name),
            std::move(const_cast<std::unique_ptr<Expression>&>(decl.value))
        ));
    }
}

void CodeGen::genAssignment(const Assignment& assign) {
    genExpression(*assign.value);

    int index = findIndex(assign.varName->name);

    // ��ȡ������ջ�е�λ��
    int offset = (index + 1) * 4;
    emit("  mov DWORD PTR [ebp-" + std::to_string(offset) + "], eax");
}

void CodeGen::genCondition(const ConditionStatement& cond) {
    std::string elseLabel = newLabel();
    std::string endLabel = newLabel();
    genExpression(*cond.condition);
    emit("  cmp eax, 0");
    emit("  je " + elseLabel); // �������Ϊ�٣���ת��else����

    genBlock(*cond.thenBlock); // ����then���ִ���
    emit("  jmp " + endLabel); // ����else����
    
    emit(elseLabel + ":");
    if(cond.elseBlock){
        genBlock(*cond.elseBlock); // ����else���ִ���
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
    emit("  je " + endLabel); // �������Ϊ�٣���ת��ѭ������

    genBlock(*loop.body); // ����ѭ�������

    emit("  jmp " + startLabel); // ����ѭ����ʼ
    emit(endLabel + ":");

    loop_labels_.erase(current_function_name_); // �����ǰ������ѭ����ǩ
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
        genExpression(*ret.value); // ����ֵ��eax��
    } else {
        emit("  mov eax, 0"); // Ĭ�Ϸ���0
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
        genFunctionCall(*call);  // ��Ӻ�������֧��
    }
    // ��������������ʽ���͵Ĵ���
}

void CodeGen::genIntegerLiteral(const IntegerLiteral& lit) {
    emit("  mov eax, " + std::to_string(lit.value));
}

void CodeGen::genFunctionCall(const FunctionCall& call) {
    emit("  push ecx");
    emit("  push edx");
    
    // ѹ�����(���ҵ���)
    for (int i = call.args.size() - 1; i >= 0; --i) {
        genExpression(*call.args[i]);
        emit("  push eax");
    }
    
    emit("  call " + call.functionName);
    
    // �������ջ
    if (!call.args.empty()) {
        emit("  add esp, " + std::to_string(call.args.size() * 4));
    }
    
    // �ָ������߱���ļĴ���
    emit("  pop edx");
    emit("  pop ecx");
}

void CodeGen::genFunctionDecl(const FunctionDecl& func) {
    emit(func.name + ":");
    emit("  push ebp");
    emit("  mov ebp, esp");
    
    // Ϊ�ֲ���������ռ� (ÿ������4�ֽ�)
    int local_var_size = 16; // �����ռ�
    emit("  sub esp, " + std::to_string(local_var_size));
    
    // ���������Ϣ
    param_counts_[func.name] = func.params.size();
    for (const auto& param : func.params) {
        func_params_[func.name].push_back(param.second);
    }
    
    current_function_name_ = func.name;
    genBlock(*func.body);
    
}


void CodeGen::genVariable(const Variable& var) {
    // �ȼ���Ƿ��ǲ���
    auto& params = func_params_[current_function_name_];
    auto param_it = std::find(params.begin(), params.end(), var.name);
    
    if (param_it != params.end()) {
        // �������� (��ƫ��)
        int param_index = std::distance(params.begin(), param_it);
        int offset = 8 + param_index * 4;  // ebp+8��һ������
        emit("  mov eax, DWORD PTR [ebp+" + std::to_string(offset) + "]");
    } else {
        // �ֲ��������� (��ƫ��)
        auto& locals = local_vars_[current_function_name_];
        auto local_it = std::find(locals.begin(), locals.end(), var.name);
        
        if (local_it == locals.end()) {
            locals.push_back(var.name);
            local_it = locals.end() - 1;
        }
        
        int local_index = std::distance(locals.begin(), local_it);
        int offset = (local_index + 1) * 4;  // ebp-4��һ���ֲ�����
        emit("  mov eax, DWORD PTR [ebp-" + std::to_string(offset) + "]");
    }
}


void CodeGen::genBinaryOp(const BinaryOp& op) {
    genExpression(*op.left);
    emit("  push eax"); // �����������
    genExpression(*op.right);
    emit("  mov ebx, eax");
    emit("  pop eax"); // �ָ��������
    
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
        emit("  mov eax, edx"); // ������edx��
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
    
    // �����������������Ĵ���
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


