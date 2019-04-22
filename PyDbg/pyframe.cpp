#include "stdafx.h"
#include "dbghelper.h"
#include "pyenv.h"
#include "pyobjimpl.h"
#include "pycmd.h"
#include "pyobject.h"

void pyframe_impl(ULONG_PTR Address)
{
	PPyFrameObject pyFrameObj;
	if (!pyFrameObj.valid())
		return;

	if (!pyFrameObj.read(Address))
	{
		myprint("fail to read pyframeobject  at 0x%08x:\n", Address);
		return;
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

		// 		myprintex(1, "line: %d\n", pyCodeObj.get("co_firstlineno"));
		myprintex(0, "line:%*d", 4, pyFrameObj.get("f_lineno"));
		dbg_pyobject((ULONG_PTR)pyCodeObj.get("co_filename"), 1, 0);

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
	return;
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

	pyframe_impl(Address);
	return S_OK;
}

REGCMD(pyframe, "<b>!pyframe <addr></b>\n"
	"\t\tAnalyze Python StackFrame.");