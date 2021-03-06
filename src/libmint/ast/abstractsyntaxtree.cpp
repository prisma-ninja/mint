#include "ast/abstractsyntaxtree.h"
#include "ast/cursor.h"
#include "compiler/compiler.h"
#include "system/filestream.h"
#include "system/filesystem.h"
#include "system/error.h"
#include "threadentrypoint.h"

#include <memory>
#include <algorithm>

using namespace std;
using namespace mint;

AbstractSyntaxTree::AbstractSyntaxTree() {

}

AbstractSyntaxTree::~AbstractSyntaxTree() {

	m_cache.clear();

	for_each(m_modules.begin(), m_modules.end(), default_delete<Module>());
	m_modules.clear();

	for_each(m_debugInfos.begin(), m_debugInfos.end(), default_delete<DebugInfos>());
	m_debugInfos.clear();
}

pair<int, int> AbstractSyntaxTree::createBuiltinMethode(int type, Builtin methode) {

	auto &methodes = builtinMembers(-type);
	int offset = methodes.size();

	methodes[offset] = methode;

	return pair<int, int>(-type, offset);
}

void AbstractSyntaxTree::callBuiltinMethode(int module, int methode, Cursor *cursor) {
	builtinMembers(module)[methode](cursor);
}

Cursor *AbstractSyntaxTree::createCursor() {
	return new Cursor(this, ThreadEntryPoint::instance());
}

Cursor *AbstractSyntaxTree::createCursor(Module::Id module) {
	return new Cursor(this, getModule(module));
}

Module::Infos AbstractSyntaxTree::createModule() {

	Module::Infos infos;

	infos.id = m_modules.size();
	infos.module = new Module;
	infos.debugInfos = new DebugInfos;

	m_modules.push_back(infos.module);
	m_debugInfos.push_back(infos.debugInfos);

	return infos;
}

Module::Infos AbstractSyntaxTree::loadModule(const string &module) {

	auto it = m_cache.find(module);

	if (it == m_cache.end()) {

		string path = FileSystem::instance().getModulePath(module);
		if (path.empty()) {
			error("module '%s' not found", module.c_str());
		}

		it = m_cache.emplace(module, createModule()).first;

		FileStream stream(path);

		Compiler compiler;
		compiler.build(&stream, it->second);
	}

	return it->second;
}

Module::Infos AbstractSyntaxTree::main() {

	if (m_modules.empty()) {
		return m_cache.emplace("main", createModule()).first->second;
	}

	Module::Infos infos;

	infos.id = Module::MainId;
	infos.module = m_modules.front();
	infos.debugInfos = m_debugInfos.front();

	return infos;
}

Module *AbstractSyntaxTree::getModule(Module::Id id) {
	return m_modules[id];
}

DebugInfos *AbstractSyntaxTree::getDebugInfos(Module::Id id) {
	return m_debugInfos[id];
}

string AbstractSyntaxTree::getModuleName(const Module *module) {

	if (module == main().module) {
		return "main";
	}

	for (auto data : m_cache) {
		if (module == data.second.module) {
			return data.first;
		}
	}

	return "unknown";
}

Module::Id AbstractSyntaxTree::getModuleId(const Module *module) {

	for (auto data : m_cache) {
		if (module == data.second.module) {
			return data.second.id;
		}
	}

	if (module == main().module) {
		return Module::MainId;
	}

	return -1;
}

map<int, AbstractSyntaxTree::Builtin> &AbstractSyntaxTree::builtinMembers(int builtinModule) {

	static map<int, map<int, Builtin>> g_builtinMembers;
	return g_builtinMembers[builtinModule];
}
