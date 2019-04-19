#include "stdafx.h"
#include "dbghelper.h"
#include "pyobjdef.h"
#include "pyenv.h"
#include "pyhelper.h"

WINDBG_EXTENSION_APIS   ExtensionApis;

BOOL APIENTRY DllMain( HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

IDebugClient* g_pDebugClient = NULL;
IDebugControl* g_pDebugControl = NULL;
IDebugDataSpaces* g_pDebugDataSpaces = NULL;
IDebugSymbols* g_pDebugSymbols = NULL;
IDebugSymbols3* g_pDebugSymbols3 = NULL;

#define SAFE_RELEASE(p)	\
	if (p)	\
{	\
	p->Release();	\
	p = NULL;	\
}

void UninitDebugService()
{
	SAFE_RELEASE(g_pDebugClient);
	SAFE_RELEASE(g_pDebugControl);
	SAFE_RELEASE(g_pDebugDataSpaces);
	SAFE_RELEASE(g_pDebugSymbols);
	SAFE_RELEASE(g_pDebugSymbols3);
}

bool InitDebugService()
{
	if (g_pDebugClient)
		return true;

	HRESULT hr  = S_OK;

	do 
	{
		hr = DebugCreate(__uuidof(IDebugClient), (void**)&g_pDebugClient);
		if (hr != S_OK)
			break;

		hr = g_pDebugClient->QueryInterface(__uuidof(IDebugControl), (void**)&g_pDebugControl);
		if (hr != S_OK)
			break;

		hr = g_pDebugClient->QueryInterface(__uuidof(IDebugDataSpaces), (void**)&g_pDebugDataSpaces);
		if (hr != S_OK)
			break;

		hr = g_pDebugClient->QueryInterface(__uuidof(IDebugSymbols), (void**)&g_pDebugSymbols);
		if (hr != S_OK)
			break;

		hr = g_pDebugClient->QueryInterface(__uuidof(IDebugSymbols3), (void**)&g_pDebugSymbols3);
		if (hr != S_OK)
			break;
	} while (false);

	if (hr != S_OK)
	{
		UninitDebugService();
	}
	return hr == S_OK;
}


// extern "C"
HRESULT CALLBACK DebugExtensionInitialize(PULONG Version, PULONG Flags)
{
	*Version = DEBUG_EXTENSION_VERSION(1,0);
	*Flags = 0;

	if (InitDebugService())
	{
		ExtensionApis.nSize = sizeof(ExtensionApis);
		return g_pDebugControl->GetWindbgExtensionApis64(&ExtensionApis);
	}
	else
	{
		return E_FAIL;
	}
}

void CALLBACK DebugExtensionUninitialize()
{
	UninitDebugService();
	return;
}

HRESULT
	CALLBACK
	KnownStructOutput(
	__in ULONG Flag,
	__in ULONG64 Address,
	__in PSTR StructName,
	__out_ecount(BufferSize) PSTR Buffer,
	__in PULONG BufferSize
	)
{
	const char* KnownStructs[] = {"ProcessNode"};
	HRESULT hr = S_OK;
	switch (Flag)
	{
	case DEBUG_KNOWN_STRUCT_GET_NAMES:
		{
			size_t remaining = *BufferSize;
			PSTR buffer = Buffer;
			size_t used = 0;
			for (size_t i = 0; i < sizeof(KnownStructs)/sizeof(KnownStructs[0]); i++)
			{
				size_t length = strlen(KnownStructs[i]);
				if (remaining >= length + 1)
				{
					strcpy(buffer, KnownStructs[i]);
					used += length+1;
					buffer += length+1;
					remaining -= length+1;
				}
				else
				{
					break;
				}
			}
			*buffer = 0;
			*BufferSize = used+1;
		}
		break;
	case DEBUG_KNOWN_STRUCT_GET_SINGLE_LINE_OUTPUT:
		{
			if (!strcmp(StructName, KnownStructs[0]))
			{
				SYSTEMTIME Data;
				ULONG ret;
				if (ReadMemory(Address, &Data, sizeof(Data), &ret))
				{
					_snprintf(Buffer, *BufferSize, " { process node:%02ld:%02ld:%02ld %02ld/%02ld/%04ld }",
						Data.wHour,
						Data.wMinute,
						Data.wSecond,
						Data.wMonth,
						Data.wDay,
						Data.wYear);
					hr = S_OK;
				}
				else
				{
					hr = E_INVALIDARG;
				}
			}
		}
		break;
	case DEBUG_KNOWN_STRUCT_SUPPRESS_TYPE_NAME:
		{

		}
		break;
	default:
		{}
	}
	return hr;
}

BOOL g_bConnected = FALSE;

void DebugExtensionNotify_Accessible()
{
	if (g_bConnected)
		return;

	if (!InitDebugService())
	{	
		return;
	}


	ULONG   TargetMachine;
	HRESULT Hr = g_pDebugControl->GetActualProcessorType(&TargetMachine);
	if (Hr != S_OK)
		return;

	g_bConnected = TRUE;
	dprintf("PyDbg Loaded\n");

// 	switch (TargetMachine)
// 	{
// 	case IMAGE_FILE_MACHINE_I386:
// 		dprintf("X86");
// 		break;
// 	case IMAGE_FILE_MACHINE_IA64:
// 		dprintf("X64");
// 		break;
// 	default:
// 		dprintf("Other");
// 		break;
// 	}
// 	dprintf("\n");

// 	g_pDebugControl->Execute(DEBUG_OUTCTL_ALL_CLIENTS | DEBUG_OUTCTL_OVERRIDE_MASK | DEBUG_OUTCTL_NOT_LOGGED,
// 		".frame",
// 		DEBUG_EXECUTE_DEFAULT);
// 	g_pDebugControl->Execute(DEBUG_OUTCTL_ALL_CLIENTS | DEBUG_OUTCTL_OVERRIDE_MASK | DEBUG_OUTCTL_NOT_LOGGED,
// 		"dv",
// 		DEBUG_EXECUTE_DEFAULT);
}

void DebugExtensionNotify_Inactive()
{
	g_bConnected = FALSE;
}

void CALLBACK DebugExtensionNotify(ULONG Notify, ULONG64 Argument)
{
	switch (Notify)
	{
	case DEBUG_NOTIFY_SESSION_ACCESSIBLE:
		{
			DebugExtensionNotify_Accessible();
		}
		break;
	case DEBUG_NOTIFY_SESSION_INACTIVE:
		{
			DebugExtensionNotify_Inactive();
		}
		break;
	default:
		{}
	}
}

