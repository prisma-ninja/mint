#include "lexer.h"
#include "Compiler/parser.hpp"

using namespace std;
using namespace yy;

map<string, int> Lexer::keywords = {
	{"assert", parser::token::assert_token},
	{"break", parser::token::break_token},
	{"catch", parser::token::catch_token},
	{"class", parser::token::class_token},
	{"continue", parser::token::continue_token},
	{"def", parser::token::def_token},
	{"defined", parser::token::defined_token},
	{"elif", parser::token::elif_token},
	{"else", parser::token::else_token},
	{"exec", parser::token::exec_token},
	{"for", parser::token::for_token},
	{"if", parser::token::if_token},
	{"in", parser::token::in_token},
	{"is", parser::token::is_token},
	{"load", parser::token::load_token},
	{"membersof", parser::token::membersof_token},
	{"print", parser::token::print_token},
	{"raise", parser::token::raise_token},
	{"return", parser::token::return_token},
	{"try", parser::token::try_token},
	{"while", parser::token::while_token},
	{"yield", parser::token::yield_token}
};

map<string, int> Lexer::operators = {
	{"+", parser::token::plus_token},
	{"-", parser::token::minus_token},
	{"*", parser::token::asterisk_token},
	{"/", parser::token::slash_token},
	{"%", parser::token::percent_token},
	{"=", parser::token::equal_token},
	{":", parser::token::dbldot_token},
	{".", parser::token::dot_token},
	{",", parser::token::comma_token},
	{"(", parser::token::open_parenthesis_token},
	{")", parser::token::close_parenthesis_token},
	{"[", parser::token::open_bracket_token},
	{"]", parser::token::close_bracket_token},
	{"{", parser::token::open_brace_token},
	{"}", parser::token::close_brace_token},
	{"++", parser::token::dbl_plus_token},
	{"--", parser::token::dbl_minus_token},
	{"**", parser::token::dbl_asterisk_token},
	{"//", parser::token::dbl_slash_token},
	{"==", parser::token::dbl_equal_token},
	{":=", parser::token::dbldot_equal_token},
	{";", parser::token::line_end_token},
	{"\n", parser::token::line_end_token},
};

Lexer::Lexer(DataStream *stream) : m_stream(stream), m_cptr(0) {}

string Lexer::nextToken() {

	while (isWhiteSpace(m_cptr)) {
		m_cptr = m_stream->getChar();
	}

	bool findOperator = isOperator(string() + (char)m_cptr);
	string token;

	if (m_cptr == '\'' || m_cptr == '"') {
		return tokenizeString(m_cptr);
	}

	while (!isWhiteSpace(m_cptr)) {
		if (findOperator) {
			if (isOperator(token + (char)m_cptr)) {
				token += (char)m_cptr;
			}
			else {
				break;
			}
		}
		else {
			if (isOperator(string() + (char)m_cptr) || isWhiteSpace(m_cptr)) {
				break;
			}
			else {
				token += (char)m_cptr;
			}
		}
		m_cptr = m_stream->getChar();
	}

	if (m_cptr == '.') {
		for (char c : token) {
			if (!isdigit(c)) {
				return token;
			}
		}
		do {
			token += m_cptr;
		} while (isdigit(m_cptr = m_stream->getChar()));
	}

	/// \todo tokenize comments

	return token;
}

int Lexer::tokenType(const string &token) {

	auto it = keywords.find(token);
	if (it != keywords.end()) {
		return it->second;
	}

	it = operators.find(token);
	if (it != operators.end()) {
		return it->second;
	}

	/// \todo constants
	if (isdigit(token.front())) { // test
		return parser::token::constant_token;
	}
	else if (token.front() == '\'' || token.front() == '"') {
		return parser::token::constant_token;
	}

	return parser::token::symbol_token;
}

size_t Lexer::lineNumber() const {
	return m_stream->lineNumber();
}

bool Lexer::atEnd() const {
	return m_stream->atEnd();
}

bool Lexer::isWhiteSpace(char c) {
	return c <= ' ' && c != '\n';
}

bool Lexer::isOperator(const std::string &token) {
	return operators.find(token) != operators.end();
}

string Lexer::tokenizeString(char delim) {

	string token;
	bool shift = false;

	do {
		token += m_cptr;
	} while (((m_cptr = m_stream->getChar()) != delim) || (shift = ((m_cptr == '\\') && !shift)));
	token += m_cptr;

	m_cptr = m_stream->getChar();
	return token;
}
