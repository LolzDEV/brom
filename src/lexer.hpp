#ifndef LEXER_H_
#define LEXER_H_

#include "token.hpp"
#include <ctype.h>
#include <string>

namespace tokenizer {

class Lexer {
public:
  Token *head;
Token *last;
  Lexer(std::string source);
  ~Lexer();

private:
  void push(Token *token);
};

} // namespace tokenizer

#endif // LEXER_H_
