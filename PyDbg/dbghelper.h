#pragma once

std::wstring a2w(std::string strIn);
std::string w2a(std::wstring strIn);

std::string fmt(char* fmt, ...);

void myprintex(int indent, char* fmt, ...);
void myprintwex(int indent, wchar_t* fmt, ...);
void myprint(char *fmt, ...);
void myprintw(wchar_t *fmt, ...);

bool readstring(ULONG_PTR addr, std::string& val);
bool readwstring(ULONG_PTR addr, size_t count, std::wstring& val);
bool readdword(ULONG_PTR addr, DWORD& val);
bool readstruct(ULONG_PTR addr, ULONG_PTR buffer, size_t bufferCountInBytes);

std::string addr2name(LONG64 addr);

unsigned long getstructmemberoffset(char const* structname, char const* member);
unsigned long getstructsize(char const* structname);

