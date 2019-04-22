#include "stdafx.h"
#include "pyobjdef.h"
#include "dbghelper.h"
#include "pyenv.h"
#include "pyobject.h"
#include "pyobjimpl.h"

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
	PPyIntObject pyObj;
	if (!pyObj.valid())
		return false;

	if (pyObj.read(addr))
	{
		myprintex(indent, "%d\n", pyObj.get("ob_ival"));
		return true;
	}
	else
	{
		myprintex(indent, "fail to read pyintobject content\n");
		return false;
	}
}

bool dbg_pyobject_long(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PPyLongObject pyObj;
	if (!pyObj.valid())
		return false;

	if (pyObj.read(addr))
	{
		Py_ssize_t size_orginal = pyObj.get_as_Py_ssize_t("ob_size");
		Py_ssize_t size = abs(size_orginal);
		bool sign = size_orginal > 0;
// 		myprintex(indent, "pylongobject length: %d\n", size);
		if (size == 0)
		{
			myprintex(indent, "0\n");
			return true;
		}
		else
		{
			digit* digits = new digit[size];
			if (readstruct((ULONG_PTR)((char*)addr + pyObj.offset("ob_digit")), (ULONG_PTR)digits, size * sizeof(digit)))
			{
				myprintex(indent, "pylongobject digits:\n");
				__int64 value = 0;
				for (Py_ssize_t i = 0 ; i < size; i++)
				{
					digit digit_ = digits[size - i - 1];
					value = 32768 * value + digit_;
// 					myprintex(indent, "0x%04x\n", digit_);
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
		myprintex(indent, "fail to read pylongobject\n");
		return false;
	}
}

bool dbg_pyobject_bool(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PPyBoolObject pyObj;
	if (!pyObj.valid())
		return false;

	if (pyObj.read(addr))
	{
		myprintex(indent, "%s\n", pyObj.get("ob_ival") ? "True" : "False");
		return true;
	}
	else
	{
		myprintex(indent, "fail to read pyboolobject content\n");
		return false;
	}
}

bool dbg_pyobject_string(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PPyStringObject pyObj;
	if (!pyObj.valid())
		return false;

	if (pyObj.read(addr))
	{
		std::string strContent;
		if (readstring((ULONG_PTR)((int)addr + pyObj.offset("ob_sval")), strContent))
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
	else
	{
		myprintex(indent, "fail to read pystringobject\n");
		return false;
	}
}

bool dbg_pyobject_unicode(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PPyUnicodeObject pyObj;
	if (!pyObj.valid())
		return false;

	if (pyObj.read(addr))
	{
		std::wstring strContent;
		if (readwstring((ULONG_PTR)pyObj.get("str"), pyObj.get("length"), strContent))
		{
			myprintwex(indent, L"u\"%s\"\n", strContent.c_str());
// 			myprintex(indent, "hash: %d\n", pyObj.get("hash"));
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
		myprintex(indent, "fail to read pyunicodeobject\n");
		return false;
	}
}

bool check_pydbg_env_impl()
{
	if (!g_py_env.ready())
	{
		myprint("不支持的python版本");
		return false;
	}
	return true;
}

#define check_pydbg_env()	\
	if (!check_pydbg_env_impl())	\
		return false;

bool dbg_pyobject(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	if (!addr) return false;

	check_nestlevel();
// 	myprintex(indent, "analyze pyobject at address 0x%08x\n", addr);

	do 
	{
		// PPyObject
		PPyObject pyObj;
		if (!pyObj.valid())
		{
			myprintex(indent, "cannot get info of %s\n", "PyObject");
			break;
		}

		if (!pyObj.read(addr))
		{
			myprintex(indent, "fail to get mem of %s\n", "PyObject");
			break;
		}

// 		myprintex(indent, "ob_refcnt: %d\n", pyObj.get("ob_refcnt"));

		// PPyTypeObject
		PPyTypeObject typeObj;
		if (!typeObj.valid())
		{
			myprintex(indent, "cannot get info of %s\n", "PyTypeObject");
			break;
		}

		if (!typeObj.read((ULONG_PTR)pyObj.get("ob_type")))
		{
			myprintex(indent, "fail to get mem of %s\n", "PyTypeObject");
			break;
		}

		std::string strType;
		if (!readstring((ULONG_PTR)typeObj.get("tp_name"), strType))
		{
			myprintex(indent, "fail to get pyobject typename\n");
			break;
		}

// 		myprintex(indent, "pytype: %s\n", strType.c_str());
		
		if (0 == strType.compare("str"))
		{
		 	dbg_pyobject_string(addr, indent);
		}
		else if (0 == strType.compare("int"))
		{
		 	dbg_pyobject_int(addr, indent);
		}
		else if (0 == strType.compare("long"))
		{
		 	dbg_pyobject_long(addr, indent);
		}
		else if (0 == strType.compare("unicode"))
		{
		 	dbg_pyobject_unicode(addr, indent);
		}
		else if (0 == strType.compare("bool"))
		{
		 	dbg_pyobject_bool(addr, indent);
		}
		// 	else if (0 == strType.compare("complex"))
		// 	{
		// 		dbg_pyobject_complex(addr, indent);
		// 	}
		// 	else if (0 == strType.compare("float"))
		// 	{
		// 		dbg_pyobject_float(addr, indent);
		// 	}
		else if (0 == strType.compare("dict"))
		{
		 	dbg_pyobject_dict(addr, indent);
		}
		else if (0 == strType.compare("set"))
		{
		 	dbg_pyobject_set(addr, indent);
		}
		else if (0 == strType.compare("list"))
		{
		 	dbg_pyobject_list(addr, indent);
		}
		else if (0 == strType.compare("tuple"))
		{
		 	dbg_pyobject_tuple(addr, indent);
		}
		// 	else if (0 == strType.compare("type"))
		// 	{
		// 		dbg_pyobject_type(addr, indent);
		// 	}
		else if (0 == strType.compare("builtin_function_or_method"))
		{
		 	dbg_pyobject_function(addr, indent);
		}
		else if (0 == strType.compare("module"))
		{
			dbg_pyobject_module(addr, indent);
		}
		else if (0 == strType.compare("code"))
		{
			dbg_pyobject_code(addr, indent);
		}
		else if (0 == strType.compare("frame"))
		{
			dbg_pyobject_frame(addr, indent);
		}
		else if (0 == strType.compare("file"))
		{
			dbg_pyobject_file(addr, indent);
		}
		else
		{
		 	myprintex(indent, "unsupported pytype: %s\n", strType.c_str());
		}
	} while (false);
	return true;
}

bool dbg_pyobject_list(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PPyListObject pyObj;
	if (!pyObj.valid())
		return false;

	if (pyObj.read(addr))
	{
		myprintex(indent, ">>list<< \n");

		Py_ssize_t size = pyObj.get_as_Py_ssize_t("ob_size");
		for (Py_ssize_t i = 0 ; i < size; i++)
		{
			DWORD ppItem = int(pyObj.get("ob_item")) + i * sizeof(int*);
			DWORD pItem = 0;
			myprintex(indent, "item【%d】", i);
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
		myprintex(indent, "fail to read pylistobject\n");
		return false;
	}
}

bool dbg_pyobject_tuple(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PPyTupleObject pyObj;
	if (!pyObj.valid())
		return false;

	if (pyObj.read(addr))
	{
		myprintex(indent, ">>tuple<< \n");

		Py_ssize_t size = pyObj.get_as_Py_ssize_t("ob_size");
		for (Py_ssize_t i = 0 ; i < size; i++)
		{
			DWORD ppItem = (DWORD)((int)addr + pyObj.offset("ob_item") + i * sizeof(int*));
			DWORD pItem = 0;
			myprintex(indent, "item【%d】", i);
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
		myprintex(indent, "fail to read pytupleobject\n");
		return false;
	}
}

bool dbg_pyobject_dict(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PPyDictObject pyObj;
	if (!pyObj.valid())
		return false;

	if (pyObj.read(addr))
	{
		myprintex(indent, ">>dict<< \n");

		Py_ssize_t size = pyObj.get_as_Py_ssize_t("ma_mask");
		Py_ssize_t size_ = 0;
		for (Py_ssize_t i = 0 ; i < size; i++)
		{
			DWORD pItem = (DWORD)((int)pyObj.get("ma_table") + i * sizeof(PPyDictEntry));
			PPyDictEntry entry;
			ZeroMemory((LPVOID)&entry, sizeof(entry));
			if (readstruct(pItem, (ULONG_PTR)&entry, sizeof(entry)))
			{
				if (entry.me_hash != 0)
				{
					myprintex(indent, "item【%d】 at 0x%08x\n", size_++, pItem);

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
		myprintex(indent, "fail to read pydictobject\n");
		return false;
	}
}

bool dbg_pyobject_set(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PPySetObject pyObj;
	if (!pyObj.valid())
		return false;

	if (pyObj.read(addr))
	{
		myprintex(indent, ">>set<< \n");

		Py_ssize_t size = pyObj.get_as_Py_ssize_t("mask");
		Py_ssize_t size_ = 0;
		for (Py_ssize_t i = 0 ; i < size; i++)
		{
			DWORD pItem = (DWORD)((int)pyObj.get("table") + i * sizeof(PPySettEntry));
			PPySettEntry entry;
			ZeroMemory((LPVOID)&entry, sizeof(entry));
			if (readstruct(pItem, (ULONG_PTR)&entry, sizeof(entry)))
			{
				if (entry.hash != 0)
				{
					myprintex(indent, "item【%d】 at 0x%08x\n", size_++, pItem);

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
		myprintex(indent, "fail to read pysetobject\n");
		return false;
	}
}

bool dbg_pyobject_function(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PPyCFunctionObject pyObj;
	if (!pyObj.valid())
		return false;

	if (pyObj.read(addr))
	{
		myprintex(indent, ">>function<< \n");

		// method
		myprintex(indent, "-method: \n");
		PPyMethodDef methodDef;
		ZeroMemory((LPVOID)&methodDef, sizeof(methodDef));
		if (readstruct((ULONG_PTR)pyObj.get("m_ml"), (ULONG_PTR)&methodDef, sizeof(methodDef)))
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

// 			std::string doc;
// 			if (readstring((ULONG_PTR)methodDef.ml_doc, doc))
// 			{
// 				myprintex(indent, "  -doc: %s\n", doc.c_str());
// 			}
// 			else
// 			{
// 				myprintex(indent, "fail to get method doc\n");
// 			}
		}
		else
		{
			myprintex(indent, "fail to get methoddef\n");
		}

		// self
// 		myprintex(indent, "-self: \n");
// 		dbg_pyobject((ULONG_PTR)pyObj.get("m_self"), indent+1, nestlevel+1);

		// self
		myprintex(indent, "-module: \n");
		dbg_pyobject((ULONG_PTR)pyObj.get("m_module"), indent+1, nestlevel+1);
	}
	else
	{
		myprintex(indent, "fail to read pyfunctionobject\n");
		return false;
	}
	return true;
}

bool dbg_pyobject_module(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PPyModuleObject pyObj;
	if (!pyObj.valid())
		return false;

	if (pyObj.read(addr))
	{
		myprintex(indent, ">>module<< \n");

		// dict
// 		dbg_pyobject((ULONG_PTR)pyObj.md_dict, indent+1, nestlevel+1);
		dbg_pyobject((ULONG_PTR)pyObj.get("md_dict"), indent, nestlevel);
	}
	else
	{
		myprintex(indent, "fail to read pymoduleobject\n");
		return false;
	}
	return true;
}

bool dbg_pyobject_code(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PPyCodeObject pyObj;
	if (!pyObj.valid())
		return false;

	if (pyObj.read(addr))
	{
		myprintex(indent, ">>code<< \n");

		myprintex(indent+1, "co_argcount: %d\n", pyObj.get("co_argcount"));
		myprintex(indent+1, "co_nlocals: %d\n", pyObj.get("co_nlocals"));
		myprintex(indent+1, "co_stacksize: %d\n", pyObj.get("co_stacksize"));
		myprintex(indent+1, "co_flags: %d\n", pyObj.get("co_flags"));

		myprintex(indent, "	-co_code at 0x%08x:\n", pyObj.get("co_code"));
		myprintex(indent, "	-co_consts at 0x%08x:\n", pyObj.get("co_consts"));
		myprintex(indent, "	-co_names at 0x%08x:\n", pyObj.get("co_names"));

		myprintex(indent, "	-co_varnames at 0x%08x:\n", pyObj.get("co_varnames"));
		dbg_pyobject((ULONG_PTR)pyObj.get("co_varnames"), indent+1, nestlevel);

		myprintex(indent, "	-co_freevars at 0x%08x:\n", pyObj.get("co_freevars"));
		dbg_pyobject((ULONG_PTR)pyObj.get("co_freevars"), indent+1, nestlevel);

		myprintex(indent, "	-co_cellvars at 0x%08x:\n", pyObj.get("co_cellvars"));
		dbg_pyobject((ULONG_PTR)pyObj.get("co_cellvars"), indent+1, nestlevel);

		myprintex(indent, "	-co_name at 0x%08x:\n", pyObj.get("co_name"));
		dbg_pyobject((ULONG_PTR)pyObj.get("co_name"), indent+1, nestlevel);

		myprintex(indent, "	-co_filename at 0x%08x:\n", pyObj.get("co_filename"));
		dbg_pyobject((ULONG_PTR)pyObj.get("co_filename"), indent+1, nestlevel);

		myprintex(indent+1, "line: %d\n", pyObj.get("co_firstlineno"));
		return true;
	}
	else
	{
		myprintex(indent, "fail to read pycodeobject\n");
		return false;
	}
}

bool dbg_pyobject_frame(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PPyFrameObject pyObj;
	if (!pyObj.valid())
		return false;

	if (pyObj.read(addr))
	{
		myprintex(indent, ">>frame<< \n");

		myprintex(indent, "	-f_code at 0x%08x:\n", pyObj.get("f_code"));
		dbg_pyobject((ULONG_PTR)pyObj.get("f_code"), indent+1, nestlevel);
		myprintex(indent, "	-f_builtins at 0x%08x:\n", pyObj.get("f_builtins"));
		dbg_pyobject((ULONG_PTR)pyObj.get("f_builtins"), indent+1, nestlevel);
		myprintex(indent, "	-f_globals at 0x%08x:\n", pyObj.get("f_globals"));
		dbg_pyobject((ULONG_PTR)pyObj.get("f_globals"), indent+1, nestlevel);
		myprintex(indent, "	-f_locals at 0x%08x:\n", pyObj.get("f_locals"));
		dbg_pyobject((ULONG_PTR)pyObj.get("f_locals"), indent+1, nestlevel);
		return true;
	}
	else
	{
		myprintex(indent, "fail to read pyframeobject\n");
		return false;
	}
}

bool dbg_pyobject_file(ULONG_PTR addr, int indent /* = 0 */, int nestlevel /* = 0 */)
{
	PPyFileObject pyObj;
	if (!pyObj.valid())
		return false;

	if (pyObj.read(addr))
	{
		myprintex(indent, ">>file<< \n");

		myprintex(indent, "	-f_name at 0x%08x:\n", pyObj.get("f_name"));
		dbg_pyobject((ULONG_PTR)pyObj.get("f_name"), indent+1, nestlevel);
		return true;
	}
	else
	{
		myprintex(indent, "fail to read pyfileobject\n");
		return false;
	}
}
