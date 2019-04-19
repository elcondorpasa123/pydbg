#include <Windows.h>
#include <string>
#include <vector>
#include "assert.h"
#include <process.h>

int FormatV(std::string& str,IN const char* szFormat, IN va_list args)
{
	int len = _vscprintf_p(szFormat, args) + 1;
	char* buf = (char*)malloc(len);
	vsprintf_s(buf, len, szFormat, args);
	str = buf;
	return true;
}

int Format(std::string& str, IN const char* szFormat, ...)
{
	int nResult = false;
	va_list args;
	va_start(args, szFormat);
	nResult = FormatV(str,szFormat, args);
	va_end(args);
	return nResult;
}

class CWDbgCmd
{
public:
	CWDbgCmd()
	{

	};
	~CWDbgCmd()
	{

	}

public:
	void addCmd(char const* cmd)
	{
		m_vecCmd.push_back(cmd);
	}

	std::string getCmdStr()
	{
		std::string str;
		std::vector<std::string>::iterator iter = m_vecCmd.begin();
		for (; iter != m_vecCmd.end(); iter++)
		{
			str.append(iter->c_str());
			str.append(";");
		}
		str.append("q");
		return str;
	}

private:
	std::vector<std::string> m_vecCmd;
};

void deleteChar(std::string& s, char c)
{	
	while(1)	
	{		
		std::string::size_type pos = s.find(c);    
		if(pos == std::string::npos)			
			break;		
		s.erase(pos, 1);	
	}
}

void filterStr(std::string& s)
{
	deleteChar(s, ' ');	 // 空格
	deleteChar(s, '	');  // tab
	deleteChar(s, '\n'); // 换行
}

char* const WINDBG_EXE		= "C:\\Program Files\\WinDbg\\WinDbg(x86)\\windbg.exe";
char* const WINDBG_PARAM	= "-z \"%s\" -logo \"%s\" -c \"%s\"";
char* const WINDBG_PARAM_C	= ".load %s;%s;q";
char* const WINDBG_LOG		= "\\windbg.log";
char* const WINDBG_DMP		= "\\..\\dmp\\python_201808031238_full_procxp.dmp";
char* const WINDBG_PYDLL	= "\\..\\Debug\\PyDbg.dll";

int pydll_exec_unittest(char const* szCmd, char const* szResult)
{
	char szPath[MAX_PATH] = {0};
	GetModuleFileNameA(NULL, szPath, MAX_PATH);
	char* szDir = strrchr(szPath, '\\');
	*szDir = 0;
	std::string strDmp(szPath);
	strDmp.append(WINDBG_DMP);
	std::string strPyDll(szPath);
	strPyDll.append(WINDBG_PYDLL);
	std::string strLog(szPath);
	strLog.append(WINDBG_LOG);

	CWDbgCmd cmd;
	std::string cmdLoadDll;
	Format(cmdLoadDll, ".load %s", strPyDll.c_str());
	cmd.addCmd(cmdLoadDll.c_str());
	cmd.addCmd(".echo begin_output");
	cmd.addCmd(szCmd);
	cmd.addCmd(".echo end_output");

	std::string strParam;
	Format(strParam, WINDBG_PARAM, strDmp.c_str(), strLog.c_str(), cmd.getCmdStr().c_str());

	HDESK hDesk = CreateDesktopA("NewDesktop", NULL, NULL, 0, GENERIC_ALL, NULL);  

	STARTUPINFOA si;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	si.lpDesktop="NewDesktop";
// 	si.dwFlags = STARTF_USESHOWWINDOW;
// 	si.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION pi;
	ZeroMemory( &pi, sizeof(pi) );

	std::string strRunParam = WINDBG_EXE;
	strRunParam.append(" ");

	strRunParam.append(strParam);
	// 创建微端进程，暂停，用于注入
	if(!::CreateProcessA(NULL, (LPSTR)strRunParam.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
/*		assert(!"启动windbg进程失败");*/
		CloseHandle(hDesk);
		return 1;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(hDesk);

	FILE* fp = fopen(strLog.c_str(), "r");
	if (!fp)
		return 2;

	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buf = (char*)malloc(len);
	fread(buf, len, 1, fp);
	fclose(fp);
	std::string strAll(buf);
	std::string::size_type begin = strAll.rfind("begin_output");
	std::string::size_type end = strAll.rfind("end_output");
	if (begin == std::string::npos || end == std::string::npos)
		return 3;

	begin += strlen("begin_output");
	std::string strOutput = strAll.substr(begin, end - begin);
	if (strOutput.length() == 0)
		return 4;

	filterStr(strOutput);
	std::string strResultShouldBe(szResult);
	filterStr(strResultShouldBe);
	if (strResultShouldBe.compare(strOutput) != 0)
		return 5;

	return 0;
}