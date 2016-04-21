#include "modul.h"
#include <cstring>

using namespace std;

map<string, Modul> Modul::cache;

Instruction &Modul::at(uint idx) {
	return m_data[idx];
}

Modul::Modul() {

}

Modul::~Modul() {

	for (auto symbol : m_symbols) {
		delete [] symbol;
	}

	for (auto constant : m_constants) {
		delete constant;
	}
}

char *Modul::makeSymbol(const char *name) {

	for (char *symbol : m_symbols) {
		if (!strcmp(symbol, name)) {
			return symbol;
		}
	}

	char *symbol = new char [strlen(name) + 1];
	strcpy(symbol, name);
	m_symbols.push_back(symbol);
	return symbol;
}

Reference *Modul::makeConstant(Data *data) {

	Reference *constant = new Reference(Reference::const_ref | Reference::const_value, data);
	m_constants.push_back(constant);
	return constant;
}

void Modul::pushInstruction(const Instruction &instruction) {
	m_data.push_back(instruction);
}

void Modul::replaceInstruction(size_t offset, const Instruction &instruction) {
	m_data[offset] = instruction;
}

size_t Modul::nextInstructionOffset() const {
	return m_data.size();
}
