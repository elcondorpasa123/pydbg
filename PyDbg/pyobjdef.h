#pragma once

typedef int ssize_t;

typedef ssize_t         Py_ssize_t;

#define _PyObject_HEAD_EXTRA            \
	struct _object *_ob_next;           \
	struct _object *_ob_prev;

#define PyObject_HEAD                   \
	_PyObject_HEAD_EXTRA                \
	Py_ssize_t ob_refcnt;               \
	struct _typeobject *ob_type;

#define PyObject_VAR_HEAD               \
	PyObject_HEAD                       \
	Py_ssize_t ob_size; /* Number of items in variable part */

typedef struct _object {
	PyObject_HEAD
} PyObject;

typedef struct {
	PyObject_VAR_HEAD
} PyVarObject;

// typedef void (*freefunc)(void *);
// typedef void (*destructor)(PyObject *);
// typedef int (*printfunc)(PyObject *, FILE *, int);
// typedef PyObject *(*getattrfunc)(PyObject *, char *);
// typedef PyObject *(*getattrofunc)(PyObject *, PyObject *);
// typedef int (*setattrfunc)(PyObject *, char *, PyObject *);
// typedef int (*setattrofunc)(PyObject *, PyObject *, PyObject *);
// typedef int (*cmpfunc)(PyObject *, PyObject *);
// typedef PyObject *(*reprfunc)(PyObject *);
// typedef long (*hashfunc)(PyObject *);
// typedef PyObject *(*richcmpfunc) (PyObject *, PyObject *, int);
// typedef PyObject *(*getiterfunc) (PyObject *);
// typedef PyObject *(*iternextfunc) (PyObject *);
// typedef PyObject *(*descrgetfunc) (PyObject *, PyObject *, PyObject *);
// typedef int (*descrsetfunc) (PyObject *, PyObject *, PyObject *);
// typedef int (*initproc)(PyObject *, PyObject *, PyObject *);
// typedef PyObject *(*newfunc)(struct _typeobject *, PyObject *, PyObject *);
// typedef PyObject *(*allocfunc)(struct _typeobject *, Py_ssize_t);

typedef PyObject *(*PyCFunction)(PyObject *, PyObject *);

struct PyMethodDef {
    const char	*ml_name;	/* The name of the built-in function/method */
    PyCFunction  ml_meth;	/* The C function that implements it */
    int		 ml_flags;	/* Combination of METH_xxx flags, which mostly
				   describe the args expected by the C func */
    const char	*ml_doc;	/* The __doc__ attribute, or NULL */
};
typedef struct PyMethodDef PyMethodDef;

typedef PyObject *(*getter)(PyObject *, void *);
typedef int (*setter)(PyObject *, PyObject *, void *);

typedef struct PyGetSetDef {
	char *name;
	getter get;
	setter set;
	char *doc;
	void *closure;
} PyGetSetDef;

typedef struct PyMemberDef {
	/* Current version, use this */
	char *name;
	int type;
	Py_ssize_t offset;
	int flags;
	char *doc;
} PyMemberDef;

typedef int* destructor;
typedef int* printfunc;
typedef int* getattrfunc;
typedef int* setattrfunc;
typedef int* cmpfunc;
typedef int* reprfunc;
typedef int* PyNumberMethods;
typedef int* PySequenceMethods;
typedef int* PyMappingMethods;
typedef int* hashfunc;
typedef int* ternaryfunc;
typedef int* reprfunc;
typedef int* getattrofunc;
typedef int* setattrofunc;
typedef int* PyBufferProcs;
typedef int* traverseproc;
typedef int* inquiry;
typedef int* richcmpfunc;
typedef int* getiterfunc;
typedef int* iternextfunc;
typedef int* descrgetfunc;
typedef int* descrsetfunc;
typedef int* initproc;
typedef int* allocfunc;
typedef int* newfunc;
typedef int* freefunc;
typedef int* inquiry;

typedef struct _typeobject {
	PyObject_VAR_HEAD
	const char *tp_name; /* For printing, in format "<module>.<name>" */
	Py_ssize_t tp_basicsize, tp_itemsize; /* For allocation */

	/* Methods to implement standard operations */

	destructor tp_dealloc;
	printfunc tp_print;
	getattrfunc tp_getattr;
	setattrfunc tp_setattr;
	cmpfunc tp_compare;
	reprfunc tp_repr;

	/* Method suites for standard classes */

	PyNumberMethods *tp_as_number;
	PySequenceMethods *tp_as_sequence;
	PyMappingMethods *tp_as_mapping;

	/* More standard operations (here for binary compatibility) */

	hashfunc tp_hash;
	ternaryfunc tp_call;
	reprfunc tp_str;
	getattrofunc tp_getattro;
	setattrofunc tp_setattro;

	/* Functions to access object as input/output buffer */
	PyBufferProcs *tp_as_buffer;

	/* Flags to define presence of optional/expanded features */
	long tp_flags;

	const char *tp_doc; /* Documentation string */

	/* Assigned meaning in release 2.0 */
	/* call function for all accessible objects */
	traverseproc tp_traverse;

	/* delete references to contained objects */
	inquiry tp_clear;

	/* Assigned meaning in release 2.1 */
	/* rich comparisons */
	richcmpfunc tp_richcompare;

	/* weak reference enabler */
	Py_ssize_t tp_weaklistoffset;

	/* Added in release 2.2 */
	/* Iterators */
	getiterfunc tp_iter;
	iternextfunc tp_iternext;

	/* Attribute descriptor and subclassing stuff */
	struct PyMethodDef *tp_methods;
	struct PyMemberDef *tp_members;
	struct PyGetSetDef *tp_getset;
	struct _typeobject *tp_base;
	PyObject *tp_dict;
	descrgetfunc tp_descr_get;
	descrsetfunc tp_descr_set;
	Py_ssize_t tp_dictoffset;
	initproc tp_init;
	allocfunc tp_alloc;
	newfunc tp_new;
	freefunc tp_free; /* Low-level free-memory routine */
	inquiry tp_is_gc; /* For PyObject_IS_GC */
	PyObject *tp_bases;
	PyObject *tp_mro; /* method resolution order */
	PyObject *tp_cache;
	PyObject *tp_subclasses;
	PyObject *tp_weaklist;
	destructor tp_del;

	/* Type attribute cache version tag. Added in version 2.6 */
	unsigned int tp_version_tag;

#ifdef COUNT_ALLOCS
	/* these must be last and never explicitly initialized */
	Py_ssize_t tp_allocs;
	Py_ssize_t tp_frees;
	Py_ssize_t tp_maxalloc;
	struct _typeobject *tp_prev;
	struct _typeobject *tp_next;
#endif
} PyTypeObject;


//////////////////////////////////////////////////////////////////////////
// PyIntObject
// typedef struct {
//     PyObject_VAR_HEAD
//     long ob_shash;
//     int ob_sstate;
//     char ob_sval[1];
// } PyStringObject;

// typedef struct {
// 	PyObject_HEAD
// 		long ob_ival;
// } PyIntObject;


//////////////////////////////////////////////////////////////////////////
// PyLongObject
typedef unsigned short digit;
// 
// struct _longobject {
// 	PyObject_VAR_HEAD
// 		digit ob_digit[1];
// };
// 
// typedef struct _longobject PyLongObject; /* Revealed in longintrepr.h */

//////////////////////////////////////////////////////////////////////////
// PyUnicodeObject
// #  define PY_UNICODE_TYPE wchar_t
// 
// typedef PY_UNICODE_TYPE Py_UNICODE;
// 
// typedef struct {
//     PyObject_HEAD
//     Py_ssize_t length;          /* Length of raw Unicode data in buffer */
//     Py_UNICODE *str;            /* Raw Unicode buffer */
//     long hash;                  /* Hash value; -1 if not set */
//     PyObject *defenc;           /* (Default) Encoded version as Python
//                                    string, or NULL; this is used for
//                                    implementing the buffer protocol */
// } PyUnicodeObject;


//////////////////////////////////////////////////////////////////////////
// PyBoolObject
/*typedef PyIntObject PyBoolObject;*/


//////////////////////////////////////////////////////////////////////////
// PyListObject
// typedef struct {
//     PyObject_VAR_HEAD
//     PyObject **ob_item;
//     Py_ssize_t allocated;
// } PyListObject;
// 

//////////////////////////////////////////////////////////////////////////
// PyTupleObject
// typedef struct {
//     PyObject_VAR_HEAD
//     PyObject *ob_item[1];
// } PyTupleObject;


//////////////////////////////////////////////////////////////////////////
// PyDictObject
// #define PyDict_MINSIZE 8
// 
// typedef struct {
//     /* Cached hash code of me_key.  Note that hash codes are C longs.
//      * We have to use Py_ssize_t instead because dict_popitem() abuses
//      * me_hash to hold a search finger.
//      */
//     Py_ssize_t me_hash;
//     PyObject *me_key;
//     PyObject *me_value;
// } PyDictEntry;
// 
// typedef struct _dictobject PyDictObject;
// 
// struct _dictobject {
//     PyObject_HEAD
//     Py_ssize_t ma_fill;  /* # Active + # Dummy */
//     Py_ssize_t ma_used;  /* # Active */
// 
//     /* The table contains ma_mask + 1 slots, and that's a power of 2.
//      * We store the mask instead of the size because the mask is more
//      * frequently needed.
//      */
//     Py_ssize_t ma_mask;
// 
//     /* ma_table points to ma_smalltable for small tables, else to
//      * additional malloc'ed memory.  ma_table is never NULL!  This rule
//      * saves repeated runtime null-tests in the workhorse getitem and
//      * setitem calls.
//      */
//     PyDictEntry *ma_table;
//     PyDictEntry *(*ma_lookup)(PyDictObject *mp, PyObject *key, long hash);
//     PyDictEntry ma_smalltable[PyDict_MINSIZE];
// };


//////////////////////////////////////////////////////////////////////////
// PyCFunctionObject
// typedef struct {
// 	PyObject_HEAD
// 		PyMethodDef *m_ml; /* Description of the C function to call */
// 	PyObject    *m_self; /* Passed as 'self' arg to the C func, can be NULL */
// 	PyObject    *m_module; /* The __module__ attribute, can be anything */
// } PyCFunctionObject;


//////////////////////////////////////////////////////////////////////////
// PySetObject
// #define PySet_MINSIZE 8
// 
// typedef struct {
// 	long hash;      /* cached hash code for the entry key */
// 	PyObject *key;
// } setentry;
// 
// 
// typedef struct _setobject PySetObject;
// struct _setobject {
//     PyObject_HEAD
// 
//     Py_ssize_t fill;  /* # Active + # Dummy */
//     Py_ssize_t used;  /* # Active */
// 
//     /* The table contains mask + 1 slots, and that's a power of 2.
//      * We store the mask instead of the size because the mask is more
//      * frequently needed.
//      */
//     Py_ssize_t mask;
// 
//     /* table points to smalltable for small tables, else to
//      * additional malloc'ed memory.  table is never NULL!  This rule
//      * saves repeated runtime null-tests.
//      */
//     setentry *table;
//     setentry *(*lookup)(PySetObject *so, PyObject *key, long hash);
//     setentry smalltable[PySet_MINSIZE];
// 
//     long hash;                  /* only used by frozenset objects */
//     PyObject *weakreflist;      /* List of weak references */
// };


//////////////////////////////////////////////////////////////////////////
// PyModuleObject
// typedef struct {
// 	PyObject_HEAD
// 		PyObject *md_dict;
// } PyModuleObject;
// 
