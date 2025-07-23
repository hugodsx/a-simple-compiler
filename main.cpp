#include ".\Lexer.h"
#include ".\Parser.h"

#include ".\CodeGen.h"
#include <fstream>


void testParser(const std::string& sourceCode) {
    std::cout << "=== Testing: " << sourceCode << " ===" << std::endl;
    
    Lexer lexer(sourceCode);
    auto tokens = lexer.tokenize();
    
    Parser parser(tokens);
    try {
        auto ast = parser.parse();
        std::cout << "Parse successful! AST:" << std::endl;
        
        ASTPrinter printer;
        printer.print(*ast);
        
    } catch (const std::runtime_error& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
    }
    
    std::cout << "=====================" << std::endl << std::endl;
}


int main(int argc, char* argv[]) {
     if (argc < 2) {
         std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
         return 1;
     }

     std::ifstream inputFile(argv[1]);
     if (!inputFile.is_open()) {
         std::cerr << "Error: Could not open file " << argv[1] << std::endl;
         return 1;
     }

     std::string source(
         (std::istreambuf_iterator<char>(inputFile)),
         std::istreambuf_iterator<char>()
     );

//    std::string source = "int main() {\
//    int a = 5, b = 3;\
//    if (a <= b) {\
        println_int(a);\
    }\
\
    println_int(b);\
    return 0;\
}";

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

//    testParser(source);
	
    Parser parser(tokens);
    auto codeGenerator = CodeGen(parser.parse());
    codeGenerator.generateCode();
    
    return 0;
}


