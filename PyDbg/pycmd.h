#pragma once

HRESULT CALLBACK help(PDEBUG_CLIENT4 Client, PCSTR args);

HRESULT CALLBACK pyobj(PDEBUG_CLIENT4 Client, PCSTR args);

HRESULT CALLBACK pyframe(PDEBUG_CLIENT4 Client, PCSTR args);
HRESULT CALLBACK pyframe_impl(ULONG_PTR Address);


HRESULT CALLBACK pythread(PDEBUG_CLIENT4 Client, PCSTR args);
HRESULT CALLBACK pythread_impl(BOOL bSimple);
HRESULT pythread_all_dml();
HRESULT pythread_single(ULONG threadId, ULONG threadSysId);
HRESULT pythread_dml(ULONG threadId, ULONG threadSysId);
BOOL pythread_is_pythonthread(ULONG threadId, ULONG threadSysId);