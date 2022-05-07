#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>
#include "compiler.hpp"

void print_ast(ast::Node *root, std::string prefix) {
  switch (root->type) {
  case ast::Type:
  case ast::Invalid:
  case ast::Identifier:
    std::cout << prefix << "└── " << root->content << std::endl;
    break;
  case ast::BinaryExpr:
  case ast::Let:
  case ast::Integer:
  case ast::Float:
  case ast::Grouping:
  case ast::Argument:
  case ast::Fn:
  case ast::Arguments:
  case ast::Block:
  case ast::Ret:
  case ast::Call:
  case ast::UnaryExpr:
  case ast::Parameters:
    std::cout << prefix << "├── " << root->content << std::endl;
    for (ast::Node *child : root->children) {
      print_ast(child, prefix + "|   ");
    }
    break;
  }

}

int main(int argc, char **argv) {

  std::cout << "Compiling " << argv[1] << std::endl;

  std::string filename = argv[1];

  std::ifstream file(filename, std::ios::in);
  std::stringstream buf;
  buf << file.rdbuf();

  ast::Parser parser(buf.str());

  std::cout << "Generating AST for: " << buf.str() << std::endl;

  print_ast(parser.ast_root, "");

  Compiler compiler(buf.str());
  compiler.compile();

  return 0;
}
