#pragma once

class PyEnv
{
public:
	PyEnv();
	~PyEnv();

public:
	bool ready();
	char const* pydll() const;

private:
	char m_pydll[20];
	bool m_bInit;
	bool m_bReady;
};

extern PyEnv g_py_env;