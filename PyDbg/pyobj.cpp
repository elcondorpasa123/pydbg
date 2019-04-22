#include "stdafx.h"
#include "dbghelper.h"
#include "pyenv.h"
#include "pyobjimpl.h"
#include "pycmd.h"
#include "pyobject.h"

int hexstr2int(char const* in)
{
	if (!in)
		return 0;

	int out = 0;
	byte c = *in;
	while (*in)
	{
		byte c = *in;
		if (c >= '0' && c <= '9')
		{
			c = c - '0';
		}
		else if (c >= 'a' && c <= 'f')
		{
			c = c - 'a' + 10;
		}
		else if (c >= 'A' && c <= 'F')
		{
			c = c - 'A' + 10;
		}
		else
		{
			break;
		}
		out = out *16 + c; 
		++in;
	}
	return out;
}

bool pyobj_splitargs(char const* arg, DWORD& addr_, int& depth_)
{
	if (!arg || !*arg)
		return false;

	bool err = false;
	std::string addr;
	std::string depth;
	do 
	{
		switch (*arg)
		{
		case '0':
			arg++;
			if ((*arg != 'x') && (*arg != 'X'))
			{
				err = true;
				break;
			}
			arg++;
			while(*arg && (*arg != ' ')){addr += *arg;arg++;}
			break;
		case '-':
			arg++;
			switch (*arg)
			{
			case 'r':
				arg++;
				while(*arg && (*arg != ' ')){depth += *arg;arg++;}
				break;
			default:
				err = true;
				break;
			}
			break;
		case ' ':
			arg++;
			break;
		default:
			err = true;
		}

		if (err)
			break;
	} while (*arg);

	if (!err)
	{
		addr_ = addr.empty() ? 0 : hexstr2int(addr.c_str());
		depth_ = depth.empty() ? 0 : atoi(depth.c_str());
	}
	return !err;
}

HRESULT CALLBACK pyobj(PDEBUG_CLIENT4 Client, PCSTR args)
{
	if (!g_py_env.ready())
	{
		myprint("不支持的python版本");
		return E_FAIL;
	}

	HRESULT hr = S_OK;

	DWORD addr = 0;
	int depth = 0;
	pyobj_splitargs(args, addr, depth);
	if (addr == 0)
	{
		dprintf("invalid argument. \n try !pyOjb 0xXXXXXXXX -r{depth}\n");
		return E_INVALIDARG;
	}

	if (depth)
	{
		setnestlevel(depth);
	}

	dbg_pyobject(addr);
	resetnestlevel();
	return S_OK;
}

REGCMD(pyobj, "<b>!pyobj <addr> [-r depth]</b>\n"
	"\t\tDump PyObject at given address. \n"
	"\t\t r: recursion-level.");
