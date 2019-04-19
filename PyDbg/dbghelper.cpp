#include "stdafx.h"
#include "pyenv.h"
#include "dbghelper.h"

std::wstring a2w(std::string strIn)
{
	if(strIn.empty())
		return L"";

	size_t nOutSize = MultiByteToWideChar(CP_ACP, NULL, strIn.c_str(), strIn.length(), NULL, 0);// 获取待转换字符串的缓冲区所需大小
	if(nOutSize == 0)
		return L"";

	nOutSize++;
	wchar_t* pszOutBuf = new wchar_t[nOutSize];
	memset((void*)pszOutBuf, 0, sizeof(wchar_t) * (nOutSize));
	MultiByteToWideChar(CP_ACP, NULL, strIn.c_str(), strIn.length(), pszOutBuf, nOutSize);
	std::wstring strRet(pszOutBuf);
	delete pszOutBuf;
	return strRet;
}

std::string w2a(std::wstring strIn)
{
	if(strIn.empty())
		return "";

	size_t nOutSize = WideCharToMultiByte(CP_ACP, NULL, strIn.c_str(), strIn.length(), NULL, 0, NULL, NULL);// 获取待转换字符串的缓冲区所需大小
	if(nOutSize == 0)
		return "";

	nOutSize++;
	char* pszOutBuf = new char[nOutSize];
	memset((void*)pszOutBuf, 0, sizeof(char)* nOutSize);
	WideCharToMultiByte(CP_ACP, NULL, strIn.c_str(), strIn.length(), pszOutBuf, nOutSize, NULL, NULL);
	std::string strRet(pszOutBuf);
	delete pszOutBuf;
	return strRet;
}

char const* indent_str = "	";

void myprintex(int indent, char* fmt, ...)
{
	for (int i = 0; i < indent; i++)
		dprintf(indent_str);

	char buffer[200] = {0};
	va_list argptr;
	int cnt;
	va_start(argptr, fmt);
	cnt = StringCbVPrintfA(buffer,200-1, fmt, argptr);
	va_end(argptr);
	dprintf(buffer);
}

void myprintwex(int indent, wchar_t* fmt, ...)
{
	for (int i = 0; i < indent; i++)
		dprintf(indent_str);

	wchar_t buffer[200] = {0};
	va_list argptr;
	int cnt;
	va_start(argptr, fmt);
	cnt = StringCbVPrintfW(buffer,200-1, fmt, argptr);
	va_end(argptr);
	dprintf(w2a(buffer).c_str());
}


void myprint(char *fmt, ...)
{
	char buffer[200] = {0};
	va_list argptr;
	int cnt;
	va_start(argptr, fmt);
	cnt = StringCbVPrintfA(buffer,200-1, fmt, argptr);
	va_end(argptr);
	dprintf(buffer);
}

void myprintw(wchar_t *fmt, ...)
{
	wchar_t buffer[200] = {0};
	va_list argptr;
	int cnt;
	va_start(argptr, fmt);
	cnt = StringCbVPrintfW(buffer,200-1, fmt, argptr);
	va_end(argptr);
	// 转换为窄字节输出

	dprintf(w2a(buffer).c_str());
}

bool readdword(ULONG_PTR addr, DWORD& val)
{
	HRESULT hr = S_OK;
	byte* pointer =  (byte*)addr;
	DWORD dw = 0;
	hr = g_pDebugDataSpaces->ReadVirtual((ULONG_PTR)pointer, (PVOID)&dw, sizeof(dw), NULL);
	if (FAILED(hr))
	{
		dprintf("fail to read at address 0x%08x\n", (ULONG_PTR)pointer);
	}
	val = dw;
	return SUCCEEDED(hr);
}

bool readstruct(ULONG_PTR addr, ULONG_PTR buffer, size_t bufferCountInBytes)
{
	ZeroMemory((PVOID)buffer, bufferCountInBytes);
	HRESULT hr = S_OK;
	byte* pointer =  (byte*)addr;
	hr = g_pDebugDataSpaces->ReadVirtual((ULONG_PTR)pointer, (PVOID)buffer, bufferCountInBytes, NULL);
	if (FAILED(hr))
	{
		dprintf("fail to read at address 0x%08x\n", (ULONG_PTR)pointer);
	}
	return SUCCEEDED(hr);

}

bool readwstring(ULONG_PTR addr, size_t count, std::wstring& val)
{
	HRESULT hr = S_OK;
	wchar_t* pointer =  (wchar_t*)addr;
	wchar_t wch = 0;
	bool err = false;
	size_t count_ = count;
	do 
	{
		hr = g_pDebugDataSpaces->ReadVirtual((ULONG_PTR)pointer, (PVOID)&wch, sizeof(wch), NULL);
		if (FAILED(hr))
		{
			dprintf("fail to read at address 0x%08x\n", (ULONG_PTR)pointer);
			err = true;
			break;
		}
		if (!wch)
		{
			break;
		}
// 		dprintf("read 0x%04x\n", wch);
		val += wch;
		pointer++;
	} while (--count_ > 0);

	return !err;
}

bool readstring(ULONG_PTR addr, std::string& val)
{
	HRESULT hr = S_OK;
	byte* pointer =  (byte*)addr;
	byte ch = 0;
	bool err = false;
	do 
	{
		hr = g_pDebugDataSpaces->ReadVirtual((ULONG_PTR)pointer, (PVOID)&ch, sizeof(ch), NULL);
		if (FAILED(hr))
		{
			dprintf("fail to read at address 0x%08x\n", (ULONG_PTR)pointer);
			err = true;
			break;
		}
		if (!ch)
		{
			break;
		}
// 		dprintf("read %c 0x%02x\n", ch, ch);
		val += ch;
		pointer++;
	} while (ch != 0);

	return !err;
}


std::string addr2name(LONG64 addr)
{
	const size_t c_namesize_reserved = 100;

	std::string ret;
	char name[c_namesize_reserved] = {0};
	ULONG nameSize = 0;
	HRESULT hr = g_pDebugSymbols3->GetNameByOffset(addr, name, c_namesize_reserved-1, &nameSize, NULL);
	if (S_OK == hr)
	{
		ret = name;
	}
	else if (S_FALSE == hr)
	{
		char* reallocName = (char*)malloc(nameSize+1);
		ZeroMemory(reallocName, nameSize+1);
		g_pDebugSymbols3->GetNameByOffset(addr, reallocName, nameSize, NULL, NULL);
		ret = reallocName;
		free(reallocName);
	}
	return ret;
}


unsigned long getstructsize(char const* structname)
{
	unsigned long ulSize = 0;

	HRESULT hr = S_OK;
	do 
	{
		ULONG64 pModuleBase = 0;
		hr = g_pDebugSymbols->GetModuleByModuleName(g_py_env.pydll(), 0, NULL, &pModuleBase);
		if (FAILED(hr))
		{
			break;
		}

		ULONG uTypeId = 0;
		hr = g_pDebugSymbols3->GetTypeId(pModuleBase, structname,  &uTypeId);
		if (FAILED(hr))
			break;

		hr = g_pDebugSymbols->GetTypeSize(pModuleBase, uTypeId, &ulSize);
		if (FAILED(hr))
			break;

	} while (false);
	return ulSize;
}

bool g_b_initenv = false;
bool g_b_debug = false;

unsigned long getstructmemberoffset(char const* structname, char const* membername)
{
	unsigned long ulOffset = -1;

	HRESULT hr = S_OK;
	do 
	{
		ULONG64 pModuleBase = 0;
		hr = g_pDebugSymbols->GetModuleByModuleName(g_py_env.pydll(), 0, NULL, &pModuleBase);
		if (FAILED(hr))
		{
			break;
		}

		ULONG uTypeId = 0;
		hr = g_pDebugSymbols3->GetTypeId(pModuleBase, structname,  &uTypeId);
		if (FAILED(hr))
			break;

		hr = g_pDebugSymbols->GetFieldOffset(pModuleBase, uTypeId, membername, &ulOffset);
		if (FAILED(hr))
			break;

	} while (false);

	return ulOffset;
}
