#ifndef PARSER_H_
#define PARSER_H_

#include "token.hpp"
#include <string>
#include <vector>
namespace ast {

enum Type { Mismatch = -1, U8, U16, U32, U64, I8, I16, I32, I64, F32, F64, Bool };

enum NodeType {
  Invalid = -1,
  BinaryExpr,
  UnaryExpr,
  Call,
  Parameters,
  Integer,
  Float,
  Grouping,
  Let,
  Identifier,
  Fn,
  Arguments,
  Argument,
  Type,
  Block,
  Ret,
};

struct Variable {
  std::string identifier;
  enum Type type;
};

struct Function {
  std::string identifier;
  std::vector<Variable> arguments;
  enum Type type;
};

class Node {
public:
  NodeType type;
  std::string content;
  std::vector<Node *> children;
};

class Parser {
public:
  Parser(std::string source);
  Parser(const char *source);
  tokenizer::Token *token_head;
  Node *ast_root;
  std::vector<Variable> variables;
  std::vector<Function> functions;

private:
  // Type checking
  enum Type evaluate_type(Node *expr);

  // Reporting
  void parsing_error(std::string message);

  // Tokens utilities
  bool check(tokenizer::TokenType type);
  bool consume(tokenizer::TokenType type);
  void advance();
  tokenizer::Token *next();

  // Statement parsing
  Node *statement();
  Node *let_statement();
  Node *function_statement();
  Node *block_statement();
  Node *type();
  Node *ret();

  // Functions utilities
  Node *argument();
  Node *arguments();

  // Expression parsing
  Node *expression();
  Node *assignment();
  Node *term();
  Node *factor();
  Node *unary();
  Node *primary();
};
} // namespace ast

#endif // PARSER_H_
