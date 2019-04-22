#pragma once

void register_cmd(char const* cmdDesc);
HRESULT CALLBACK help(PDEBUG_CLIENT4 Client, PCSTR args);

class CRegCmd
{
public:
	CRegCmd(const char* cmd);
};

#define REGCMD(__cmd__, __desc__) CRegCmd cmd_##__cmd__(__desc__)