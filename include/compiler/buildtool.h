#ifndef BUILD_TOOL_H
#define BUILD_TOOL_H

#include "compiler/lexer.h"
#include "ast/abstractsyntaxtree.h"
#include "memory/globaldata.h"

#include <string>
#include <stack>

namespace mint {

class MINT_EXPORT BuildContext {
public:
	BuildContext(DataStream *stream, Module::Infos data);

	Lexer lexer;
	Module::Infos data;

	void beginLoop();
	void endLoop();
	bool isInLoop() const;

	void startJumpForward();
	void loopJumpForward();
	void shiftJumpForward();
	void resolveJumpForward();

	void startJumpBackward();
	void loopJumpBackward();
	void shiftJumpBackward();
	void resolveJumpBackward();

	void startDefinition();
	bool addParameter(const std::string &symbol);
	bool setVariadic();
	bool saveParameters();
	bool addDefinitionSignature();
	void saveDefinition();
	Data *retriveDefinition();

	void startClassDescription(const std::string &name);
	void classInheritance(const std::string &parent);
	bool createMember(Reference::Flags flags, const std::string &name, Data *value = Reference::alloc<Data>());
	bool updateMember(Reference::Flags flags, const std::string &name, Data *value = Reference::alloc<Data>());
	void resolveClassDescription();

	void startCall();
	void addToCall();
	void resolveCall();

	void capture(const std::string &symbol);
	void captureAll();

	void pushNode(Node::Command command);
	void pushNode(int parameter);
	void pushNode(const char *symbol);
	void pushNode(Data *constant);

	void setModifiers(Reference::Flags flags);
	Reference::Flags getModifiers() const;

	void parse_error(const char *error_msg);

private:
	struct Definition {
		Reference *function;
		std::stack<std::string> parameters;
		std::list<std::string> capture;
		int beginOffset;
		bool variadic;
		bool capture_all;
	};

	std::stack<Definition *> m_definitions;
	std::stack<int> m_calls;

	std::stack<ClassDescription> m_classDescription;

	std::stack<std::list<size_t>> m_jumpForward;
	std::stack<size_t> m_jumpBackward;

	struct Loop {
		std::list<size_t> *forward;
		size_t *backward;
	};

	std::stack<Loop> m_loops;

	Reference::Flags m_modifiers;
};

}

#endif // BUILD_TOOL_H
