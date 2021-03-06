#include "memory/casttool.h"
#include "memory/memorytool.h"
#include "memory/builtin/string.h"
#include "ast/cursor.h"
#include "system/utf8iterator.h"
#include "system/error.h"

#include <string>

using namespace std;
using namespace mint;

string number_to_char(long number) {

	string result;

	while (number) {
		result.insert(result.begin(), number % (1 << 8));
		number = number / (1 << 8);
	}

	return result;
}

double mint::to_number(Cursor *cursor, const Reference &ref) {

	switch (ref.data()->format) {
	case Data::fmt_none:
		error("invalid use of none value in an operation");
		break;
	case Data::fmt_null:
		cursor->raise((Reference *)&ref);
		break;
	case Data::fmt_number:
		return ref.data<Number>()->value;
	case Data::fmt_boolean:
		return ref.data<Boolean>()->value;
	case Data::fmt_object:
		switch (ref.data<Object>()->metadata->metatype()) {
		case Class::string:
			if (const char *value = ref.data<String>()->str.c_str()) {

				if (value[0] == '0') {
					switch (value[1]) {
					case 'b':
					case 'B':
						return strtol(value + 2, nullptr, 2);

					case 'o':
					case 'O':
						return strtol(value + 2, nullptr, 8);

					case 'x':
					case 'X':
						return strtol(value + 2, nullptr, 16);

					default:
						break;
					}
				}

				return strtod(value, nullptr);
			}
			break;
		case Class::iterator:
			for (SharedReference item; iterator_next(const_cast<Iterator *>(ref.data<Iterator>()), item);) {
				return to_number(cursor, *item);
			}
			error("invalid use of none value in an operation");
			break;
		default:
			error("invalid conversion from '%s' to 'number'", ref.data<Object>()->metadata->name().c_str());
		}
		break;
	case Data::fmt_function:
		error("invalid conversion from 'function' to 'number'");
		break;
	}

	return 0;
}

bool mint::to_boolean(Cursor *cursor, const Reference &ref) {

	((void)cursor);

	switch (ref.data()->format) {
	case Data::fmt_none:
	case Data::fmt_null:
		return false;
	case Data::fmt_number:
		return ref.data<Number>()->value;
	case Data::fmt_boolean:
		return ref.data<Boolean>()->value;
	case Data::fmt_object:
		switch (ref.data<Object>()->metadata->metatype()) {
		case Class::iterator:
			return !ref.data<Iterator>()->ctx.empty();
		default:
			break;
		}
	default:
		break;
	}

	return true;
}

string mint::to_char(const Reference &ref) {

	switch (ref.data()->format) {
	case Data::fmt_none:
	case Data::fmt_null:
		return string();
	case Data::fmt_number:
		return number_to_char(ref.data<Number>()->value);
	case Data::fmt_boolean:
		return ref.data<Boolean>()->value ? "y" : "n";
	case Data::fmt_object:
		if (ref.data<Object>()->metadata->metatype() == Class::string) {
			return *const_utf8iterator(ref.data<String>()->str.begin());
		}
		error("invalid conversion from '%s' to 'character'", ref.data<Object>()->metadata->name().c_str());
		break;
	case Data::fmt_function:
		error("invalid conversion from 'function' to 'character'");
		break;
	}

	return string();
}

string mint::to_string(const Reference &ref) {

	switch (ref.data()->format) {
	case Data::fmt_none:
		return "(none)";
	case Data::fmt_null:
		return "(null)";
	case Data::fmt_number:
		return std::to_string(ref.data<Number>()->value);
	case Data::fmt_boolean:
		return ref.data<Boolean>()->value ? "true" : "false";
	case Data::fmt_object:
		switch (ref.data<Object>()->metadata->metatype()) {
		case Class::string:
			return ref.data<String>()->str;
		case Class::array:
			return "[" + [] (const Array::values_type &values) {
				string join;
				for (auto it = values.begin(); it != values.end(); ++it) {
					if (it != values.begin()) {
						join += ", ";
					}
					join += to_string(**it);
				}
				return join;
			} (ref.data<Array>()->values) + "]";
		case Class::hash:
			return "{" + [] (const Hash::values_type &values) {
				string join;
				for (auto it = values.begin(); it != values.end(); ++it) {
					if (it != values.begin()) {
						join += ", ";
					}
					join += to_string(*it->first);
					join += " : ";
					join += to_string(*it->second);
				}
				return join;
			} (ref.data<Hash>()->values) + "}";
		case Class::iterator:
			for (SharedReference item; iterator_next(const_cast<Iterator *>(ref.data<Iterator>()), item);) {
				return to_string(*item);
			}
			error("invalid use of none value in an operation");
			break;
		default:
			char buffer[(sizeof(void *) * 2) + 3];
			sprintf(buffer, "%p", ref.data());
			return buffer;
		}
		break;
	case Data::fmt_function:
		return "(function)";
	}

	return string();
}

Array::values_type mint::to_array(const Reference &ref) {

	Array::values_type result;

	switch (ref.data()->format) {

	case Data::fmt_object:
		switch (ref.data<Object>()->metadata->metatype()) {
		case Class::array:
			for (size_t i = 0; i < ref.data<Array>()->values.size(); ++i) {
				result.push_back(array_get_item(ref.data<Array>(), i));
			}
			return result;
		case Class::hash:
			for (auto &item : ref.data<Hash>()->values) {
				result.push_back(hash_get_key(item));
			}
			return result;
		case Class::iterator:
			for (SharedReference item; iterator_next(const_cast<Iterator *>(ref.data<Iterator>()), item);) {
				result.push_back(SharedReference::unique(new Reference(*item)));
			}
			return result;
		default:
			break;
		}
	default:
		result.push_back(SharedReference::unique(new Reference(ref)));
	}

	return result;
}

Hash::values_type mint::to_hash(Cursor *cursor, const Reference &ref) {

	Hash::values_type result;

	switch (ref.data()->format) {
	case Data::fmt_object:
		switch (ref.data<Object>()->metadata->metatype()) {
		case Class::array:
			for (size_t i = 0; i < ref.data<Array>()->values.size(); ++i) {
				Reference *index = Reference::create<Number>();
				index->data<Number>()->value = i;
				result.emplace(SharedReference::unique(index), array_get_item(ref.data<Array>(), i));
			}
			return result;
		case Class::hash:
			for (auto &item : ref.data<Hash>()->values) {
				result.emplace(hash_get_key(item), hash_get_value(item));
			}
			return result;
		case Class::iterator:
			for (SharedReference item; iterator_next(const_cast<Iterator *>(ref.data<Iterator>()), item);) {
				result.emplace(SharedReference::unique(new Reference(*item)), SharedReference());
			}
			return result;
		default:
			break;
		}
	default:
		result.emplace(SharedReference::unique(new Reference(ref)), SharedReference());
	}

	return result;
}
