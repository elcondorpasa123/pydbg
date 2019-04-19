#include "stdafx.h"
#include "dbghelper.h"
#include "pyenv.h"
#include "pyhelper.h"
#include "pycmd.h"
#include "pyobject.h"

HRESULT CALLBACK help(PDEBUG_CLIENT4 Client, PCSTR args)
{
	dprintf("Help for PyDbg.dll\n"
		"	help	- Shows help\n"
		"	pyobj <addr> [-r[depth]]	- Dump PyObject at given address. r: recursion-level\n"
		"	pyframe <addr> [-r[depth]]	- Analyze Python StackFrame.\n"
		);
	return S_OK;
}

int hatoi(char const* in)
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


void splitargs(char const* arg, DWORD& addr_, int& depth_)
{
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
		addr_ = addr.empty() ? 0 : hatoi(addr.c_str());
		depth_ = depth.empty() ? 0 : atoi(depth.c_str());
	}
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
	splitargs(args, addr, depth);
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

ULONG64 g_TraceFrom[3];

// .logopen tmp.log;~* kvn;.logclose
extern IDebugControl* g_pDebugControl;
extern IDebugClient* g_pDebugClient;

BOOL PreferDML()
{
	BOOL bPreferDML = FALSE;
	ULONG ulOptions = 0;
	if (SUCCEEDED(g_pDebugControl->GetEngineOptions(&ulOptions)))
	{
		bPreferDML = (ulOptions & DEBUG_ENGOPT_PREFER_DML);
		g_pDebugControl->SetEngineOptions(ulOptions | DEBUG_ENGOPT_PREFER_DML);
	}
	return bPreferDML;
}


BOOL AbilityDML()
{
	BOOL bAbilityDML = FALSE;
	IDebugAdvanced2* pDebugAdvanced2;
	if (SUCCEEDED(g_pDebugClient->QueryInterface(__uuidof(IDebugAdvanced2), 
		(void **)& pDebugAdvanced2)))
	{
		HRESULT hr = 0;
		if (SUCCEEDED(hr = pDebugAdvanced2->Request(
			DEBUG_REQUEST_CURRENT_OUTPUT_CALLBACKS_ARE_DML_AWARE, 
			NULL, 0, NULL, 0, NULL)))
		{
			if (hr == S_OK) bAbilityDML = TRUE;
		}
		pDebugAdvanced2->Release();
	}
	return bAbilityDML;
}


HRESULT CALLBACK 
	ifdml()
{

		// A condition is usually not required;
		// Rely on content conversion when there isn't 
		// any abbreviation or superfluous content
		if (PreferDML() && AbilityDML())
		{
			g_pDebugControl->ControlledOutput(DEBUG_OUTCTL_AMBIENT_DML, 
				DEBUG_OUTPUT_NORMAL, "<b>Hello</b> <i>DML</i> <u>World!</u>\n");
		}
		else
		{
			g_pDebugControl->ControlledOutput(
				DEBUG_OUTCTL_AMBIENT_TEXT, DEBUG_OUTPUT_NORMAL, 
				"Hello TEXT World!\n");
		}
	return S_OK;
}

HRESULT CALLBACK DumpStack()
{
	ifdml();
	g_pDebugControl->ControlledOutput(DEBUG_OUTCTL_AMBIENT_DML,
		DEBUG_OUTPUT_NORMAL, "[Start DML]\n");
	g_pDebugControl->ControlledOutput(DEBUG_OUTCTL_AMBIENT_DML,
		DEBUG_OUTPUT_NORMAL, "<link cmd=\"!pyobj 0x06747410\">pyobj at 0x06747410</link>\n");
	g_pDebugControl->ControlledOutput(DEBUG_OUTCTL_AMBIENT_DML,
		DEBUG_OUTPUT_NORMAL, "<exec cmd=\"!pyframe 06747410\">pyframe</exec>\n");
	g_pDebugControl->ControlledOutput(DEBUG_OUTCTL_AMBIENT_DML,
		DEBUG_OUTPUT_NORMAL, "<b>Hello</b> <i>DML</i> <u>World!</u>\n");
	return S_OK;


	HRESULT Status;
	PDEBUG_STACK_FRAME Frames = NULL;
	int Count = 50;

	myprint("\nFirst %d frames of the call stack:\n", Count);

/*	if (g_TraceFrom[0] || g_TraceFrom[1] || g_TraceFrom[2])*/
	{
		ULONG Filled;

		Frames = new DEBUG_STACK_FRAME[Count];
		if (Frames == NULL)
		{
			myprint("Unable to allocate stack frames\n");
		}

		if ((Status = g_pDebugControl->
			GetStackTrace(0, 0, 0,
			Frames, Count, &Filled)) != S_OK)
		{
			myprint("GetStackTrace failed, 0x%X\n", Status);
		}

		Count = Filled;
	}

	// Print the call stack.
	if ((Status = g_pDebugControl->
		OutputStackTrace(DEBUG_OUTCTL_ALL_CLIENTS, Frames,
		Count, DEBUG_STACK_SOURCE_LINE |
		DEBUG_STACK_FRAME_ADDRESSES |
		DEBUG_STACK_COLUMN_NAMES |
		DEBUG_STACK_PARAMETERS |
		DEBUG_STACK_FRAME_NUMBERS)) != S_OK)
	{
		myprint("OutputStackTrace failed, 0x%X\n", Status);
	}

	delete[] Frames;
	return S_OK;
}

HRESULT CALLBACK pyframe(PDEBUG_CLIENT4 Client, PCSTR args)
{
	if (!g_py_env.ready())
	{
		myprint("不支持的python版本");
		return E_FAIL;
	}

	DumpStack();

	HRESULT hr = S_OK;
	ULONG_PTR Address = (ULONG_PTR)GetExpression(args);
	if (!Address)
		return E_INVALIDARG;

	PPyFrameObject pyFrameObj;
	if (!pyFrameObj.valid())
		return E_INVALIDARG;

	if (!pyFrameObj.read(Address))
	{
		myprint("fail to read pyframeobject  at 0x%08x:\n", Address);
		return E_INVALIDARG;
	}

	do 
	{
		int f_code = pyFrameObj.get("f_code");

		PPyCodeObject pyCodeObj;
		if (!pyCodeObj.read(f_code))
		{
			myprint("fail to read pycodeobject  at 0x%08x:\n", f_code);
			break;
		}

		dbg_pyobject((ULONG_PTR)pyCodeObj.get("co_filename"), 1, 0);
// 		myprintex(1, "line: %d\n", pyCodeObj.get("co_firstlineno"));
		myprintex(1, "line: %d\n", pyFrameObj.get("f_lineno"));

		int f_back = pyFrameObj.get("f_back");
		if (!f_back)
		{
			break;
		}

		pyFrameObj.clear();
		if (!pyFrameObj.read(f_back))
		{
			myprint("fail to read pyframeobject  at 0x%08x:\n", f_back);
			break;
		}
	} while (true);

	return S_OK;
}