#include "memory/functiontool.h"
#include "memory/casttool.h"
#include "scheduler/scheduler.h"

#include <string>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

using namespace std;
using namespace mint;

MINT_FUNCTION(mint_process_exec, 2, cursor) {

	FunctionHelper helper(cursor, 2);

	Array::values_type args = to_array(*helper.popParameter());
	string command = to_string(*helper.popParameter());

	/// \todo build command line

	string command_line = command;
	for (SharedReference &arg : args) {
		command_line += " " + to_string(*arg);
	}
	helper.returnValue(create_number(system(command_line.c_str())));
}

MINT_FUNCTION(mint_process_fork, 0, cursor) {

	FunctionHelper helper(cursor, 0);

	helper.returnValue(create_number(fork()));
}

MINT_FUNCTION(mint_process_getpid, 0, cursor) {

	FunctionHelper helper(cursor, 0);

	helper.returnValue(create_number(getpid()));
}

MINT_FUNCTION(mint_process_waitpid, 1, cursor) {

	FunctionHelper helper(cursor, 1);

	double pid = to_number(cursor, *helper.popParameter());

	/// \todo optional additional options
	helper.returnValue(create_number(waitpid(pid, nullptr, 0)));
}
