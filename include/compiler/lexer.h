#ifndef LEXER_H
#define LEXER_H

#include "system/datastream.h"

#include <map>

namespace mint {

class MINT_EXPORT Lexer {
public:
	Lexer(DataStream *stream);

	std::string nextToken();
	int tokenType(const std::string &token);

	std::string formatError(const char *error) const;
	bool atEnd() const;

protected:
	bool isWhiteSpace(char c);
	bool isOperator(const std::string &token);

	std::string tokenizeString(char delim);

private:
	static const std::map<std::string, int> keywords;
	static const std::map<std::string, int> operators;

	DataStream *m_stream;
	int m_cptr;
	int m_remaining; // hack
};

}

#endif // LEXER_H
