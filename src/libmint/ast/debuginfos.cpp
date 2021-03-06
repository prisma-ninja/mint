#include "ast/debuginfos.h"
#include "ast/module.h"

using namespace std;
using namespace mint;

void DebugInfos::newLine(Module *module, size_t lineNumber) {
	m_lines.emplace(module->nextNodeOffset(), lineNumber);
}

size_t DebugInfos::lineNumber(size_t offset) {

	auto line = m_lines.lower_bound(offset);

	if (line == m_lines.end()) {
		line--;
		return line->second + 1;
	}

	if (line->first != offset) {
		if (line != m_lines.begin()) {
			line--;
		}
	}

	return line->second;
}
