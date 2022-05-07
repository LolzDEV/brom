#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>

namespace tokenizer {

enum TokenType {
  None,
  I32Literal,
  LParen,
  RParen,
  Plus,
  Minus,
  Star,
  Slash,
  Identifier,
  Let,
  Equal,
  SemiColon,
  Fn,
  Type,
  LCurly,
  RCurly,
  Ret,
  RightArrow,
  Colon,
  Comma
};

class Token {
public:
  std::string lexeme;
  bool is(TokenType type);
  Token *get_next();
  Token(TokenType type, std::string lexeme);
  Token *next;
  TokenType type;
};

} // namespace tokenizer
#endif // TOKEN_H_
