#include "stdafx.h"
#include "pycmd.h"
#include "dbghelper.h"

std::vector<std::string> AllCmds;

CRegCmd::CRegCmd(const char* cmd)
{
	AllCmds.push_back(cmd);
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
	g_pDebugControl->ControlledOutput(
		DEBUG_OUTCTL_AMBIENT_DML, DEBUG_OUTPUT_NORMAL, 
		content.c_str());
	return S_OK;
}
