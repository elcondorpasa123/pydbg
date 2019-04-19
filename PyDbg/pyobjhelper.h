#pragma once

bool dbg_pyobject_int(ULONG_PTR addr, int indent = 0, int nestlevel = 0);

bool dbg_pyobject_long(ULONG_PTR addr, int indent = 0, int nestlevel = 0);

bool dbg_pyobject_string(ULONG_PTR addr, int indent = 0, int nestlevel = 0);

bool dbg_pyobject_unicode(ULONG_PTR addr, int indent = 0, int nestlevel = 0);

bool dbg_pyobject_bool(ULONG_PTR addr, int indent = 0, int nestlevel = 0);

bool dbg_pyobject_list(ULONG_PTR addr, int indent = 0, int nestlevel = 0);

bool dbg_pyobject_tuple(ULONG_PTR addr, int indent = 0, int nestlevel = 0);

bool dbg_pyobject_dict(ULONG_PTR addr, int indent = 0, int nestlevel = 0);

bool dbg_pyobject_set(ULONG_PTR addr, int indent = 0, int nestlevel = 0);

bool dbg_pyobject_function(ULONG_PTR addr, int indent = 0, int nestlevel = 0);

bool dbg_pyobject_module(ULONG_PTR addr, int indent = 0, int nestlevel = 0);

bool dbg_pyobject(ULONG_PTR addr, int indent = 0, int nestlevel = 0);

void setnestlevel(int level);
void resetnestlevel();

unsigned long getstructsize(char const* name);
bool readstruct_by_name(ULONG_PTR addr, char const* name, ULONG_PTR buffer, size_t bufferCountInBytes);