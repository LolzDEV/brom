#include "token.hpp"

namespace tokenizer {

Token::Token(TokenType type, std::string lexeme) : type(type), lexeme(lexeme){};

Token *Token::get_next() { return this->next; }

bool Token::is(TokenType type) { return this->type == type; }

} // namespace tokenizer
