#include "compiler.hpp"
#include "parser.hpp"
#include <cstdlib>
#include <iostream>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/Optional.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <memory>
#include <string>
#include <system_error>
#include <vector>

Compiler::Compiler(std::string source) {
  context = std::make_unique<llvm::LLVMContext>();
  module = std::make_unique<llvm::Module>("program", *context);
  builder = std::make_unique<llvm::IRBuilder<>>(*context);
  this->ast_root = (new ast::Parser(source))->ast_root;
}

void Compiler::compile() {
  for (auto child : this->ast_root->children) {
    this->compile_statement(child);
  }

  std::string ir;
  llvm::raw_string_ostream os(ir);
  os << *module;
  os.flush();
  std::cout << ir << std::endl;


  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  auto target_triple = llvm::sys::getDefaultTargetTriple();
  module->setTargetTriple(target_triple);

  std::string error;
  auto target = llvm::TargetRegistry::lookupTarget(target_triple, error);

  if (!target) {
    llvm::errs() << error;
    exit(1);
  }

  auto cpu = "generic";
  auto features = "";
  llvm::TargetOptions opt;
  auto rm = llvm::Optional<llvm::Reloc::Model>();
  auto target_machine =
      target->createTargetMachine(target_triple, cpu, features, opt, rm);
  module->setDataLayout(target_machine->createDataLayout());

  auto filename = "output.o";
  std::error_code ec;
  llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);

  if (ec) {
    llvm::errs() << "Could not open file: " << ec.message();
    exit(1);
  }

  llvm::legacy::PassManager pass;
  auto file_type = llvm::CGFT_ObjectFile;

  if (target_machine->addPassesToEmitFile(pass, dest, nullptr, file_type)) {
    llvm::errs() << "TheTargetMachine can't emit a file of this type";
    exit(1);
  }

  pass.run(*module);
  dest.flush();

}

llvm::Type *get_type(ast::Node *expr) {
  if (expr->content == "u8") {
    return llvm::Type::getInt8Ty(*context);
  } else if (expr->content == "u16") {
    return llvm::Type::getInt16Ty(*context);
  } else if (expr->content == "u32") {
    return llvm::Type::getInt32Ty(*context);
  } else if (expr->content == "u64") {
    return llvm::Type::getInt64Ty(*context);
  } else if (expr->content == "i8") {
    return llvm::Type::getInt8Ty(*context);
  } else if (expr->content == "i16") {
    return llvm::Type::getInt16Ty(*context);
  } else if (expr->content == "i32") {
    return llvm::Type::getInt32Ty(*context);
  } else if (expr->content == "i64") {
    return llvm::Type::getInt64Ty(*context);
  } else if (expr->content == "f32") {
    return llvm::Type::getFloatTy(*context);
  } else if (expr->content == "f64") {
    return llvm::Type::getFloatTy(*context);
  } else if (expr->content == "bool") {
    return llvm::Type::getInt1Ty(*context);
  } else if (expr->content == "void") {
    return llvm::Type::getVoidTy(*context);
  } else {
    return llvm::Type::getVoidTy(*context);
  }
}

llvm::Value *Compiler::compile_expr(ast::Node *expr) {
  if (expr->type == ast::NodeType::BinaryExpr) {
    auto lhs = expr->children[0];
    auto rhs = expr->children[1];
    if (expr->content == "+") {
      return builder->CreateAdd(compile_expr(lhs), compile_expr(rhs), "tmpadd");
    } else if (expr->content == "-") {
      return builder->CreateSub(compile_expr(lhs), compile_expr(rhs), "tmpadd");
    } else if (expr->content == "*") {
      return builder->CreateMul(compile_expr(lhs), compile_expr(rhs), "tmpadd");
    } else if (expr->content == "/") {
      return builder->CreateSDiv(compile_expr(lhs), compile_expr(rhs),
                                 "tmpadd");
    } else if (expr->content == "=") {
      return builder->CreateStore(compile_expr(rhs),
                                  this->variables[lhs->content]);
    }
  } else if (expr->type == ast::NodeType::Integer) {
    return llvm::ConstantInt::get(get_type(expr->children[0]),
                                  std::stoi(expr->content));
  } else if (expr->type == ast::NodeType::UnaryExpr) {
    return builder->CreateSub(
        llvm::ConstantInt::get(get_type(expr->children[0]->children[0]), 0),
        compile_expr(expr->children[0]), "tmpsub");
  } else if (expr->type == ast::NodeType::Grouping) {
    return compile_expr(expr->children[0]);
  } else if (expr->type == ast::NodeType::Call) {
    llvm::Function *callee = module->getFunction(expr->content);
    std::vector<llvm::Value *> args;
    for (auto arg : expr->children[0]->children) {
      args.push_back(compile_expr(arg));
    }

    return builder->CreateCall(callee, args, "tmpcall" + expr->content);
  } else if (expr->type == ast::NodeType::Identifier) {
    return builder->CreateLoad(
        variables[expr->content]->getType()->getPointerElementType(),
        variables[expr->content]);
  }
}

void Compiler::compile_statement(ast::Node *stmt) {
  switch (stmt->type) {
  case ast::Fn: {

    this->variables.clear();

    // Generate arguments
    std::vector<llvm::Type *> args;
    auto arguments = stmt->children[1]->children;
    for (auto arg : arguments) {
      args.push_back(get_type(arg->children[0]));
    }
    // Generate function type
    llvm::FunctionType *fn_type =
        llvm::FunctionType::get(get_type(stmt->children[2]), args, false);
    llvm::Function *func =
        llvm::Function::Create(fn_type, llvm::GlobalValue::ExternalLinkage,
                               stmt->children[0]->content, module.get());

    int i = 0;
    for (auto &arg : func->args()) {
      arg.setName(arguments[i]->content);
      this->variables[arguments[i]->content] = &arg;
    }

    llvm::BasicBlock *basic_block =
        llvm::BasicBlock::Create(*context, "entry", func);
    builder->SetInsertPoint(basic_block);

    auto block = stmt->children[3]->children;

    for (auto statement : block) {
      compile_statement(statement);
    }
    if (stmt->children[2]->content == "void") {
      builder->CreateRetVoid();
    }
    break;
  }
  case ast::Let: {
    auto ptr = builder->CreateAlloca(
        get_type(stmt->children[0]->children[1]->children[0]), nullptr,
        stmt->children[0]->children[0]->content);
    this->variables[stmt->children[0]->children[0]->content] = ptr;
    compile_expr(stmt->children[0]);
  } break;
  case ast::Ret:
    builder->CreateRet(compile_expr(stmt->children[0]));
  }
}
