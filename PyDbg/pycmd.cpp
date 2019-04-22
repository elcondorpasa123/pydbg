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
		"	pythread [-s]- Output All Python Threads. s: in simple format\n"
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


void pyobj_splitargs(char const* arg, DWORD& addr_, int& depth_)
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

ULONG64 g_TraceFrom[3];

// .logopen tmp.log;~* kvn;.logclose
extern IDebugControl* g_pDebugControl;
extern IDebugClient* g_pDebugClient;
extern IDebugSymbols* g_pDebugSymbols;
extern IDebugSystemObjects* g_pDebugSystemObjects;

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

HRESULT ifdml()
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

HRESULT test_dml()
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
}

const int c_n_framesize_per_inc = 20;
const int c_n_framesize_start = 50;
HRESULT CALLBACK DumpCStack()
{
	HRESULT Status;
	PDEBUG_STACK_FRAME Frames = NULL;
	int Count = c_n_framesize_start;
	Frames = new DEBUG_STACK_FRAME[Count];

	do 
	{
		ULONG Filled;
		if ((Status = g_pDebugControl->
			GetStackTrace(0, 0, 0,
			Frames, Count, &Filled)) != S_OK)
		{
			myprint("GetStackTrace failed, 0x%X\n", Status);
		}

		if (Filled <= Count)
		{
			break;
		}

		delete[] Frames;
		Count += c_n_framesize_per_inc;
		Frames = new DEBUG_STACK_FRAME[Count];
	} while (TRUE);

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

#define c_str_python_stack_key_function  "PyEval_EvalFrameEx"

HRESULT CALLBACK DumpPyStack(ULONG threadId, ULONG threadSysId)
{
	HRESULT Status = E_FAIL;
	PDEBUG_STACK_FRAME Frames = NULL;
	int Count = c_n_framesize_start;
	Frames = new DEBUG_STACK_FRAME[Count];
	ULONG Filled = 0;

	do 
	{
		Filled = 0;
		if ((Status = g_pDebugControl->
			GetStackTrace(0, 0, 0,
			Frames, Count, &Filled)) != S_OK)
		{
			myprint("GetStackTrace failed, 0x%X\n", Status);
		}

		if (Filled <= Count)
		{
			break;
		}

		delete[] Frames;
		Count += c_n_framesize_per_inc;
		Frames = new DEBUG_STACK_FRAME[Count];
	} while (TRUE);

	// 寻找PyEval_Frame的第一个参数
	for(ULONG u = 0; u < Filled; u++)
	{
		char SymName[4096];
		memset(SymName, 0, 4096);
		ULONG64 Displacement = 0;
		ULONG SymSize = 0;
		HRESULT hr = g_pDebugSymbols->GetNameByOffset(
			Frames[u].InstructionOffset, SymName, 4096, 
			&SymSize, &Displacement);
		if (FAILED(hr) || 0 == SymSize)
			continue;

		char* func = strchr(SymName, '!');
		if (!func)
			continue;

		if (0 == strcmp(func+1, c_str_python_stack_key_function))
		{
			myprint("It's a python thread. %d:%d\n", threadId, threadSysId);
			pyframe_impl(Frames[u].Params[0]);
			break;
		}
	}
	delete[] Frames;
	return S_OK;
}

BOOL pythread_splitargs(char const* arg, ULONG& threadId, ULONG& threadSysId)
{
	if (!arg || !*arg)
		return TRUE;

	bool err = false;
	std::vector<std::string> strVec(10, std::string(""));
	int idx = 0;
	do 
	{
		switch (*arg)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			while(*arg && (*arg != ' ')){strVec[idx] += *arg;arg++;}
			idx++;
			break;
		case ' ':
			arg++;
			break;
		case '\0':
			break;
		default:
			err = true;
		}

		if (err)
			break;
	} while (*arg);

	if (err)
		return FALSE;

	if(idx > 0)
	{
		threadId = atoi(strVec[0].c_str());
	}
	if(idx > 1)
	{
		threadSysId = atoi(strVec[1].c_str());
	}
	return true;
}

BOOL pythread_is_pythonthread(ULONG threadId, ULONG threadSysId)
{
	DWORD curThreadId  = 0;
	g_pDebugSystemObjects->GetCurrentThreadId(&curThreadId);

	g_pDebugSystemObjects->SetCurrentThreadId(threadId);

	BOOL isPyThread = FALSE;
	HRESULT Status = E_FAIL;
	PDEBUG_STACK_FRAME Frames = NULL;
	int Count = c_n_framesize_start;
	Frames = new DEBUG_STACK_FRAME[Count];
	ULONG Filled = 0;

	do 
	{
		Filled = 0;
		if ((Status = g_pDebugControl->
			GetStackTrace(0, 0, 0,
			Frames, Count, &Filled)) != S_OK)
		{
			myprint("GetStackTrace failed, 0x%X\n", Status);
		}

		if (Filled <= Count)
		{
			break;
		}

		delete[] Frames;
		Count += c_n_framesize_per_inc;
		Frames = new DEBUG_STACK_FRAME[Count];
	} while (TRUE);

	// 寻找PyEval_Frame的第一个参数
	for(ULONG u = 0; u < Filled; u++)
	{
		char SymName[4096];
		memset(SymName, 0, 4096);
		ULONG64 Displacement = 0;
		ULONG SymSize = 0;
		HRESULT hr = g_pDebugSymbols->GetNameByOffset(
			Frames[u].InstructionOffset, SymName, 4096, 
			&SymSize, &Displacement);
		if (FAILED(hr) || 0 == SymSize)
			continue;

		char* func = strchr(SymName, '!');
		if (!func)
			continue;

		if (0 == strcmp(func+1, c_str_python_stack_key_function))
		{
/*			myprint("It's a python thread. %d:%d\n", threadId, threadSysId);*/
			isPyThread = TRUE;
/*			pyframe_impl(Frames[u].Params[0]);*/
			break;
		}
	}
	delete[] Frames;

	g_pDebugSystemObjects->SetCurrentThreadId(curThreadId);
	return isPyThread;
}

HRESULT CALLBACK pythread(PDEBUG_CLIENT4 Client, PCSTR args)
{
	if (!g_py_env.ready())
	{
		myprint("不支持的python版本");
		return E_FAIL;
	}

	ULONG threadId = 0;
	ULONG threadSysId = 0;
	BOOL argsGood = pythread_splitargs(args, threadId, threadSysId);
	if (!argsGood)
	{
		dprintf("invalid argument. \n try !pythread [-s]\n");
		return E_INVALIDARG;
	}

	if (!threadId && !threadSysId)
	{
		pythread_all_dml();
	}
	else
	{
		pythread_single(threadId, threadSysId);
	}
	return S_OK;
}

HRESULT pythread_single(ULONG threadId, ULONG threadSysId)
{
	HRESULT hr = g_pDebugSystemObjects->SetCurrentThreadId(threadId);
	ULONG curThreadId;
	g_pDebugSystemObjects->GetCurrentThreadId(&curThreadId);
	if (FAILED(hr) || curThreadId != threadId)
	{
		myprint("switch to thread %d fail", threadId);
		return E_FAIL;
	}

	//DumpCStack();
	DumpPyStack(threadId, threadSysId);
	return S_OK;
}

HRESULT pythread_all_dml()
{
	// 输出当前python堆栈
	ULONG curThreadId = 0;
	ULONG curThreadSysId = 0;
	g_pDebugSystemObjects->GetCurrentThreadId(&curThreadId);
	g_pDebugSystemObjects->GetCurrentThreadSystemId(&curThreadSysId);
	DumpPyStack(curThreadId, curThreadSysId);

	// 其他堆栈以DML格式输出
	HRESULT hr = E_FAIL;
	ULONG threadNumbers = 0;
	g_pDebugSystemObjects->GetNumberThreads(&threadNumbers);
	// myprint("GetNumberThreads: 0x%X\n", threadNumbers);

	ULONG*  threadsIds = new ULONG[threadNumbers];
	ULONG*  threadsSysIds = new ULONG[threadNumbers];
	g_pDebugSystemObjects->GetThreadIdsByIndex(0, threadNumbers, threadsIds, threadsSysIds);

	for(ULONG u = 0; u < threadNumbers; u++)
	{
		if ((threadsIds[u] == curThreadId) && (threadsSysIds[u] == curThreadSysId))
			continue;

		if (FALSE == pythread_is_pythonthread(threadsIds[u], threadsSysIds[u]))
			continue;

		std::string dmlCmd = fmt("!pythread %d %d", threadsIds[u], threadsSysIds[u]);
		std::string dmlLink = fmt("<exec cmd=\"%s\">%d:%d</exec>\n", dmlCmd.c_str(), threadsIds[u], threadsSysIds[u]);
		g_pDebugControl->ControlledOutput(DEBUG_OUTCTL_AMBIENT_DML,
			DEBUG_OUTPUT_NORMAL, dmlLink.c_str());
	}
	delete threadsIds;
	delete threadsSysIds;
	return S_OK;
}

HRESULT CALLBACK pyframe_impl(ULONG_PTR Address)
{
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

HRESULT CALLBACK pyframe(PDEBUG_CLIENT4 Client, PCSTR args)
{
	if (!g_py_env.ready())
	{
		myprint("不支持的python版本");
		return E_FAIL;
	}

	HRESULT hr = S_OK;
	ULONG_PTR Address = (ULONG_PTR)GetExpression(args);
	if (!Address)
		return E_INVALIDARG;

	return pyframe_impl(Address);
}