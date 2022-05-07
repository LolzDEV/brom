#include "lexer.hpp"
#include "token.hpp"
#include <cctype>
#include <iostream>
#include <ostream>

namespace tokenizer {
Lexer::Lexer(std::string source) {
  this->head = new Token(TokenType::None, "");
  this->last = head;

  int line = 0;
  int cc = 0;
  std::string identifier;
  const char *s = source.c_str();
  while (*s != '\0' && s != nullptr) {
    if (*s == '\n') {
      line++;
      s++;
      continue;
    }

    switch (*s) {
    case '+':
      this->push(new Token(TokenType::Plus, "+"));
      s++;
      break;
    case '-':
      if (*(s + 1) == '>') {
        this->push(new Token(TokenType::RightArrow, "->"));
        s += 2;
        break;
      }
      this->push(new Token(TokenType::Minus, "-"));
      s++;
      break;
    case '*':
      this->push(new Token(TokenType::Star, "*"));
      s++;
      break;
    case '/':
      this->push(new Token(TokenType::Slash, "/"));
      s++;
      break;
    case '(':
      this->push(new Token(TokenType::LParen, "("));
      s++;
      break;
    case ')':
      this->push(new Token(TokenType::RParen, ")"));
      s++;
      break;
    case '=':
      this->push(new Token(TokenType::Equal, "="));
      s++;
      break;
    case ';':
      this->push(new Token(TokenType::SemiColon, ";"));
      s++;
      break;
    case '{':
      this->push(new Token(TokenType::LCurly, "{"));
      s++;
      break;
    case '}':
      this->push(new Token(TokenType::RCurly, "}"));
      s++;
      break;
    case ':':
      this->push(new Token(TokenType::Colon, ":"));
      s++;
      break;
    case ',':
      this->push(new Token(TokenType::Comma, ","));
      s++;
      break;

      continue;
    }

    if (isspace(*s)) {
      s++;
      continue;
    }

    if (isdigit(*s)) {
      identifier.push_back(*s);
      while (isdigit(*(++s))) {
        identifier.push_back(*s);
      }

      this->push(new Token(TokenType::I32Literal, identifier));
      identifier = "";

      continue;
    }

    if (isalnum(*s)) {
      identifier.push_back(*s);
      while (isalnum(*(++s))) {
        identifier.push_back(*s);
      }
      if (identifier.length() > 0 && identifier != "") {
        if (identifier == "let") {
          this->push(new Token(TokenType::Let, identifier));
          identifier = "";
        } else if (identifier == "fn") {
          this->push(new Token(TokenType::Fn, identifier));
          identifier = "";
        } else if (identifier == "ret") {
          this->push(new Token(TokenType::Ret, identifier));
          identifier = "";
        } else if (identifier == "bool") {
          this->push(new Token(TokenType::Type, identifier));
          identifier = "";
        } else if (identifier == "u8") {
          this->push(new Token(TokenType::Type, identifier));
          identifier = "";
        } else if (identifier == "u16") {
          this->push(new Token(TokenType::Type, identifier));
          identifier = "";
        } else if (identifier == "u32") {
          this->push(new Token(TokenType::Type, identifier));
          identifier = "";
        } else if (identifier == "u64") {
          this->push(new Token(TokenType::Type, identifier));
          identifier = "";
        } else if (identifier == "i8") {
          this->push(new Token(TokenType::Type, identifier));
          identifier = "";
        } else if (identifier == "i16") {
          this->push(new Token(TokenType::Type, identifier));
          identifier = "";
        } else if (identifier == "i32") {
          this->push(new Token(TokenType::Type, identifier));
          identifier = "";
        } else if (identifier == "i64") {
          this->push(new Token(TokenType::Type, identifier));
          identifier = "";
        } else if (identifier == "f32") {
          this->push(new Token(TokenType::Type, identifier));
          identifier = "";
        } else if (identifier == "f64") {
          this->push(new Token(TokenType::Type, identifier));
          identifier = "";
        } else if (identifier == "void") {
          this->push(new Token(TokenType::Type, identifier));
          identifier = "";
        } else {
          this->push(new Token(TokenType::Identifier, identifier));
          identifier = "";
        }
      }
    }

    cc++;
  }

  std::cout << "Finished parsing" << std::endl;

}

Lexer::~Lexer() { delete this->head; }

void Lexer::push(Token *token) {
  if (this->head->is(TokenType::None)) {
    this->head = token;
    this->last = this->head;
    return;
  }

  this->last->next = token;
  this->last = token;
}
} // namespace tokenizer
