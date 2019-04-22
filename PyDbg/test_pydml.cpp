#include "stdafx.h"
#include "dbghelper.h"
#include "pyenv.h"
#include "pyobjimpl.h"
#include "pycmd.h"
#include "pyobject.h"
// .logopen tmp.log;~* kvn;.logclose

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
