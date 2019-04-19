#include "stdafx.h"
#include "pyenv.h"

PyEnv g_py_env;


PyEnv::PyEnv()
	: m_bReady(false)
	, m_bInit(false)
{
	ZeroMemory(m_pydll, sizeof(m_pydll));
};

PyEnv::~PyEnv()
{

};

bool PyEnv::ready()
{
	if (m_bInit)
		return m_bReady;

	m_bInit = true;
	HRESULT hr = E_FAIL;
	do 
	{
		ULONG64 pModuleBase = 0;
		hr = g_pDebugSymbols->GetModuleByModuleName("python27_d", 0, NULL, &pModuleBase);
		if (SUCCEEDED(hr))				
		{
			strcpy(m_pydll, "python27_d");
			break;
		}
		hr = g_pDebugSymbols->GetModuleByModuleName("snack276", 0, NULL, &pModuleBase);
		if (SUCCEEDED(hr))				
		{
			strcpy(m_pydll, "snack276");
			break;
		}
		hr = g_pDebugSymbols->GetModuleByModuleName("python27", 0, NULL, &pModuleBase);
		if (SUCCEEDED(hr))
		{
			strcpy(m_pydll, "python27");
			break;
		}
	} while (false);

	m_bReady = strlen(m_pydll) > 0;
	return m_bReady;
}

char const* PyEnv::pydll() const
{
	return m_pydll;
}
