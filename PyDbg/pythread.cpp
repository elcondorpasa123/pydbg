#include "stdafx.h"
#include "dbghelper.h"
#include "pyenv.h"
#include "pyobjimpl.h"
#include "pycmd.h"
#include "pyobject.h"
#include "pyframe.h"

const int c_n_framesize_per_inc = 20;
const int c_n_framesize_start = 50;
HRESULT CALLBACK DumpCStack()
{
	HRESULT hr;
	PDEBUG_STACK_FRAME frames = NULL;
	int count = c_n_framesize_start;
	frames = new DEBUG_STACK_FRAME[count];

	do 
	{
		ULONG filled;
		if ((hr = g_pDebugControl->
			GetStackTrace(0, 0, 0,
			frames, count, &filled)) != S_OK)
		{
			myprint("GetStackTrace failed, 0x%X\n", hr);
		}

		if (filled <= count)
		{
			break;
		}

		delete[] frames;
		count += c_n_framesize_per_inc;
		frames = new DEBUG_STACK_FRAME[count];
	} while (TRUE);

	// Print the call stack.
	if ((hr = g_pDebugControl->
		OutputStackTrace(DEBUG_OUTCTL_ALL_CLIENTS, frames,
		count, DEBUG_STACK_SOURCE_LINE |
		DEBUG_STACK_FRAME_ADDRESSES |
		DEBUG_STACK_COLUMN_NAMES |
		DEBUG_STACK_PARAMETERS |
		DEBUG_STACK_FRAME_NUMBERS)) != S_OK)
	{
		myprint("OutputStackTrace failed, 0x%X\n", hr);
	}

	delete[] frames;
	return S_OK;
}

#define c_str_python_stack_key_function  "PyEval_EvalFrameEx"

HRESULT CALLBACK DumpPyStack(ULONG threadId, ULONG threadSysId)
{
	HRESULT hr = E_FAIL;
	PDEBUG_STACK_FRAME frames = NULL;
	int count = c_n_framesize_start;
	frames = new DEBUG_STACK_FRAME[count];
	ULONG filled = 0;

	do 
	{
		filled = 0;
		if ((hr = g_pDebugControl->
			GetStackTrace(0, 0, 0,
			frames, count, &filled)) != S_OK)
		{
			myprint("GetStackTrace failed, 0x%X\n", hr);
		}

		if (filled <= count)
		{
			break;
		}

		delete[] frames;
		count += c_n_framesize_per_inc;
		frames = new DEBUG_STACK_FRAME[count];
	} while (TRUE);

	// 寻找PyEval_Frame的第一个参数
	for(ULONG u = 0; u < filled; u++)
	{
		char symName[4096];
		memset(symName, 0, 4096);
		ULONG64 displacement = 0;
		ULONG symSize = 0;
		HRESULT hr = g_pDebugSymbols->GetNameByOffset(
			frames[u].InstructionOffset, symName, 4096, 
			&symSize, &displacement);
		if (FAILED(hr) || 0 == symSize)
			continue;

		char* func = strchr(symName, '!');
		if (!func)
			continue;

		if (0 == strcmp(func+1, c_str_python_stack_key_function))
		{
/*			myprint("It's a python thread. %d:%d\n", threadId, threadSysId);*/
			pyframe_impl(frames[u].Params[0]);
			break;
		}
	}
	delete[] frames;
	return S_OK;
}

bool pythread_splitargs(char const* arg, ULONG& threadId, ULONG& threadSysId)
{
	if (!arg || !*arg)
		return true;

	bool err = false;
	std::vector<std::string> vecArg(10, std::string(""));
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
			while(*arg && (*arg != ' ')){vecArg[idx] += *arg;arg++;}
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
		return false;

	if(idx > 0)
	{
		threadId = atoi(vecArg[0].c_str());
	}
	if(idx > 1)
	{
		threadSysId = atoi(vecArg[1].c_str());
	}
	return true;
}

BOOL is_pythonthread(ULONG threadId, ULONG threadSysId)
{
	DWORD curThreadId  = 0;
	g_pDebugSystemObjects->GetCurrentThreadId(&curThreadId);

	g_pDebugSystemObjects->SetCurrentThreadId(threadId);

	BOOL isPyThread = FALSE;
	HRESULT hr = E_FAIL;
	PDEBUG_STACK_FRAME frames = NULL;
	int count = c_n_framesize_start;
	frames = new DEBUG_STACK_FRAME[count];
	ULONG filled = 0;

	do 
	{
		filled = 0;
		if ((hr = g_pDebugControl->
			GetStackTrace(0, 0, 0,
			frames, count, &filled)) != S_OK)
		{
			myprint("GetStackTrace failed, 0x%X\n", hr);
		}

		if (filled <= count)
		{
			break;
		}

		delete[] frames;
		count += c_n_framesize_per_inc;
		frames = new DEBUG_STACK_FRAME[count];
	} while (TRUE);

	for(ULONG u = 0; u < filled; u++)
	{
		char symName[4096];
		memset(symName, 0, 4096);
		ULONG64 displacement = 0;
		ULONG symSize = 0;
		HRESULT hr = g_pDebugSymbols->GetNameByOffset(
			frames[u].InstructionOffset, symName, 4096, 
			&symSize, &displacement);
		if (FAILED(hr) || 0 == symName)
			continue;

		char* func = strchr(symName, '!');
		if (!func)
			continue;

		if (0 == strcmp(func+1, c_str_python_stack_key_function))
		{
			isPyThread = TRUE;
			break;
		}
	}
	delete[] frames;

	g_pDebugSystemObjects->SetCurrentThreadId(curThreadId);
	return isPyThread;
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

	DumpPyStack(threadId, threadSysId);
	return S_OK;
}

HRESULT pythread_all_dml()
{
	// 1. 输出当前python堆栈
	g_pDebugControl->ControlledOutput(DEBUG_OUTCTL_AMBIENT_DML,
		DEBUG_OUTPUT_NORMAL, "<b>current Thread</b>\n");
	ULONG curThreadId = 0;
	ULONG curThreadSysId = 0;
	g_pDebugSystemObjects->GetCurrentThreadId(&curThreadId);
	g_pDebugSystemObjects->GetCurrentThreadSystemId(&curThreadSysId);
	DumpPyStack(curThreadId, curThreadSysId);

	// 2. 其他堆栈以DML格式输出
	g_pDebugControl->ControlledOutput(DEBUG_OUTCTL_AMBIENT_DML,
		DEBUG_OUTPUT_NORMAL, "<b>other Threads</b>\n");
	HRESULT hr = E_FAIL;
	ULONG threadNumbers = 0;
	g_pDebugSystemObjects->GetNumberThreads(&threadNumbers);

	ULONG*  threadIds = new ULONG[threadNumbers];
	ULONG*  threadSysIds = new ULONG[threadNumbers];
	g_pDebugSystemObjects->GetThreadIdsByIndex(0, threadNumbers, threadIds, threadSysIds);

	for(ULONG u = 0; u < threadNumbers; u++)
	{
		ULONG threadId = threadIds[u];
		ULONG threadSysId = threadSysIds[u];
		if ((threadId == curThreadId) && (threadSysId == curThreadSysId))
			continue;

		if (!is_pythonthread(threadId, threadSysId))
			continue;

		std::string dmlCmd = strfmt("!pythread %d %d", threadId, threadSysId);
		std::string dmlLink = strfmt("<exec cmd=\"%s\">%d:%d</exec>\n", dmlCmd.c_str(), threadId, threadSysId);
		g_pDebugControl->ControlledOutput(DEBUG_OUTCTL_AMBIENT_DML,
			DEBUG_OUTPUT_NORMAL, dmlLink.c_str());
	}
	delete threadIds;
	delete threadSysIds;
	return S_OK;
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
	if (!pythread_splitargs(args, threadId, threadSysId))
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

REGCMD(pythread, "<b>!pythread [threadid] [threadsysid]</b>\n"
	"\t\tOutput Python Threads. \n"
	"\t\tthreadid: threadid in windbg. \n"
	"\t\tthreadsysid: threadid in windows. \n"
	"\t\tno args means Output current thread in detail and other threads in DML.");