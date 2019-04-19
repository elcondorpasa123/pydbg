#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�:
#include <windows.h>
#include <strsafe.h>

// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
#include <stdio.h>
#include <string.h>
#include <vector>
using namespace std;

#include <assert.h>


#define KDEXT_64BIT

#include <wdbgexts.h>
#include <dbgeng.h>

#include <extsfns.h>

#pragma comment(lib, "dbgeng.lib")


#pragma warning( disable : 4996 )
#pragma warning( disable : 4995 )


extern IDebugClient* g_pDebugClient;
extern IDebugControl* g_pDebugControl;
extern IDebugDataSpaces* g_pDebugDataSpaces;
extern IDebugSymbols* g_pDebugSymbols;
extern IDebugSymbols3* g_pDebugSymbols3;

typedef int ssize_t;
typedef ssize_t         Py_ssize_t;
