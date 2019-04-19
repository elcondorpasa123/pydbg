#include "stdafx.h"
#include "pyobj.h"
#include "dbghelper.h"
#include "pyhelper.h"

#define OFFSET(TYPE, MEMBER) ((unsigned long)(&(((TYPE *)0)->MEMBER)))

int g_level_default = 2;
int g_level = 2;

void setnestlevel(int level)
{
	g_level = level;
}

void resetnestlevel()
{
	g_level = g_level_default;
}

#define check_nestlevel();	\
	if (indent > g_level)	\
		return true;	

bool dbg_pyobject_int(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	DWORD dwVal;
	if (readdword((ULONG_PTR)((BYTE*)addr + OFFSET(PyIntObject, ob_ival)), dwVal))
	{
		myprintex(indent, "%d\n", dwVal);
		return true;
	}
	else
	{
		myprintex(indent, "fail to get pyintobject content\n");
		return false;
	}
}

bool dbg_pyobject_long(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PyLongObject pyObj;
	if (readstruct(addr, (ULONG_PTR)&pyObj, sizeof(pyObj)))
	{
		Py_ssize_t size = abs(pyObj.ob_size);
		bool sign = pyObj.ob_size > 0;
		myprintex(indent, "pylongobject length: %d\n", pyObj.ob_size);
		if (size == 0)
		{
			myprintex(indent, "0\n");
			return true;
		}
		else
		{
			digit* digits = new digit[size];
			if (readstruct((ULONG_PTR)((BYTE*)addr + OFFSET(PyLongObject, ob_digit)), (ULONG_PTR)digits, size * sizeof(digit)))
			{
				myprintex(indent, "pylongobject digits:\n");
				__int64 value = 0;
				for (Py_ssize_t i = 0 ; i < size; i++)
				{
					digit digit_ = digits[size - i - 1];
					value = 32768 * value + digit_;
					myprintex(indent, "0x%04x\n", digit_);
				}
				myprintex(indent, "long: %c%I64uL\n", sign ? '+': '-', value);
				delete digits;
				return true;
			}
			else
			{
				myprintex(indent, "fail to get pylongobject digits\n");
				return false;
			}		
		}
	}
	else
	{
		myprintex(indent, "fail to get pylongobject\n");
		return false;
	}
}

bool dbg_pyobject_bool(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	DWORD dwVal;
	if (readdword((ULONG_PTR)((BYTE*)addr + OFFSET(PyBoolObject, ob_ival)), dwVal))
	{
		myprintex(indent, "%s\n", dwVal ? "True" : "False");
		return true;
	}
	else
	{
		myprintex(indent, "fail to get pyboolobject content\n");
		return false;
	}
}

bool dbg_pyobject_string(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	std::string strContent;
	if (readstring((ULONG_PTR)((BYTE*)addr + OFFSET(PyStringObject, ob_sval)), strContent))
	{
		myprintex(indent, "\"%s\"\n", strContent.c_str());
		return true;
	}
	else
	{
		myprintex(indent, "fail to get pystringobject content\n");
		return false;
	}
}


bool dbg_pyobject_unicode(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PyUnicodeObject pyObj;
	if (readstruct(addr, (ULONG_PTR)&pyObj, sizeof(pyObj)))
	{
		std::wstring strContent;
		if (readwstring((ULONG_PTR)pyObj.str, pyObj.length, strContent))
		{
			myprintwex(indent, L"u\"%s\"\n", strContent.c_str());
			myprintex(indent, "hash: %d\n", pyObj.hash);
			return true;
		}
		else
		{
			myprintex(indent, "fail to get pyunicodeobject content\n");
			return false;
		}
	}
	else
	{
		myprintex(indent, "fail to get pyunicodeobject\n");
		return false;
	}
}

unsigned long getstructsize(char const* structname)
{
	unsigned long ulSize = 0;

	HRESULT hr = S_OK;
	do 
	{
		ULONG64 pModuleBase = 0;
		hr = g_pDebugSymbols->GetModuleByModuleName("python27_d", 0, NULL, &pModuleBase);
		if (FAILED(hr))
			break;

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

unsigned long getstructmemberoffset(char const* structname, char const* membername)
{
	unsigned long ulOffset = 0;

	HRESULT hr = S_OK;
	do 
	{
		ULONG64 pModuleBase = 0;
		hr = g_pDebugSymbols->GetModuleByModuleName("python27_d", 0, NULL, &pModuleBase);
		if (FAILED(hr))
			break;

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

class CPyMem
{
public:
	CPyMem(int nSize)
		: m_size(nSize)
	{
		m_ptr = malloc(nSize);
		ZeroMemory(m_ptr, nSize);
	}

	~CPyMem()
	{
		free(m_ptr);
		m_ptr = nullptr;
	}

public:
	void* operator &()
	{
		return m_ptr;
	}

	unsigned long getat(unsigned long offset)
	{
		assert(offset < m_size);
		return *((unsigned long*)((char*)m_ptr + offset));
	}

private:
	void* m_ptr;
	unsigned long m_size;
};

class CPyObjectRestructed
{
public:
	CPyObjectRestructed(char const* name)
		: m_name(name)
	{
		m_size = getstructsize(name);
	}

	~CPyObjectRestructed()
	{
	}

public:
	bool valid()
	{
		return m_size > 0;
	}

	size_t size()
	{
		return m_size;
	}

	DWORD member_value_localaddr(ULONG_PTR* addr, char const* member)
	{
		if (!valid())
		{
			return -1;
		}
		return -1;

	}

	DWORD member_value_remoteaddr(ULONG_PTR* addr, char const* member)
	{
		if (!valid())
		{
			return -1;
		}
		return -1;

	}

	int member_offset(char const* member)
	{
		if (!valid())
			return -1;

		unsigned long ulOffset = getstructmemberoffset(m_name.c_str(), member);
		return ulOffset;
	}

private:
	size_t m_size;
	ULONG64 m_pModuleBase;
	ULONG m_uTypeId;
	std::string m_name;
};

bool dbg_pyobject(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	check_nestlevel();

	myprintex(indent, "analyze pyobject at address 0x%08x\n", addr);

	CPyObjectRestructed oPyObject("PyObject");
	if (!oPyObject.valid())
	{
		myprintex(indent, "cannot get size:%s\n", "PyObject");
		return false;
	}

	CPyMem pyObj(oPyObject.size());
	if (readstruct(addr, (ULONG_PTR)&pyObj, oPyObject.size()))
	{
		myprintex(indent, "ob_refcnt: %d\n", pyObj.getat(oPyObject.member_offset("ob_refcnt")));

		CPyObjectRestructed oPyTypeObject("PyTypeObject");
		CPyMem typeObj(oPyTypeObject.size());
// 		if (readstruct((ULONG_PTR)pyObj.ob_type, (ULONG_PTR)&typeObj, sizeof(typeObj)))
		if (readstruct((ULONG_PTR)pyObj.getat(oPyObject.member_offset("ob_type")), (ULONG_PTR)&typeObj, oPyTypeObject.size()))
		{
			std::string strType;
// 			if (readstring((ULONG_PTR)typeObj.tp_name, strType))
			if (readstring((ULONG_PTR)typeObj.getat(oPyTypeObject.member_offset("tp_name")), strType))
			{
				myprintex(indent, "pytype: %s\n", strType.c_str());
// 				if (0 == strType.compare("str"))
// 				{
// 					dbg_pyobject_string(addr, indent);
// 				}
// 				else if (0 == strType.compare("int"))
// 				{
// 					dbg_pyobject_int(addr, indent);
// 				}
// 				else if (0 == strType.compare("long"))
// 				{
// 					dbg_pyobject_long(addr, indent);
// 				}
// 				else if (0 == strType.compare("unicode"))
// 				{
// 					dbg_pyobject_unicode(addr, indent);
// 				}
// 				else if (0 == strType.compare("bool"))
// 				{
// 					dbg_pyobject_bool(addr, indent);
// 				}
// 				// 	else if (0 == strType.compare("complex"))
// 				// 	{
// 				// 		dbg_pyobject_complex(addr, indent);
// 				// 	}
// 				// 	else if (0 == strType.compare("float"))
// 				// 	{
// 				// 		dbg_pyobject_float(addr, indent);
// 				// 	}
// 				else if (0 == strType.compare("dict"))
// 				{
// 					dbg_pyobject_dict(addr, indent);
// 				}
// 				else if (0 == strType.compare("set"))
// 				{
// 					dbg_pyobject_set(addr, indent);
// 				}
// 				else if (0 == strType.compare("list"))
// 				{
// 					dbg_pyobject_list(addr, indent);
// 				}
// 				else if (0 == strType.compare("tuple"))
// 				{
// 				 	dbg_pyobject_tuple(addr, indent);
// 				}
// 				// 	else if (0 == strType.compare("type"))
// 				// 	{
// 				// 		dbg_pyobject_type(addr, indent);
// 				// 	}
// 				else if (0 == strType.compare("builtin_function_or_method"))
// 				{
// 					dbg_pyobject_function(addr, indent);
// 				}
// 				else if (0 == strType.compare("module"))
// 				{
// 					dbg_pyobject_module(addr, indent);
// 				}
// 				else
// 				{
// 					myprintex(indent, "unsupported pytype: %s\n", strType.c_str());
// 				}
// 				return true;
			}
			else
			{
				myprintex(indent, "fail to get pyobject typename\n");
				return false;
			}
		}
		else
		{
			myprintex(indent, "fail to get pytypeobject\n");
			return false;
		}

	}
	else
	{
		myprintex(indent, "fail to get pyobject\n");
		return false;
	}
}

bool dbg_pyobject_list(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PyListObject pyObj;
	if (readstruct(addr, (ULONG_PTR)&pyObj, sizeof(pyObj)))
	{
		myprintex(indent, ">>>>list<<<<: \n");

		Py_ssize_t size = pyObj.ob_size;
		for (Py_ssize_t i = 0 ; i < size; i++)
		{
			DWORD ppItem = int(pyObj.ob_item) + i * sizeof(int*);
			DWORD pItem = 0;
			myprintex(indent, "item¡¾%d¡¿", i);
			if (readdword((ULONG_PTR)ppItem, pItem))
			{
				myprintex(0, " at 0x%08x\n",pItem);
				dbg_pyobject(pItem, indent+1, nestlevel+1);
			}
			else
			{
				myprintex(0, "\n");
				myprintex(indent, "fail to get item address\n");
			}
		}
		return true;
	}
	else
	{
		myprintex(indent, "fail to get pylistobject\n");
		return false;
	}
}

bool dbg_pyobject_tuple(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PyTupleObject pyObj;
	if (readstruct(addr, (ULONG_PTR)&pyObj, sizeof(pyObj)))
	{
		myprintex(indent, ">>>>tuple<<<<: \n");

		Py_ssize_t size = pyObj.ob_size;
		for (Py_ssize_t i = 0 ; i < size; i++)
		{
			DWORD ppItem = (DWORD)((BYTE*)addr + OFFSET(PyTupleObject, ob_item) + i * sizeof(int*));
			DWORD pItem = 0;
			myprintex(indent, "item¡¾%d¡¿", i);
			if (readdword((ULONG_PTR)ppItem, pItem))
			{
				myprintex(0, " at 0x%08x\n",pItem);
				dbg_pyobject(pItem, indent+1, nestlevel+1);
			}
			else
			{
				myprintex(0, "\n");
				myprintex(indent, "fail to get item address\n");
			}
		}
		return true;
	}
	else
	{
		myprintex(indent, "fail to get pytupleobject\n");
		return false;
	}
}

bool dbg_pyobject_dict(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PyDictObject pyObj;
	if (readstruct(addr, (ULONG_PTR)&pyObj, sizeof(pyObj)))
	{
		myprintex(indent, ">>>>dict<<<<: \n");

		Py_ssize_t size = pyObj.ma_mask;
		Py_ssize_t size_ = 0;
		for (Py_ssize_t i = 0 ; i < size; i++)
		{
			DWORD pItem = (DWORD)((BYTE*)pyObj.ma_table + i * sizeof(PyDictEntry));
			PyDictEntry entry;
			ZeroMemory((LPVOID)&entry, sizeof(entry));
			if (readstruct(pItem, (ULONG_PTR)&entry, sizeof(entry)))
			{
				if (entry.me_hash != 0)
				{
					myprintex(indent, "item¡¾%d¡¿ at 0x%08x\n", size_++, pItem);

					myprintex(indent, "	-hash 0x%08x\n",entry.me_hash);
					myprintex(indent, "	-key at 0x%08x:\n", entry.me_key);
					dbg_pyobject((ULONG_PTR)entry.me_key, indent+2, nestlevel+1);
					myprintex(indent, "	-value at 0x%08x:\n", entry.me_value);
					dbg_pyobject((ULONG_PTR)entry.me_value, indent+2, nestlevel+1);				
				}
			}
			else
			{
				myprintex(indent, "\n");
				myprintex(indent, "fail to get item address\n");
			}
		}
		return true;
	}
	else
	{
		myprintex(indent, "fail to get pydictobject\n");
		return false;
	}
}

bool dbg_pyobject_set(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PySetObject pyObj;
	if (readstruct(addr, (ULONG_PTR)&pyObj, sizeof(pyObj)))
	{
		myprintex(indent, ">>>>set<<<<: \n");

		Py_ssize_t size = pyObj.mask;
		Py_ssize_t size_ = 0;
		for (Py_ssize_t i = 0 ; i < size; i++)
		{
			DWORD pItem = (DWORD)((BYTE*)pyObj.table + i * sizeof(setentry));
			setentry entry;
			ZeroMemory((LPVOID)&entry, sizeof(entry));
			if (readstruct(pItem, (ULONG_PTR)&entry, sizeof(entry)))
			{
				if (entry.hash != 0)
				{
					myprintex(indent, "item¡¾%d¡¿ at 0x%08x\n", size_++, pItem);

					myprintex(indent, "	-hash 0x%08x\n",entry.hash);
					myprintex(indent, "	-value at 0x%08x:\n", entry.key);
					dbg_pyobject((ULONG_PTR)entry.key, indent+2, nestlevel+1);				
				}
			}
			else
			{
				myprintex(indent, "\n");
				myprintex(indent, "fail to get item address\n");
			}
		}
		return true;
	}
	else
	{
		myprintex(indent, "fail to get pysetobject\n");
		return false;
	}
}

bool dbg_pyobject_function(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PyCFunctionObject pyObj;
	ZeroMemory((LPVOID)&pyObj, sizeof(pyObj));
	if (readstruct(addr, (ULONG_PTR)&pyObj, sizeof(pyObj)))
	{
		myprintex(indent, ">>>>function<<<<: \n");

		// method
		myprintex(indent, "-method: \n");
		PyMethodDef methodDef;
		ZeroMemory((LPVOID)&methodDef, sizeof(methodDef));
		if (readstruct((ULONG_PTR)pyObj.m_ml, (ULONG_PTR)&methodDef, sizeof(methodDef)))
		{
			std::string name;
			if (readstring((ULONG_PTR)methodDef.ml_name, name))
			{
				myprintex(indent, "  -name: %s\n", name.c_str());
			}
			else
			{
				myprintex(indent, "fail to get method name\n");
			}
			DWORD meth = (DWORD)methodDef.ml_meth;
			myprintex(indent, "  -meth: 0x%08x\n", meth);
			myprintex(indent, "    - %s\n", addr2name(meth).c_str());

			int flag = methodDef.ml_flags;
			myprintex(indent, "  -flag: 0x%08x\n", flag);

			std::string doc;
			if (readstring((ULONG_PTR)methodDef.ml_doc, doc))
			{
				myprintex(indent, "  -doc: %s\n", doc.c_str());
			}
			else
			{
				myprintex(indent, "fail to get method doc\n");
			}
		}
		else
		{
			myprintex(indent, "fail to get methoddef\n");
		}

		// self
		myprintex(indent, "-self: \n");
		dbg_pyobject((ULONG_PTR)pyObj.m_self, indent+1, nestlevel+1);

		// self
		myprintex(indent, "-module: \n");
		dbg_pyobject((ULONG_PTR)pyObj.m_module, indent+1, nestlevel+1);
	}
	else
	{
		myprintex(indent, "fail to get pyfunctionobject\n");
		return false;
	}
	return true;
}

bool dbg_pyobject_module(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PyModuleObject pyObj;
	ZeroMemory((LPVOID)&pyObj, sizeof(pyObj));
	if (readstruct(addr, (ULONG_PTR)&pyObj, sizeof(pyObj)))
	{
		myprintex(indent, ">>>>module<<<<: \n");

		// dict
// 		dbg_pyobject((ULONG_PTR)pyObj.md_dict, indent+1, nestlevel+1);
		dbg_pyobject((ULONG_PTR)pyObj.md_dict, indent, nestlevel);
	}
	else
	{
		myprintex(indent, "fail to get pymoduleobject\n");
		return false;
	}
	return true;
}
