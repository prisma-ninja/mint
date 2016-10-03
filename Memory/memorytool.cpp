#include "Memory/memorytool.h"
#include "Memory/globaldata.h"
#include "Memory/casttool.h"
#include "Memory/class.h"
#include "Memory/object.h"
#include "AbstractSyntaxTree/abstractsyntaxtree.h"
#include "System/error.h"

using namespace std;

size_t get_base(AbstractSynatxTree *ast) {
	return ast->stack().size() - 1;
}

bool is_not_zero(SharedReference ref) {
	switch (ref.get().data()->format) {
	case Data::fmt_none:
	case Data::fmt_null:
		return false;
	case Data::fmt_number:
		return ((Number*)ref.get().data())->value;
	}
	return true;
}

Printer *toPrinter(SharedReference ref) {

	switch (ref.get().data()->format) {
	case Data::fmt_number:
		return new Printer((int)((Number*)ref.get().data())->value);
	case Data::fmt_object:
		if (((Object *)ref.get().data())->metadata == StringClass::instance()) {
			return new Printer(((String *)ref.get().data())->str.c_str());
		}
	default:
		/// \todo error
		break;
	}

	return nullptr;
}

void print(Printer *printer, SharedReference ref) {

	if (printer) {
		switch (ref.get().data()->format) {
		case Data::fmt_none:
			printer->printNone();
			break;
		case Data::fmt_null:
			printer->printNull();
			break;
		case Data::fmt_number:
			printer->print(((Number*)ref.get().data())->value);
			break;
		case Data::fmt_object:
			if (((Object *)ref.get().data())->metadata == StringClass::instance()) {
				printer->print(((String *)ref.get().data())->str.c_str());
			}
			else if (((Object *)ref.get().data())->metadata == ArrayClass::instance()) {
				printer->print(to_string(ref).c_str());
			}
			else if (((Object *)ref.get().data())->metadata == HashClass::instance()) {
				printer->print(to_string(ref).c_str());
			}
			else {
				printer->print(ref.get().data());
			}
			break;
		case Data::fmt_function:
			printer->printFunction();
			break;
		}
	}
}

void init_call(AbstractSynatxTree *ast) {

	if (ast->stack().back().get().data()->format == Data::fmt_object) {

		Object *object = (Object *)ast->stack().back().get().data();
		if (object->data == nullptr) {
			object->construct();

			auto it = object->metadata->members().find("new");
			if (it != object->metadata->members().end()) {

				if (it->second->value.flags() & Reference::user_hiden) {
					if (object->metadata != ast->symbols().metadata) {
						error("could not access protected member 'new' of class '%s'", object->metadata->name().c_str());
					}
				}
				else if (it->second->value.flags() & Reference::child_hiden) {
					if (it->second->owner != ast->symbols().metadata) {
						error("could not access private member 'new' of class '%s'", object->metadata->name().c_str());
					}
				}

				ast->waitingCalls().push(&object->data[it->second->offset]);
			}
			else {
				ast->waitingCalls().push(SharedReference::unique(Reference::create<None>()));
			}
		}
		else {
			auto it = object->metadata->members().find("()");
			if (it != object->metadata->members().end()) {
				ast->waitingCalls().push(&object->data[it->second->offset]);
			}
			else {
				error("class '%s' dosen't ovreload operator '()'", object->metadata->name().c_str());
			}
		}

		ast->waitingCalls().top().setMember(true);
	}
	else {
		ast->waitingCalls().push(ast->stack().back());
		ast->stack().pop_back();
	}
}

void init_parameter(AbstractSynatxTree *ast, const std::string &symbol) {

	SharedReference value = ast->stack().back();
	ast->stack().pop_back();

	if (value.get().flags() & Reference::const_value) {
		ast->symbols()[symbol].copy(value);
	}
	else {
		ast->symbols()[symbol].move(value);
	}
}

Function::mapping_type::iterator find_function_signature(AbstractSynatxTree *ast, Function::mapping_type &mapping, int signature) {

	auto it = mapping.find(signature);

	if (it != mapping.end()) {
		return it;
	}

	for (int required = 1; required <= signature; ++required) {

		it = mapping.find(-required);

		if (it != mapping.end()) {

			Iterator *va_args = Reference::alloc<Iterator>();
			va_args->construct();

			for (int i = 0; i < (signature - required); ++i) {
				va_args->ctx.push_front(ast->stack().back());
				ast->stack().pop_back();
			}

			ast->stack().push_back(SharedReference::unique(new Reference(Reference::standard, va_args)));
			return it;
		}
	}

	return it;
}

SharedReference get_symbol_reference(SymbolTable *symbols, const std::string &symbol) {

	if (Class *desc = GlobalData::instance().getClass(symbol)) {
		return SharedReference::unique(new Reference(Reference::standard, desc->makeInstance()));
	}

	auto it = GlobalData::instance().symbols().find(symbol);
	if (it != GlobalData::instance().symbols().end()) {
		return &it->second;
	}

	return &(*symbols)[symbol];
}

SharedReference get_object_member(AbstractSynatxTree *ast, const std::string &member) {

	Reference *result = nullptr;
	Reference &lvalue = ast->stack().back().get();

	if (lvalue.data()->format != Data::fmt_object) {
		error("non class values dosen't have member '%s'", member.c_str());
	}

	Object *object = (Object *)lvalue.data();

	if (Class *desc = object->metadata->globals().getClass(member)) {
		return SharedReference::unique(new Reference(Reference::standard, desc->makeInstance()));
	}

	auto it_global = object->metadata->globals().members().find(member);
	if (it_global != object->metadata->globals().members().end()) {

		result = &it_global->second->value;

		if (result->flags() & Reference::user_hiden) {
			if (object->metadata != ast->symbols().metadata) {
				error("could not access protected member '%s' of class '%s'", member.c_str(), object->metadata->name().c_str());
			}
		}
		else if (result->flags() & Reference::child_hiden) {
			if (it_global->second->owner != ast->symbols().metadata) {
				error("could not access private member '%s' of class '%s'", member.c_str(), object->metadata->name().c_str());
			}
		}

		return result;
	}

	if (object->data == nullptr) {
		error("class '%s' has no global member '%s'", object->metadata->name().c_str(), member.c_str());
	}

	auto it_member = object->metadata->members().find(member);
	if (it_member == object->metadata->members().end()) {
		error("class '%s' has no member '%s'", object->metadata->name().c_str(), member.c_str());
	}

	result = &object->data[it_member->second->offset];

	if (result->flags() & Reference::user_hiden) {
		if (object->metadata != ast->symbols().metadata) {
			error("could not access protected member '%s' of class '%s'", member.c_str(), object->metadata->name().c_str());
		}
	}
	else if (result->flags() & Reference::child_hiden) {
		if (it_member->second->owner != ast->symbols().metadata) {
			error("could not access private member '%s' of class '%s'", member.c_str(), object->metadata->name().c_str());
		}
	}

	return result;
}

void reduce_member(AbstractSynatxTree *ast) {

	SharedReference member = ast->stack().back();
	ast->stack().pop_back();
	ast->stack().pop_back();
	ast->stack().push_back(member);
}

string var_symbol(AbstractSynatxTree *ast) {

	Reference var = ast->stack().back();
	ast->stack().pop_back();

	return to_string(var);
}

void create_symbol(AbstractSynatxTree *ast, const std::string &symbol, Reference::Flags flags) {

	if (flags & Reference::global) {

		auto result = GlobalData::instance().symbols().insert({symbol, Reference(flags)});

		if (!result.second) {
			error("symbol '%s' was already defined in global context", symbol.c_str());
		}
		ast->stack().push_back(&result.first->second);
	}
	else {

		auto result = ast->symbols().insert({symbol, Reference(flags)});

		if (!result.second) {
			error("symbol '%s' was already defined in this context", symbol.c_str());
		}
		ast->stack().push_back(&result.first->second);
	}
}

void array_insert(AbstractSynatxTree *ast) {

	size_t base = get_base(ast);

	Reference &value = ast->stack().at(base).get();
	Reference &array = ast->stack().at(base - 1).get();

	((Array *)array.data())->values.push_back(unique_ptr<Reference>(new Reference(value)));
	ast->stack().pop_back();
}

void hash_insert(AbstractSynatxTree *ast) {

	size_t base = get_base(ast);

	Reference &value = ast->stack().at(base).get();
	Reference &key = ast->stack().at(base - 1).get();
	Reference &hash = ast->stack().at(base - 2).get();

	Reference keyItem;
	if (key.flags() & Reference::const_value) {
		keyItem.copy(key);
	}
	else {
		keyItem.move(key);
	}

	Reference valueItem;
	if (value.flags() & Reference::const_value) {
		valueItem.copy(value);
	}
	else {
		valueItem.move(value);
	}

	((Hash *)hash.data())->values.insert({keyItem, valueItem});
	ast->stack().pop_back();
	ast->stack().pop_back();
}
