#pragma once

# include <iomanip>
# include <iostream>
# include <sstream>
# include <string>
# include "Color.hpp"

enum e_token_kind {
	kTokenKindInit,
	kTokenKindBraces,
	kTokenKindBlockName,
	kTokenKindBlockParam,
	kTokenKindDirectiveName,
	kTokenKindDirectiveParam,
	kTokenKindSemicolin,
	kTokenKindComment,
	kTokenKindLineFeed,
	kTokenKindError
};

enum e_param_type {
	kParamTypeInit,
	kParamTypeBlock,
	kParamTypeDirective
};

struct Token {
 public:
	Token();
	Token(const std::string &str,
		  e_token_kind kind,
		  std::size_t line_number);
	Token(const Token &other);
	~Token();

	Token &operator=(const Token &rhs);

	static std::string get_token_kind_str(e_token_kind kind);
	static std::string get_token_output(const Token &token, bool with_color);

	std::string str_;
	e_token_kind kind_;
	std::size_t line_number_;
};

std::ostream &operator<<(std::ostream &out, const Token &token);
