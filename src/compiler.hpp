#ifndef COMPILER_H_
#define COMPILER_H_

#include "parser.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

static std::unique_ptr<llvm::LLVMContext> context;
static std::unique_ptr<llvm::IRBuilder<>> builder;
static std::unique_ptr<llvm::Module> module;

class Compiler {
public:
  ast::Node *ast_root;
  Compiler(std::string source);
  void compile();

private:
  llvm::Value *compile_expr(ast::Node *expr);
  void compile_statement(ast::Node *stmt);
        std::map<std::string, llvm::Value*> variables;
};

#endif // COMPILER_H_
