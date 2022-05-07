#include "parser.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

namespace ast {
Parser::Parser(std::string source) {
  std::cout << "Generating AST" << std::endl;
  tokenizer::Lexer lexer(source);
  lexer.last->next = new tokenizer::Token(tokenizer::TokenType::None, "");
  this->token_head = lexer.head;

  this->ast_root = new Node();
  ast_root->type = NodeType::Block;
  ast_root->content = "program";
  auto stmt = statement();
  while (stmt->type != NodeType::Invalid) {
    ast_root->children.push_back(stmt);
    stmt = statement();
  }
}

Parser::Parser(const char *source) : Parser(std::string(source)) {}

bool Parser::check(tokenizer::TokenType type) {
  if (!this->token_head) {
    return false;
  }
  return this->token_head->is(type);
}

bool Parser::consume(tokenizer::TokenType type) {
  if (check(type)) {
    this->token_head = this->token_head->get_next();
    return true;
  }

  return false;
}

void Parser::advance() { this->token_head = this->token_head->next; }

tokenizer::Token *Parser::next() {
  tokenizer::Token *head = this->token_head;
  this->token_head = this->token_head->get_next();
  return head;
}

// Type checking

enum Type Parser::evaluate_type(Node *expr) {
  switch (expr->type) {
    case Invalid:
      return Type::Mismatch;
    case BinaryExpr:
      if (expr->content == "=") {
        return evaluate_type(expr->children[1]);
      } else if (expr->content == "+" || expr->content == "-" || expr->content == "*" || expr->content == "/") {
        enum Type lhs = evaluate_type(expr->children[0]);
        enum Type rhs = evaluate_type(expr->children[1]);

        if (lhs != rhs) {
          return Type::Mismatch;
        }

        return lhs;
      }
    case Grouping:
    case UnaryExpr:
    case Argument:
    case Integer:
      return evaluate_type(expr->children[0]);
    case Fn:
      return evaluate_type(expr->children[2]);
    case Type:

      if (expr->type == NodeType::Identifier) {
        for (auto var : this->variables) {
          if (var.identifier == expr->content) {
            return var.type;
          }
        }
      } else if (expr->type == NodeType::Integer) {
        return evaluate_type(expr->children[0]);
      }

      if (expr->content == "u8") {
        return Type::U8;
      } else if (expr->content == "u16") {
        return Type::U16;
      } else if (expr->content == "u32") {
        return Type::U32;
      } else if (expr->content == "u64") {
        return Type::U64;
      } else if (expr->content == "i8") {
        return Type::I8;
      } else if (expr->content == "i16") {
        return Type::I16;
      } else if (expr->content == "i32") {
        return Type::I32;
      } else if (expr->content == "i64") {
        return Type::I64;
      } else if (expr->content == "f32") {
        return Type::F32;
      } else if (expr->content == "f64") {
        return Type::F64;
      } else if (expr->content == "bool") {
        return Type::Bool;
      } else {
        return Type::Mismatch;
      }
    case Identifier:
      for (auto var : this->variables) {
        if (var.identifier == expr->content) {
          return var.type;
        }
      }
      return Type::Mismatch;
    case Call:
      for (auto func : this->functions) {
        if (func.identifier == expr->content) {
          if (func.arguments.size() != expr->children[0]->children.size()) return Type::Mismatch;
          for (int i=0; i < func.arguments.size(); i++) {
            auto arg = func.arguments[i];
            if (arg.type != evaluate_type(expr->children[0]->children[i])) {
              return Type::Mismatch;
            }
          }
          return func.type;
        }
      }
      return Type::Mismatch;
    default:
      return Type::Mismatch;
  }
}

// Reporting

void Parser::parsing_error(std::string message) {
  std::cout << "Parsing error: " << message << std::endl;
  exit(-1);
}

// Statement parsing

Node *Parser::statement() {
  if (consume(tokenizer::TokenType::Let)) {
    return let_statement();
  } else if (consume(tokenizer::TokenType::Ret)) {
    return ret();
  } else if (consume(tokenizer::TokenType::Fn)) {
    return function_statement();
  }

  auto inv = new Node();
  inv->type = NodeType::Invalid;
  inv->content = "invalid";

  return inv;
}

Node *Parser::let_statement() {
  auto expr = expression();
  if (expr->type != NodeType::BinaryExpr && expr->content != "=") {
    parsing_error("Expected `assignment`, found " + expr->content);
  }

  auto node = new Node();
  node->type = NodeType::Let;
  node->content = "let";
  node->children.push_back(expr);

  if (!consume(tokenizer::TokenType::SemiColon)) {
    parsing_error("Expected ';', found " + this->token_head->lexeme);
  }

  Variable var{};
  var.identifier = expr->children[0]->content;
  var.type = evaluate_type(expr);
  this->variables.push_back(var);

  return node;
}

Node *Parser::function_statement() {
  auto id = next();
  if (!id->is(tokenizer::TokenType::Identifier))
    parsing_error("Expected `identifier`, found " + id->lexeme);
  auto identifier = new Node();
  identifier->type = NodeType::Identifier;
  identifier->content = id->lexeme;

  auto args = arguments();

  auto type = new Node();
  type->type = NodeType::Type;
  type->content = "void";

  if (consume(tokenizer::TokenType::RightArrow)) {
      auto tok = next();
      if (!tok->is(tokenizer::TokenType::Type)) parsing_error("Expected `type`, found " + tok->lexeme);
      type->content = tok->lexeme;
  }

  this->variables.clear();

  Function func{};
  func.identifier = identifier->content;
  func.type = evaluate_type(type);

  for (auto child : args->children) {
    Variable var{};
    var.identifier = child->content;
    var.type = evaluate_type(child);
    this->variables.push_back(var);
    func.arguments.push_back(var);
  }

  this->functions.push_back(func);

  auto block = block_statement();

  auto ret = new Node();

  ret->type = NodeType::Fn;
  ret->content = "fn";
  ret->children.push_back(identifier);
  ret->children.push_back(args);
  ret->children.push_back(type);
  ret->children.push_back(block);

  return ret;
}

Node *Parser::block_statement() {
  if (!consume(tokenizer::TokenType::LCurly))
    parsing_error("Expected '{', found " + this->token_head->lexeme);

  auto block = new Node();
  block->type = NodeType::Block;
  block->content = "block";

  auto stmt = statement();

  while(stmt->type != NodeType::Invalid) {
    block->children.push_back(stmt);
    stmt = statement();
  }

  if (!consume(tokenizer::TokenType::RCurly))
    parsing_error("Expected '}', found " + this->token_head->lexeme);

  return block;
}

Node *Parser::ret() {
  auto ret = new Node();
  ret->type = NodeType::Ret;
  ret->content = "ret";
  ret->children.push_back(expression());
  if (!consume(tokenizer::TokenType::SemiColon))
    parsing_error("Expected ';', found " + this->token_head->lexeme);
  return ret;
}

Node *Parser::type() {
  auto tok = next();
  if (!tok->is(tokenizer::TokenType::Type))
    parsing_error("Expected `type`, found " + tok->lexeme);
  auto ret = new Node();
  ret->type = NodeType::Type;
  ret->content = tok->lexeme;

  return ret;
}

// Function utilities

Node *Parser::argument() {
  auto id = next();
  if (!id->is(tokenizer::TokenType::Identifier))
    parsing_error("Expected `identifier`, found " + id->lexeme);
  auto identifier = new Node();
  identifier->type = NodeType::Identifier;
  identifier->content = id->lexeme;
  if (!consume(tokenizer::TokenType::Colon))
    parsing_error("Expected ':', found " + this->token_head->lexeme);
  auto type = this->type();

  auto arg = new Node();
  arg->type = NodeType::Argument;
  arg->content = identifier->content;
  arg->children.push_back(type);

  return arg;
}

Node *Parser::arguments() {
  if (!consume(tokenizer::TokenType::LParen))
    parsing_error("Expected '(', found " + this->token_head->lexeme);
  if (consume(tokenizer::TokenType::RParen)) {
    auto ret = new Node();
    ret->type = NodeType::Arguments;
    ret->content = "args";
    return ret;
  }

  auto ret = new Node();
  ret->type = NodeType::Arguments;

  auto arg = argument();
  ret->children.push_back(arg);

  while (consume(tokenizer::TokenType::Comma)) {
    auto arg = argument();
    ret->children.push_back(arg);
  }

  ret->content = "args";

  if (!consume(tokenizer::TokenType::RParen))
    parsing_error("Expected ')', found " + this->token_head->lexeme);

  return ret;
}

// Expression parsing

Node *Parser::expression() {
  auto res = assignment();
  if (evaluate_type(res) == Type::Mismatch) {
    parsing_error("Mismatched types!");
  }
  return res;
}

Node *Parser::assignment() {
  Node *res = term();

  if (check(tokenizer::TokenType::Equal)) {
    Node *new_node = new Node();
    new_node->content = this->token_head->lexeme;
    new_node->type = NodeType::BinaryExpr;
    advance();
    Node *rhs = term();
    new_node->children.push_back(res);
    new_node->children.push_back(rhs);
    res = new_node;
  }

  return res;
}

Node *Parser::term() {
  Node *res = factor();

  while (check(tokenizer::TokenType::Plus) ||
         check(tokenizer::TokenType::Minus)) {

    Node *new_node = new Node();
    new_node->content = this->token_head->lexeme;
    new_node->type = NodeType::BinaryExpr;
    advance();
    Node *rhs = factor();
    new_node->children.push_back(res);
    new_node->children.push_back(rhs);
    res = new_node;
  }

  return res;
}

Node *Parser::factor() {
  Node *res = unary();

  while (check(tokenizer::TokenType::Star) ||
         check(tokenizer::TokenType::Slash)) {
    Node *new_node = new Node();
    new_node->content = this->token_head->lexeme;
    new_node->type = NodeType::BinaryExpr;
    advance();
    Node *rhs = unary();
    new_node->children.push_back(res);
    new_node->children.push_back(rhs);
    res = new_node;
  }

  return res;
}

Node *Parser::unary() {
  if (check(tokenizer::TokenType::Minus)) {
    advance();
    Node *new_node = new Node();
    new_node->children.push_back(primary());
    new_node->content = "-";
    new_node->type = NodeType::UnaryExpr;
    return new_node;
  } else {
    return primary();
  }
}

Node *Parser::primary() {
  tokenizer::Token *tok = next();
  Node *node = new Node();

  if (tok->is(tokenizer::TokenType::I32Literal)) {
    node->type = NodeType::Integer;
    node->content = tok->lexeme;
    auto type = new Node();
    type->type = NodeType::Type;
    type->content = "i32";

    if (check(tokenizer::TokenType::Type)) {
      type->content = this->token_head->lexeme;
      advance();
    }

    node->children.push_back(type);
  } else if (tok->is(tokenizer::TokenType::LParen)) {
    auto expr = expression();
    consume(tokenizer::TokenType::RParen);
    node->type = NodeType::Grouping;
    node->content = "group";
    node->children.push_back(expr);
  } else if (tok->is(tokenizer::TokenType::Identifier)) {
    if (check(tokenizer::TokenType::LParen)) {
      advance();
      auto params = new Node();
      params->type = NodeType::Parameters;
      params->content = "params";

      if (!check(tokenizer::TokenType::RParen)) {
        params->children.push_back(expression());

        while (consume(tokenizer::TokenType::Comma)) {
          params->children.push_back(expression());
        }
      }

      if (!consume(tokenizer::TokenType::RParen))
        parsing_error("Expected ')', found " + this->token_head->lexeme);
      node->type = NodeType::Call;
      node->content = tok->lexeme;
      node->children.push_back(params);
    } else {
      node->type = NodeType::Identifier;
      node->content = tok->lexeme;
    }
  } else {
    parsing_error("Expected `primary`, found " + tok->lexeme);
  }

  return node;
}
} // namespace ast
