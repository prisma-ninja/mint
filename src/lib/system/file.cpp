#include "ast/abstractsyntaxtree.h"
#include "memory/builtin/string.h"
#include "memory/builtin/libobject.h"
#include "memory/memorytool.h"
#include "memory/casttool.h"
#include "system/utf8iterator.h"

#include <stdio.h>

using namespace std;

extern "C" {

void mint_file_fopen_2(AbstractSyntaxTree *ast) {

	size_t base = get_base(ast);

	string path = to_string(*ast->stack().at(base - 1));
	string mode = to_string(*ast->stack().at(base));

	Reference *file = Reference::create<LibObject<FILE>>();
	((LibObject<FILE> *)file->data())->impl = fopen(path.c_str(), mode.c_str());

	ast->stack().pop_back();
	ast->stack().pop_back();

	if (((LibObject<FILE> *)file->data())->impl) {
		((Object *)file->data())->construct();
		ast->stack().push_back(SharedReference::unique(file));
	}
	else {
		ast->stack().push_back(SharedReference::unique(Reference::create<Null>()));
	}
}

void mint_file_fclose_1(AbstractSyntaxTree *ast) {

	Reference &file = *ast->stack().back();

	if (((LibObject<FILE> *)file.data())->impl) {
		fclose(((LibObject<FILE> *)file.data())->impl);
		((LibObject<FILE> *)file.data())->impl = nullptr;
	}
}

void mint_file_fgetc_1(AbstractSyntaxTree *ast) {

	Reference &file = *ast->stack().back();

	int cptr = fgetc(((LibObject<FILE> *)file.data())->impl);

	if (cptr == EOF) {
		ast->stack().push_back(SharedReference());
	}
	else {
		Reference *result = Reference::create<String>();
		((Object *)result->data())->construct();
		((String *)result->data())->str = cptr;
		size_t length = utf8char_length(cptr);
		while (--length) {
			((String *)result->data())->str += cptr;
		}
		ast->stack().push_back(SharedReference::unique(result));
	}
}

void mint_file_readline_1(AbstractSyntaxTree *ast) {

	Reference &file = *ast->stack().back();

	int cptr = fgetc(((LibObject<FILE> *)file.data())->impl);
	Reference *result = Reference::create<String>();

	while ((cptr != '\n') && (cptr != EOF)) {
		((String *)result->data())->str += cptr;
		cptr = fgetc(((LibObject<FILE> *)file.data())->impl);
	}

	ast->stack().pop_back();

	if (cptr == EOF) {
		ast->stack().push_back(SharedReference());
	}
	else {
		((Object *)result->data())->construct();
		ast->stack().push_back(SharedReference::unique(result));
	}
}

}
