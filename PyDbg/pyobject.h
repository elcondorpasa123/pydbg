#include "stdafx.h"
#include "dbghelper.h"

#define nullptr 0
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

class PPyObjectBase
{
public:
	PPyObjectBase(char const* name)
		: m_name(name)
		, m_ptr(nullptr)
	{
		m_size = getstructsize(name);
	}

	~PPyObjectBase()
	{
		if (m_ptr)
		{
			free(m_ptr);
			m_ptr = nullptr;
		}
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

	int offset(char const* member)
	{
		if (!valid())
			return -1;

		unsigned long ulOffset = getstructmemberoffset(m_name.c_str(), member);
		return ulOffset;
	}

	bool read(ULONG_PTR addr)
	{
		assert(m_size);
		assert(addr);
		assert(!m_ptr);
		m_ptr = malloc(m_size);
		ZeroMemory(m_ptr, m_size);
		return readstruct(addr, (ULONG_PTR)m_ptr, m_size);
	}

	void clear()
	{
		if (m_ptr)
		{
			free(m_ptr);
			m_ptr = nullptr;
		}
	}

	unsigned long get(char const* member)
	{
		assert(member);
		assert(m_ptr);
		int voffset = offset(member);
		assert(voffset < m_size);
		return *((unsigned long*)((char*)m_ptr + voffset));
	}

	Py_ssize_t get_as_Py_ssize_t(char const* member)
	{
		assert(member);
		assert(m_ptr);
		int voffset = offset(member);
		assert(voffset < m_size);
		return *((Py_ssize_t*)((char*)m_ptr + voffset));
	}

protected:
	size_t m_size;
// 	ULONG64 m_pModuleBase;
// 	ULONG m_uTypeId;
	std::string m_name;
	void* m_ptr;
};

class PPyObject : public PPyObjectBase
{
public:
	PPyObject()
		: PPyObjectBase("PyObject")
	{}
};

class PPyTypeObject : public PPyObjectBase
{
public:
	PPyTypeObject()
		: PPyObjectBase("PyTypeObject")
	{}
};

class PPyStringObject : public PPyObjectBase
{
public:
	PPyStringObject()
		: PPyObjectBase("PyStringObject")
	{}
};

class PPyIntObject : public PPyObjectBase
{
public:
	PPyIntObject()
		: PPyObjectBase("PyIntObject")
	{}
};

typedef PPyIntObject PPyBoolObject;

class PPyUnicodeObject : public PPyObjectBase
{
public:
	PPyUnicodeObject()
		: PPyObjectBase("PyUnicodeObject")
	{}
};

class PPyLongObject : public PPyObjectBase
{
public:
	PPyLongObject()
		: PPyObjectBase("PyLongObject")
	{}
};

class PPyListObject : public PPyObjectBase
{
public:
	PPyListObject()
		: PPyObjectBase("PyListObject")
	{}
};

class PPyTupleObject : public PPyObjectBase
{
public:
	PPyTupleObject()
		: PPyObjectBase("PyTupleObject")
	{}
};

typedef struct {
    /* Cached hash code of me_key.  Note that hash codes are C longs.
     * We have to use Py_ssize_t instead because dict_popitem() abuses
     * me_hash to hold a search finger.
     */
    Py_ssize_t me_hash;
//     PyObject *me_key;
//     PyObject *me_value;
	void* me_key;
	void* me_value;
} PPyDictEntry;

class PPyDictObject : public PPyObjectBase
{
public:
	PPyDictObject()
		: PPyObjectBase("PyDictObject")
	{}
};

typedef struct {
	long hash;      /* cached hash code for the entry key */
	void* key;
} PPySettEntry;

class PPySetObject : public PPyObjectBase
{
public:
	PPySetObject()
		: PPyObjectBase("PySetObject")
	{}
};


// typedef PyObject *(*PyCFunction)(PyObject *, PyObject *);

struct PPyMethodDef {
    const char	*ml_name;	/* The name of the built-in function/method */
// 	PyCFunction  ml_meth;	/* The C function that implements it */
	void*  ml_meth;	/* The C function that implements it */
    int		 ml_flags;	/* Combination of METH_xxx flags, which mostly
				   describe the args expected by the C func */
    const char	*ml_doc;	/* The __doc__ attribute, or NULL */
};

class PPyCFunctionObject : public PPyObjectBase
{
public:
	PPyCFunctionObject()
		: PPyObjectBase("PyCFunctionObject")
	{}
};

class PPyModuleObject : public PPyObjectBase
{
public:
	PPyModuleObject()
		: PPyObjectBase("PyModuleObject")
	{}
};

class PPyCodeObject : public PPyObjectBase
{
public:
	PPyCodeObject()
		: PPyObjectBase("PyCodeObject")
	{}
};

class PPyFrameObject : public PPyObjectBase
{
public:
	PPyFrameObject()
		: PPyObjectBase("PyFrameObject")
	{}
};

class PPyFileObject : public PPyObjectBase
{
public:
	PPyFileObject()
		: PPyObjectBase("PyFileObject")
	{}
};

