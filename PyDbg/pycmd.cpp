#include "stdafx.h"
#include "pycmd.h"
#include "dbghelper.h"

std::vector<std::string> AllCmds;
void register_cmd(char const* cmdDesc)
{
	AllCmds.push_back(cmdDesc);
}

CRegCmd::CRegCmd(const char* cmd)
{
	register_cmd(cmd);
}

HRESULT CALLBACK help(PDEBUG_CLIENT4 Client, PCSTR args)
{
	std::string content;
	content.append("Help for PyDbg.dll\n");
	std::vector<std::string>::iterator iter = AllCmds.begin(); 
	for(; iter != AllCmds.end(); iter++)
	{
		content.append("\t");
		content.append(*iter);
		content.append("\n");
	}
	dprintf(content.c_str());
// 	dprintf("Help for PyDbg.dll\n"
// 		"	help	- Shows help\n"
// 		"	pyobj <addr> [-r[depth]]	- Dump PyObject at given address. r: recursion-level\n"
// 		"	pyframe <addr> [-r[depth]]	- Analyze Python StackFrame.\n"
// 		"	pythread [threadid] [threadsysid] - Output Python Threads.\n"
// 		);
	return S_OK;
}
